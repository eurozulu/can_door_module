//
// Created by Rob GIlham on 22/05/2022.
//

#include "canqueues.h"

#include "freertos/FreeRTOS.h"
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <esp32-hal-gpio.h>

#include "mcp2515.h"

const uint8_t PIN_CANBUS_IRQ = 11;
const enum CAN_SPEED CANSPEED = CAN_250KBPS;
portBASE_TYPE t_HPTaskAwoken = pdFALSE;
uint8_t MODULE_ID = 0;

QueueHandle_t inbound;
QueueHandle_t outbound;

SemaphoreHandle_t inboundAvailable;
SemaphoreHandle_t busInUse;

MCP2515 *mcp2515;

// Interrupt handler for inbound messages.
void irqBufferReceived() {
    xSemaphoreGiveFromISR(inboundAvailable, &t_HPTaskAwoken);
}


void frameToCan(FRAME *frame, struct can_frame *canFrame) {
    canFrame->can_id = frame->id;
    canFrame->can_dlc = frame->size;
    for (int i = 0; i < frame->size; i++) {
        canFrame->data[i] = frame->data[i];
    }
}

void canToFrame(can_frame *canFrame, struct FRAME *frame) {
    frame->id = canFrame->can_id;
    frame->size = canFrame->can_dlc;
    for (int i = 0; i < frame->size; i++) {
        frame->data[i] = canFrame->data[i];
    }
}

/**
 * non returning loop which monitors the interrupt semaphore and empties
 * the inbound canbus buffer into the inbound queue whenever interrupt occurs
 * Runs independantly of outbound monitor, both sharing bus via 'busInUse' semaphore
 */
void readInboundToQueue(void *param) {
    struct can_frame frame;
    struct FRAME qFrame;

    for (;;) {
        // Block on semaphore until an IRQ releases it.
        if (xSemaphoreTake(inboundAvailable, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        // Check which interrupt has fired, indicating inbound msg in the buffer
        xSemaphoreTake(busInUse, portMAX_DELAY);
        uint8_t irq = mcp2515->getInterrupts();

        // Push any found messages into inbound queue
        if (irq & MCP2515::CANINTF_RX0IF) {
            if (mcp2515->readMessage(MCP2515::RXB0, &frame) == MCP2515::ERROR_OK) {
                canToFrame(&frame, &qFrame);
                xQueueSend(inbound, &qFrame, portMAX_DELAY);
            }
        }

        if (irq & MCP2515::CANINTF_RX1IF) {
            if (mcp2515->readMessage(MCP2515::RXB1, &frame) == MCP2515::ERROR_OK) {
                canToFrame(&frame, &qFrame);
                xQueueSend(inbound, &qFrame, portMAX_DELAY);
            }
        }
        xSemaphoreGive(busInUse);

        // We have finished our task.  Return to the top of the loop where
        // we will block on the semaphore until it is time to execute
        // again.  Note when using the semaphore for synchronisation with an
        // ISR in this manner there is no need to 'give' the semaphore back.
    }
}

// non retuning loop to block on outbound queue and push messages to the bus as they arrive
void writeOutboundFromQueue() {
    struct FRAME qFrame;
    struct can_frame frame;
    for (;;) {
        // Blocks on empty queue
        if (xQueueReceive(outbound, &qFrame, portMAX_DELAY) != pdTRUE) {
            continue;
        }
        frameToCan(&qFrame, &frame);
        xSemaphoreTake(busInUse, portMAX_DELAY);
        mcp2515->sendMessage(&frame);
        xSemaphoreGive(busInUse);
    }
}

// main can bus task, running on second core.
// Note each core has its own set of independant interrupts
void canBusTask(void *param) {
    uint8_t csPin = *(uint8_t*)param;
    free(param);

    mcp2515 = new MCP2515(csPin);
    mcp2515->reset();
    mcp2515->setBitrate(CANSPEED);
    mcp2515->setLoopbackMode();

    busInUse = xSemaphoreCreateBinary();
    inboundAvailable = xSemaphoreCreateCounting(MAX_BUFFER_INBOUND, 0);

    // Run another thread to feed the inbound message queue from the CANBUS buffer.
    xTaskCreate(
            readInboundToQueue,
            "can input task",
            128,
            NULL,
            1,
            NULL);

    attachInterrupt(PIN_CANBUS_IRQ, irqBufferReceived, FALLING);
    writeOutboundFromQueue();
}

extern void startCanBus(const uint8_t csPin, uint8_t moduleId) {
    MODULE_ID = moduleId;
    outbound = xQueueCreate(MAX_BUFFER_OUTBOUND, sizeof(struct FRAME));
    inbound = xQueueCreate(MAX_BUFFER_INBOUND, sizeof(struct FRAME));

    if (outbound == NULL || inbound == NULL) {
        printf("Error creating queues");
        return;
    }
    uint8_t *pin = (uint8_t *)malloc(sizeof(uint8_t));
    *pin = csPin;
    // Kick off CANBUS tasks on other core
    xTaskCreatePinnedToCore(
            canBusTask,
            "CanBus Task",
            1000, // stack size
            (void*) pin, // param
            1, // priority
            NULL,
            0);  // core

}

void sendMessage(struct FRAME *frame) {
    frame->data[0] = MODULE_ID;
    xQueueSend(outbound, frame, portMAX_DELAY);
}


void sendMessageFromISR(struct FRAME *frame) {
    frame->data[0] = MODULE_ID;
    xQueueSendFromISR(outbound, frame, &t_HPTaskAwoken);
}


ERROR receiveMessage(struct FRAME *frame) {
    // Blocks until messages arrive from the BUS
    if (xQueueReceive(inbound, frame, portMAX_DELAY) != pdTRUE) {
        return ERR_MESSAGE_READING_INBOUND_QUEUE;
    }
    return 0;
}
