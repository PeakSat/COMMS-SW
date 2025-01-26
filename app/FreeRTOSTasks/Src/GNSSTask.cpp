#include <iomanip>
#include "GNSSTask.hpp"
#include "main.h"
#include "Logger.hpp"

void GNSSTask::printing(uint8_t* buf) {
    printing_counter++;
    etl::string<1024> GNSSMessageString = "";
    if (printing_counter == printing_frequency) {
        printing_counter = 0;
        GNSSMessageString.assign(buf, buf + GNSSPayloadSize);
        LOG_INFO << GNSSMessageString.c_str();
        // Clear the previous data
        GNSSMessageString.clear();
        memset(buf, 0, 1024);
    }
}

void GNSSTask::GNSSprint(const GNSSData& c) {
    LOG_INFO << "---------RMC---------";
    LOG_INFO << "latitude " << c.latitudeI;
    LOG_INFO << "longitude " << c.longitudeI;
    LOG_INFO << "year: " << c.year;
    LOG_INFO << "month: " << c.month;
    LOG_INFO << "day: " << c.day;
    LOG_INFO << "valid: " << c.valid;
    LOG_INFO << "hours " << c.hours;
    LOG_INFO << "minutes " << c.minutes;
    LOG_INFO << "seconds " << c.seconds;
    LOG_INFO << "microseconds " << c.microseconds;
    LOG_INFO << "speed over ground[km/s] " << c.speed;
    LOG_INFO << "course over ground[deg] " << c.course;
    LOG_INFO << "---------GGA---------";
    LOG_INFO << "altitude " << c.altitudeI;
    LOG_INFO << "quality indicator " << c.fix_quality;
    LOG_INFO << "satellites tracked " << c.satellites_tracked;
}


void GNSSTask::setCompactGnssDataRMC(GNSSData& compact, const minmea_sentence_rmc& frame_rmc) {
    // latitude
    compact.latitudeD = static_cast<double>(frame_rmc.latitude.value) / 10000000.0;
    compact.latitudeD = convertToDecimalDegrees(compact.latitudeD);
    // convert to int
    compact.latitudeI = static_cast<int32_t>(10000000 * compact.latitudeD);
    // longitude
    compact.longitudeD = static_cast<double>(frame_rmc.longitude.value) / 10000000.0;
    compact.longitudeD = convertToDecimalDegrees(compact.longitudeD);
    compact.longitudeI = static_cast<int32_t>(10000000 * compact.longitudeD);
    // date
    compact.year = static_cast<int8_t>(frame_rmc.date.year);
    compact.month = static_cast<int8_t>(frame_rmc.date.month);
    compact.day = static_cast<int8_t>(frame_rmc.date.day);
    // time
    compact.hours = static_cast<int8_t>(frame_rmc.time.hours);
    compact.minutes = static_cast<int8_t>(frame_rmc.time.minutes);
    compact.seconds = static_cast<int8_t>(frame_rmc.time.seconds);
    compact.microseconds = frame_rmc.time.microseconds;
    // speed
    compact.speed = static_cast<double>(frame_rmc.speed.value) / 10.0f;
    // convert to km/s for speed over ground
    compact.speed = (double) (0.000514444 * compact.speed);
    // course over ground
    compact.course = static_cast<double>(frame_rmc.course.value) / 10.0f;
}

void GNSSTask::setCompactGnssDataGGA(GNSSData& compact, const minmea_sentence_gga& frame_gga) {
    // altitude
    compact.altitude = static_cast<double>(frame_gga.altitude.value) / 10.0f;
    // convert
    compact.altitudeI = static_cast<int32_t>(compact.altitude * 10);
    // fix quality
    compact.fix_quality = static_cast<int8_t>(frame_gga.fix_quality);
    // satellites tracked
    compact.satellites_tracked = static_cast<int8_t>(frame_gga.satellites_tracked);
}

// void GNSSTask::initGNSS() {
//     HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_SET);
//     HAL_GPIO_WritePin(GNSS_EN_GPIO_Port, GNSS_EN_Pin, GPIO_PIN_RESET);
//     HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_SET);
//     resetGNSSHardware();
//     controlGNSSwithACK(GNSSReceiver::setFactoryDefaults(DefaultType::RebootAfterSettingToFactoryDefaults));
//     controlGNSSwithACK(GNSSReceiver::configureNMEATalkerID(TalkerIDType::GPMode, Attributes::UpdateSRAMandFLASH));
//     etl::vector<uint8_t, 12> interval_vec;
//     interval_vec.resize(12, 0);
//     uint8_t seconds = 1;
//     // 4 is for RMC, 2 for GSV, 0 is for GGA
//     interval_vec[0] = seconds;
//     //    interval_vec[2] = seconds;
//     interval_vec[4] = seconds;
//     controlGNSSwithACK(GNSSReceiver::configureGNSSNavigationMode(NavigationMode::Auto, Attributes::UpdateToSRAM));
//     controlGNSSwithACK(GNSSReceiver::configureSystemPositionRate(PositionRate::Option2Hz, Attributes::UpdateToSRAM));
//     controlGNSSwithACK(GNSSReceiver::configureExtendedNMEAMessageInterval(interval_vec, Attributes::UpdateToSRAM));
// }


void GNSSTask::initGNSS() {
    HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GNSS_EN_GPIO_Port, GNSS_EN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_SET);
    resetGNSSHardware();
    controlGNSSwithACK(GNSSReceiver::setFactoryDefaults(DefaultType::RebootAfterSettingToFactoryDefaults));
    controlGNSSwithACK(GNSSReceiver::configureNMEATalkerID(TalkerIDType::GPMode, Attributes::UpdateSRAMandFLASH));
    etl::vector<uint8_t, 12> interval_vec;
    interval_vec.resize(12, 0);
    uint8_t seconds = 1;
    // 4 is for RMC, 2 for GSV, 0 is for GGA
    interval_vec[0] = seconds;
    //    interval_vec[2] = seconds;
    interval_vec[4] = seconds;
    controlGNSSwithACK(GNSSReceiver::configureGNSSNavigationMode(NavigationMode::Auto, Attributes::UpdateToSRAM));
    controlGNSSwithACK(GNSSReceiver::configureSystemPositionRate(PositionRate::Option2Hz, Attributes::UpdateToSRAM));
    controlGNSSwithACK(GNSSReceiver::configureExtendedNMEAMessageInterval(interval_vec, Attributes::UpdateToSRAM));
}



void GNSSTask::parser(uint8_t* buf, GNSSData& compact) {
    etl::string<1024> GNSSMessageString = "";
    GNSSMessageString.assign(buf, buf + GNSSPayloadSize);
    minmea_sentence_rmc frame_rmc{};
    minmea_sentence_gga frame_gga{};
    // LOG_INFO << GNSSMessageString.c_str();
    // Tokenize the string by newline characters
    char* line = strtok(&GNSSMessageString[0], "\n");
    while (line != nullptr) {
        switch (minmea_sentence_id(line, true)) {
            case MINMEA_SENTENCE_RMC: {
                if (minmea_parse_rmc(&frame_rmc, line)) {
                    setCompactGnssDataRMC(compact, frame_rmc);
                }
                break;
            }
            case MINMEA_SENTENCE_GGA: {
                if (minmea_parse_gga(&frame_gga, line)) {
                    setCompactGnssDataGGA(compact, frame_gga);
                }
                break;
            }
            default:
                break;
        }
        line = strtok(nullptr, "\n"); // Get the next line
    }
}

void GNSSTask::switchGNSSMode() {
    etl::vector<uint8_t, 12> interval_vec;
    uint8_t seconds;
    if (interval_mode_flag) {
        LOG_INFO << "FAST MODE";
        interval_vec.resize(12, 0);
        // seconds = 1, Position Rate = 2Hz gives 2Hz
        seconds = 1;
        // 4 is for RMC, 6 for ZDA, 0 is for GGA
        interval_vec[0] = seconds;
        interval_vec[4] = seconds;
        controlGNSSwithACK(GNSSReceiver::configureSystemPositionRate(PositionRate::Option5Hz, Attributes::UpdateSRAMandFLASH));
    } else {
        LOG_INFO << "SLOW MODE";
        interval_vec.resize(12, 0);
        // seconds = 6, Position Rate = 1Hz gives 1/6 Hz
        seconds = 2;
        // 4 is for RMC, 6 for ZDA, 0 is for GGA
        interval_vec[0] = seconds;
        interval_vec[4] = seconds;
        controlGNSSwithACK(GNSSReceiver::configureSystemPositionRate(PositionRate::Option2Hz, Attributes::UpdateToSRAM));
    }
    auto status = controlGNSSwithACK(GNSSReceiver::configureExtendedNMEAMessageInterval(interval_vec, Attributes::UpdateToSRAM));
    if (status.has_value())
        LOG_DEBUG << "GNSS INTERVAL CHANGED WITH SUCCESS";
    else
        LOG_ERROR << "GNSS INTERVAL COULD NOT BE CHANGED";
    interval_mode_flag = !interval_mode_flag;
}

void GNSSTask::startReceiveFromUARTwithIdle(uint8_t* buf, uint16_t size) {
    // start the DMA
    HAL_UARTEx_ReceiveToIdle_DMA(&huart5, buf, size);
    // disabling the half buffer interrupt //
    __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_HT);
    //  disabling the full buffer interrupt //
    __HAL_DMA_DISABLE_IT(&hdma_uart5_rx, DMA_IT_TC);
}

// Helper function to handle GNSS reset
void GNSSTask::resetGNSSHardware() {
    HAL_GPIO_WritePin(GNSS_RSTN_GPIO_Port, GNSS_RSTN_Pin, GPIO_PIN_RESET);
    vTaskDelay(50);
    // TODO pass this delay to the parameter map
    HAL_GPIO_WritePin(GNSS_RSTN_GPIO_Port, GNSS_RSTN_Pin, GPIO_PIN_SET);
    vTaskDelay(50);
}

etl::expected<Status, Error> GNSSTask::controlGNSSwithACK(GNSSMessage gnssMessageToSend) {
    control = true;
    // This is actually useful for the very first command because for an unknown reason does not return ACK
    constexpr uint8_t maxRetries = MAX_RETRIES;
    // Initialize error to indicate no error initially
    Error currentError = Error::Timeout;
    Status currentStatus = Status::ERROR;
    // Try up to maxRetries to receive an ACK/NACK
    vTaskDelay(DELAY_BTW_COMMANDS);
    for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
        if (HAL_UART_Transmit(&huart5, gnssMessageToSend.messageBody.data(), gnssMessageToSend.messageBody.size(), 1000) != HAL_OK)
            currentError = Error::TransmissionFailed;
        uint32_t received_events;
        if (xTaskNotifyWaitIndexed(GNSS_INDEX_ACK, pdFALSE, pdTRUE, &received_events, pdMS_TO_TICKS(MAXIMUM_INTERVAL_ACK)) == pdTRUE) {
            LOG_DEBUG << "Received GNSS ACK";
            currentError = Error::NoError;
            break;
        }
    }
    if (currentError != Error::NoError) {
        currentStatus = Status::ERROR;
        LOG_ERROR << "Operation failed with error: " << static_cast<int>(currentError);
    }
    else
        currentStatus = Status::OK;
    return currentStatus;
}


void GNSSTask::initQueuesToAcceptPointers() {
    gnssQueueHandle = xQueueCreate(uxQueueLength, sizeof(uint8_t*));
    if (gnssQueueHandle == nullptr)
        LOG_ERROR << "Queue did not initialized properly";
    else
        LOG_INFO << "Queue initialized with success";
}

[[noreturn]] void GNSSTask::execute() {
    rx_buf_pointer = rx_buf;
    uint8_t* rx_buf_p_from_queue;
    startReceiveFromUARTwithIdle(rx_buf_pointer, 1024);
    initGNSS();
    GNSSData compact{};
    int timeoutCounter = 0;
    Logger::format.precision(Precision);
    uint32_t receivedEvents = 0;
    while (true) {
        // you may have a counter that counts how many
        if (xTaskNotifyWaitIndexed(GNSS_INDEX_MESSAGE, pdFALSE, pdTRUE, &receivedEvents, pdMS_TO_TICKS(MAXIMUM_INTERVAL)) == pdTRUE) {
            if (receivedEvents & GNSS_MESSAGE_READY) {
                // Receive a message on the created queue.Block for 100ms if the message is not immediately available
                if (xQueueReceive(gnssQueueHandle, &rx_buf_p_from_queue, pdMS_TO_TICKS(100)) == pdTRUE) {
                    if (rx_buf_p_from_queue != nullptr) {
                        parser(rx_buf_p_from_queue, compact);
                        // filter the messages
                        // fix quality 1 means : valid position fix, SPS mode
                        // if (compact.fix_quality == 1 && compact.satellites_tracked > 3)
                        GNSSprint(compact);
                        timeoutCounter = 0;
                    }
                } else {
                    LOG_ERROR << "Queue Timeout";
                }
            }
        } else {
            LOG_ERROR << "Timeout waiting for GNSS message.";
            timeoutCounter++;
            if (timeoutCounter >= 5) {
                LOG_ERROR << "Multiple GNSS timeouts, attempting to reset GNSS communication.";
                resetGNSSHardware();
                LOG_ERROR << "GNSS reset due to timeout";
                timeoutCounter = 0; // Reset the counter after corrective action
            }
        }
    }
}
