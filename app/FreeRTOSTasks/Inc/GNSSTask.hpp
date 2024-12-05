#pragma once
#include <etl/optional.h>
#include "Task.hpp"
#include "queue.h"
#include "GNSSDefinitions.hpp"
#include "GNSS.hpp"
#include "GNSSMessage.hpp"
#include "stm32h7xx_hal.h"
#include <etl/string.h>
#include <etl/expected.h>
#include "minmea.h"
#include "Logger.hpp"

#define printing_frequency 1

extern UART_HandleTypeDef huart5;
extern DMA_HandleTypeDef hdma_uart5_rx;

class GNSSTask : public Task {
public:
    /**
     * Logger Precision
     */
    static constexpr uint8_t Precision = 6;
    /**
    * Depth of the stack allocated for the task, in 16-bit words.
    */
    static const uint16_t TaskStackDepth = 4000;

    /**
    * Array representing the stack memory for the task.
    * This array is statically allocated with `TaskStackDepth` elements.
    */
    StackType_t taskStack[TaskStackDepth];

    /**
    * Buffer for storing raw GNSS data received via UART.
    * Initialized to zeros and has a fixed size of 1024 bytes.
    */
    uint8_t rx_buf[1024] = {0};

    /**
    * Pointer to the `rx_buf` array
    */
    uint8_t* rx_buf_pointer = nullptr;

    /**
    * Flag to toggle between GNSS operational modes (e.g., fast mode or slow mode).
    */
    uint8_t interval_mode_flag = 0;

    /**
    * Size of the incoming data received from the GNSS module, measured in bytes.
    * `size_response` holds the size of the GNSS response, while `size_message`
    * refers to the size of the message being processed.
    */
    uint16_t size_response, size_message = 0;

    /**
     * Counter to control the frequency of GNSS data printing.
    * Helps limit the number of prints for performance or debugging purposes.
    */
    uint8_t printing_counter = 0;

    /**
    * Size of each item in the queue, measured in bytes.
    */
    unsigned long uxItemSize = sizeof(uint8_t*);

    /**
    * Maximum number of items the queue can hold at a time.
    * Configured to `1` in this case, indicating a single-slot queue.
    */
    const unsigned long uxQueueLength = 1;

    /**
    * Handle for the default GNSS queue, which separates its operations from other queues.
    * Used to send and receive general GNSS-related messages.
    */
    QueueHandle_t gnssQueueHandleDefault;

    /**
    * Handle for the GNSS response queue, dedicated to processing responses such as ACK or NACK.
    * Used for task communication specific to GNSS command handling.
    */
    QueueHandle_t gnssQueueHandleGNSSResponse;

    /**
    * Pointer to data being sent to the default GNSS queue.
    * This is a staging area for preparing data before pushing it into the queue.
    */
    uint8_t* sendToQueue;

    /**
    * Pointer to data being sent to the GNSS response queue.
    * Holds response-specific data to be processed by tasks that wait for ACK/NACK.
    */
    uint8_t* sendToQueueResponse;

    /**
    * Control flag used for task synchronization or GNSS state management.
    * Its value can be modified dynamically to track or adjust GNSS-related operations.
    */
    uint8_t control = 0;

    /**
    * Executes the main logic of the GNSS task.
    */
    void execute();

    /**
    * Prints the GNSS data with a configurable frequency. The data to be printed
    * is provided via the input buffer.
    * @param buf A pointer to the buffer containing the GNSS data to print.
    */
    void printing(uint8_t* buf);


    /**
    * Resets the GNSS hardware by toggling the reset pin. This is used to recover
    * the GNSS module from errors or unresponsive states.
    */
    static void resetGNSSHardware();

    /**
    * Prints the variables contained in the compact GNSS data structure.
    * @param c A pointer to the GNSSData structure to be printed.
     */
    static void rawGNSSprinting(const GNSSData& c);

    /**
    * Updates the compact GNSS data structure with variables from a parsed RMC sentence.
    * @param compact A pointer to the GNSSData structure to update.
    * @param frame_rmc A pointer to the parsed RMC sentence structure containing the data.
    */
    static void setCompactGnssDataRMC(GNSSData& compact, const minmea_sentence_rmc& frame_rmc);

    /**
    * Updates the compact GNSS data structure with variables from a parsed GGA sentence.
    * @param compact A pointer to the GNSSData structure to update.
    * @param frame_gga A pointer to the parsed GGA sentence structure containing the data.
    */
    static void setCompactGnssDataGGA(GNSSData& compact, const minmea_sentence_gga& frame_gga);

    /**
    * Updates the compact GNSS data structure with variables from a parsed GGA sentence.
    * @param compact A pointer to the GNSSData structure to update.
    * @param frame_gsv A pointer to the parsed GSV sentence structure containing the data.
    */
    static void setCompactGnssDataGSV(GNSSData& compact, const minmea_sentence_gsv& frame_gsv);

    /**
    * Parses raw GNSS data and extracts key variables into a compact structure.
    * This function processes the input buffer to parse sentences (e.g., RMC, GGA)
    * and stores only the necessary data in the compact structure.
    * @param buf A pointer to the raw GNSS data buffer.
     @param compact A pointer to the CompactGNSSData structure where parsed data will be stored.
    */
    static void parser(uint8_t* buf, GNSSData& compact);

    /**
    * Sends a command to the GNSS module and waits for an ACK or NACK response.
    * Retries transmission up to a maximum number of attempts and resets the GNSS module
    * if no response is received.
    * @param gnssMessageToSend The GNSSMessage containing the command to be sent.
    * @return An `etl::expected` containing `void` on success (ACK received), or an `ErrorFromGNSS` enumeration value (`TransmissionFailed`, `Timeout`, or `NACKReceived`) on failure.
    *
    */
    etl::expected<void, ErrorFromGNSS> controlGNSSwithNotify(GNSSMessage gnssMessageToSend);

    /**
    * Toggles between fast mode and slow mode for testing the GNSS module's functionality
    * and the responsiveness of the control logic.
    */
    void switchGNSSMode();

    /**
    * Starts receiving data from the UART interface with idle line detection enabled and disables the HT, TC interrupts
    * @param buf A pointer to the buffer where received data will be stored.
    * @param size The size of the buffer in bytes, specifying the maximum amount of data to receive.
    */
    static void startReceiveFromUARTwithIdle(uint8_t* buf, uint16_t size);

    /**
    *   Initializes the FreeRTOS queues to handle pointers.
    */
    void initQueuesToAcceptPointers();

    /**
     * initializes the GNSS
     */
    void initGNSS();

    /**
     *
     */
    // Template function to convert DDM to Degrees Decimal Degrees
    template <typename T>
    static T convertToDecimalDegrees(T ddm) {
        int degrees = static_cast<int>(ddm);
        // Extract minutes (fractional part)
        T minutes = (ddm - degrees) * 100;
        //        LOG_DEBUG << "minutes: " << minutes;
        // Return the decimal degrees by adding the minutes divided by 60
        T dd = degrees + (minutes / static_cast<T>(60.0));
        //        LOG_DEBUG << "dd: " << dd;
        return dd;
    }

    GNSSTask() : Task("GNSS Logging Task") {}

    void createTask() {
        taskHandle = xTaskCreateStatic(vClassTask<GNSSTask>, this->TaskName,
                                       GNSSTask::TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                                       this->taskStack, &(this->taskBuffer));
        initQueuesToAcceptPointers();
    }

private:
};

inline etl::optional<GNSSTask> gnssTask;
