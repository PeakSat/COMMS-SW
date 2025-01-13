#pragma once
#include "Task.hpp"
#include "queue.h"
#include "CANDriver.hpp"
#include "Logger.hpp"

#include <optional>
/**
* Every variable needed to control the incoming frames' fifo buffer
* will be stored in this struct.
 */
struct incomingFIFO {
    uint8_t* buffer;
    uint32_t NOfItems;
    uint32_t lastItemPointer;
    incomingFIFO() : buffer(nullptr), NOfItems(0), lastItemPointer(0) {}
    incomingFIFO(uint8_t* externalBuffer, uint32_t NOfItems) : buffer(externalBuffer), NOfItems(NOfItems), lastItemPointer(0) {}
};
extern incomingFIFO incomingFIFO;


/**
 * Contains functionality of a Gatekeeper Task for the CAN Bus. It has the sole access to CAN, to avoid any
 * deadlocks that might be caused by simultaneous requests of access to the same resource. It works by having anyone
 * needing to access CAN, send the data in a queue. Then this task receives queue elements and sends them via CAN.
 *
 * @example @code
 * uint32_t id = 0x4; // Specify the sending Node ID.
 * etl::vector<uint8_t, 8> data = {0,1,2,3,4,5,6,7}; // Specify an array of data, up to 64 bytes.
 * CAN::Packet message = {id, data}; // Create a CAN::Packet object.
 * canGatekeeperTask->addToQueue(message); // Add the message to the outgoing queue.
 * @endcode
 */
class CANGatekeeperTask : public Task {
public:
    /**
     * A freeRTOS queue to handle outgoing messages, to keep order in case tasks interrupt each other.
     */
    QueueHandle_t outgoingQueue;
    /**
     * The variable used to hold the queue's data structure.
     */
    static inline StaticQueue_t outgoingQueueBuffer;
    /**
     * The maximum of the length of the queue for incoming/outgoing CAN Packets.
     */
    static const uint8_t PacketQueueSize = 20;
    /**
     * Storage area given to freeRTOS to manage the queue items.
     */
    static inline uint8_t outgoingQueueStorageArea[PacketQueueSize * sizeof(CAN::Packet)];
 * A freeRTOS queue to handle incoming Packets part of a CAN-TP message, since they need to be parsed as a whole.
 */
    QueueHandle_t incomingFrameQueue;
    /**
   * The variable used to hold the queue's data structure.
   */
    static inline StaticQueue_t incomingFrameQueueBuffer;

    /**
   * Storage area given to freeRTOS to manage the queue items.
   */
    static inline uint8_t incomingFrameQueueStorageArea[sizeOfIncommingFrameBuffer * sizeof(CAN::Frame)];


    /**
* A freeRTOS queue to handle incoming Packets part of a CAN-TP message, since they need to be parsed as a whole.
*/
    QueueHandle_t storedPacketQueue;
    /**
   * The variable used to hold the queue's data structure.
   */
    static inline StaticQueue_t storedPacketQueueBuffer;

    /**
   * Storage area given to freeRTOS to manage the queue items.
   */
    static inline uint8_t storedPacketQueueStorageArea[sizeOfIncommingFrameBuffer * sizeof(CAN::Frame)];

    const static inline uint16_t TaskStackDepth = 7000;

    StackType_t taskStack[TaskStackDepth];

    CAN::ActiveBus ActiveBus = CAN::ActiveBus::Main;

public:
    void execute();

    /**
     * The constructor calls the initialize() function of the CAN::Driver. It also initializes the FreeRTOS queues for
     * incoming/outgoing messages.
     */
    CANGatekeeperTask();

    /**
     * Adds an CAN::Packet to the CAN Gatekeeper's queue.
     *
     * This function was added as an extra abstraction layer to house the `xQueueSendToBack` function.
     * It can be used from anywhere in the code to get access to the CAN queue/CAN Gatekeeper task, without having to
     * know the low level details of the queue.
     *
     * If the queue is full, the message is not added to the queue and an error is logged.
     *
     * @param message the CAN::Packet to be added in the queue of the CAN Gatekeeper task.
     * @param isISR indicating if the message is a response to another CAN Message, thus composed through an ISR
     */
    inline void send(const CAN::Packet& message, bool isISR = false) {
        BaseType_t status;

        if (isISR) {
            BaseType_t taskShouldYield = pdFALSE;

            status = xQueueSendToBackFromISR(outgoingQueue, &message, &taskShouldYield);

            if (taskShouldYield) {
                taskYIELD();
            }
        } else {
            status = xQueueSendToBack(outgoingQueue, &message, 0);
        }

        if (status == errQUEUE_FULL) {
            LOG_ERROR << "Tried sending CAN Message while outgoing queue is full!";
        }
    }

    /**
     * Adds a CAN::Packet to the incomingQueue.
     * If the queue is full the message is lost.
     *
     * @note This function is designed to be used from within the ISR of a CAN Message Receipt. Thus, it uses
     * freeRTOS's ISR-Specific functions.
     *
     * @param message The incoming CAN::Packet.
     */
    inline void addSFToIncoming(const CAN::Packet& message) {
        BaseType_t taskShouldYield = pdFALSE;


        if (taskShouldYield) {
            taskYIELD();
        }
    }

    /**
     * Adds a CAN::Packet to the incomingQueue.
     * If the queue is full the message is lost.
     *
     * @note This function is designed to be used from within the ISR of a CAN Message Receipt. Thus, it uses
     * freeRTOS's ISR-Specific functions.
     *
     * @param message The incoming CAN::Packet.
     */
    inline void addMFToIncoming(const CAN::Packet& message) {
        BaseType_t taskShouldYield = pdFALSE;


        if (taskShouldYield) {
            taskYIELD();
        }
    }

    inline void switchActiveBus(CAN::ActiveBus activeBus) {
        this->ActiveBus = activeBus;
    }


    void createTask() {
        taskHandle = xTaskCreateStatic(vClassTask<CANGatekeeperTask>, this->TaskName, CANGatekeeperTask::TaskStackDepth, this,
                                       tskIDLE_PRIORITY + 2, this->taskStack, &(this->taskBuffer));
    }
};

inline std::optional<CANGatekeeperTask> canGatekeeperTask;