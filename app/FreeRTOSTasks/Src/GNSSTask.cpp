#include <iomanip>
#include "GNSSTask.hpp"
#include "main.h"
#include "Logger.hpp"
#include <eMMC.hpp>

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
    // LOG_INFO << "latitude " << c.latitudeI;
    // LOG_INFO << "longitude " << c.longitudeI;
    LOG_INFO << "latitude " << COMMSParameters::gnss_lat.getValue();
    LOG_INFO << "longitude " << COMMSParameters::gnss_long.getValue();
    LOG_INFO << "utc: " << c.utc;
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
    // LOG_INFO << "altitude " << c.altitudeI;
    LOG_INFO << "altitude " << COMMSParameters::gnss_alt.getValue();
    // LOG_INFO << "quality indicator " << c.fix_quality;
    LOG_INFO << "quality indicator " << COMMSParameters::gnss_fix_quality.getValue();
    // LOG_INFO << "satellites tracked " << c.satellites_tracked;
    LOG_INFO << "satellites tracked: " << COMMSParameters::satellites_tracked.getValue();
}


void GNSSTask::setCompactGnssDataRMC(GNSSData& compact, const minmea_sentence_rmc& frame_rmc) {
    // latitude
    compact.latitudeD = static_cast<double>(frame_rmc.latitude.value) / 10000000.0;
    compact.latitudeD = convertToDecimalDegrees(compact.latitudeD);
    // convert to int
    compact.latitudeI = static_cast<int32_t>(10000000 * compact.latitudeD);
    COMMSParameters::gnss_lat.setValue(compact.latitudeI);
    // longitude
    compact.longitudeD = static_cast<double>(frame_rmc.longitude.value) / 10000000.0;
    compact.longitudeD = convertToDecimalDegrees(compact.longitudeD);
    compact.longitudeI = static_cast<int32_t>(10000000 * compact.longitudeD);
    COMMSParameters::gnss_long.setValue(compact.longitudeI);
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
    COMMSParameters::gnss_alt.setValue(compact.altitudeI);
    // fix quality
    compact.fix_quality = static_cast<int8_t>(frame_gga.fix_quality);
    COMMSParameters::gnss_fix_quality.setValue(compact.fix_quality);
    // satellites tracked
    compact.satellites_tracked = static_cast<int8_t>(frame_gga.satellites_tracked);
    COMMSParameters::satellites_tracked.setValue(compact.satellites_tracked);
}


void GNSSTask::initGNSS() {
    HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GNSS_EN_GPIO_Port, GNSS_EN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_SET);
    resetGNSSHardware();
    controlGNSS(GNSSReceiver::setFactoryDefaults(DefaultType::RebootAfterSettingToFactoryDefaults));
    controlGNSS(GNSSReceiver::configureNMEATalkerID(TalkerIDType::GPMode, Attributes::UpdateSRAMandFLASH));
    etl::vector<uint8_t, 12> interval_vec;
    interval_vec.resize(12, 0);
    uint8_t seconds = 1;
    // 4 is for RMC, 2 for GSV, 0 is for GGA
    interval_vec[0] = seconds;
    //    interval_vec[2] = seconds;
    interval_vec[4] = seconds;
    controlGNSS(GNSSReceiver::configureGNSSNavigationMode(NavigationMode::Auto, Attributes::UpdateToSRAM));
    controlGNSS(GNSSReceiver::configureSystemPositionRate(PositionRate::Option1Hz, Attributes::UpdateToSRAM));
    controlGNSS(GNSSReceiver::configureExtendedNMEAMessageInterval(interval_vec, Attributes::UpdateToSRAM));
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
        controlGNSS(GNSSReceiver::configureSystemPositionRate(PositionRate::Option5Hz, Attributes::UpdateSRAMandFLASH));
    } else {
        LOG_INFO << "SLOW MODE";
        interval_vec.resize(12, 0);
        // seconds = 6, Position Rate = 1Hz gives 1/6 Hz
        seconds = 2;
        // 4 is for RMC, 6 for ZDA, 0 is for GGA
        interval_vec[0] = seconds;
        interval_vec[4] = seconds;
        controlGNSS(GNSSReceiver::configureSystemPositionRate(PositionRate::Option2Hz, Attributes::UpdateToSRAM));
    }
    auto status = controlGNSS(GNSSReceiver::configureExtendedNMEAMessageInterval(interval_vec, Attributes::UpdateToSRAM));
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

etl::expected<Status, Error> GNSSTask::controlGNSS(GNSSMessage gnssMessageToSend) {
    gnss_handler.CONTROL = true;
    uint8_t maxRetries = gnss_handler.CMD_RETRIES;
    Error currentError = {};
    Status currentStatus;
    uint32_t received_events;
    vTaskDelay(gnss_handler.DELAY_BTW_CMDS_MS);
    for (uint8_t attempt = 0; attempt < maxRetries; attempt++) {
        if (HAL_UART_Transmit(&huart5, gnssMessageToSend.messageBody.data(), gnssMessageToSend.messageBody.size(), 1000) != HAL_OK)
            currentError = Error::TransmissionFailed;
        if (xTaskNotifyWaitIndexed(GNSS_INDEX_ACK, pdFALSE, pdTRUE, &received_events, pdMS_TO_TICKS(gnss_handler.ACK_TIMOUT_MS)) == pdTRUE) {
            LOG_DEBUG << "Received GNSS ACK";
            currentError = Error::NoError;
            break;
        }
    }
    if (currentError != Error::NoError) {
        currentStatus = Status::ERROR;
        LOG_ERROR << "Operation failed with error: " << static_cast<int>(currentError);
    } else
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
    vTaskDelay(1000);
    HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_SET);
    COMMSParameters::gnss_ack_timeout.setValue(gnss_handler.ACK_TIMOUT_MS);
    COMMSParameters::gnss_cmd_retries.setValue(gnss_handler.CMD_RETRIES);
    COMMSParameters::gnss_delay_cmds.setValue(gnss_handler.DELAY_BTW_CMDS_MS);
    COMMSParameters::gnss_error_timeout.setValue(gnss_handler.ERROR_TIMEOUT_MS);
    COMMSParameters::error_timeout_cnt_thrhd.setValue(gnss_handler.ERROR_TIMEOUT_COUNTER_THRD);
    rx_buf_pointer = rx_buf;
    uint8_t* rx_buf_p_from_queue;
    startReceiveFromUARTwithIdle(rx_buf_pointer, 1024);
    initGNSS();
    GNSSData compact{};
    uint16_t gnss_error_timout_counter = 0;
    Logger::format.precision(Precision);
    uint32_t receivedEvents = 0;

    uint32_t NumberOfMeasurementsInStruct = 0;

    eMMCDataTailPointer = GNSSReceiver::findTailPointer();

    while (true) {
        if (xTaskNotifyWaitIndexed(GNSS_INDEX_MESSAGE, pdFALSE, pdTRUE, &receivedEvents, pdMS_TO_TICKS(gnss_handler.ERROR_TIMEOUT_MS)) == pdTRUE) {
            if (receivedEvents & GNSS_MESSAGE_READY) {
                // Receive a message on the created queue.Block for 100ms if the message is not immediately available
                if (xQueueReceive(gnssQueueHandle, &rx_buf_p_from_queue, pdMS_TO_TICKS(100)) == pdTRUE) {
                    if (rx_buf_p_from_queue != nullptr) {
                        parser(rx_buf_p_from_queue, compact);

                        if (GNSSReceiver::isDataValid(compact.year, compact.month, compact.day) == true) {
                            std::tm time{};
                            time.tm_year = compact.year + 2000;
                            time.tm_mon = compact.month - 1;
                            time.tm_mday = static_cast<uint8_t>(compact.day);
                            time.tm_hour = static_cast<uint8_t>(compact.hours);
                            time.tm_min = static_cast<uint8_t>(compact.minutes);
                            time.tm_sec = static_cast<uint8_t>(compact.seconds);
                            uint64_t timeFromEpoch = std::mktime(&time);
                            timeFromEpoch *= 1000000;
                            timeFromEpoch += static_cast<uint64_t>(compact.microseconds);
                            LOG_DEBUG << "Time from epoch: " << timeFromEpoch;

                            uint8_t numberOfSatelites = compact.satellites_tracked;
                            if (numberOfSatelites > 0x1F) {
                                numberOfSatelites = 0x1F;
                            }
                            uint64_t timeAndNofSat = (timeFromEpoch << 5) | numberOfSatelites;

                            GNSSDataForEMMC.altitudeI[NumberOfMeasurementsInStruct] = COMMSParameters::gnss_alt.getValue();
                            GNSSDataForEMMC.latitudeI[NumberOfMeasurementsInStruct] = COMMSParameters::gnss_lat.getValue();
                            GNSSDataForEMMC.longitudeI[NumberOfMeasurementsInStruct] = COMMSParameters::gnss_long.getValue();
                            GNSSDataForEMMC.usFromEpoch_NofSat[NumberOfMeasurementsInStruct] = timeAndNofSat;

                            //send data to eMMC
                            if (NumberOfMeasurementsInStruct >= GNSS_MEASUREMENTS_PER_STRUCT) {
                                GNSSDataForEMMC.valid = 0xAA;
                                if (eMMCDataTailPointer > eMMC::memoryMap[eMMC::GNSSData].size / eMMC::memoryPageSize) {
                                    eMMCDataTailPointer = 0;
                                }
                                auto status = eMMC::storeItem(eMMC::memoryMap[eMMC::GNSSData], reinterpret_cast<uint8_t*>(&GNSSDataForEMMC), eMMC::memoryPageSize, eMMCDataTailPointer, 1);

                                eMMCDataTailPointer++;

                                NumberOfMeasurementsInStruct = 0;
                            }
                            LOG_DEBUG << "eMMC tail pointer for GNSS = " << eMMCDataTailPointer;
                            NumberOfMeasurementsInStruct++;
                            GNSSprint(compact);
                        } else {
                            __NOP();
                        }

                        gnss_error_timout_counter = 0;
                    }
                }
            }
        } else {
            LOG_ERROR << "Timeout waiting for GNSS message.";
            gnss_error_timout_counter++;
            if (gnss_error_timout_counter >= gnss_handler.ERROR_TIMEOUT_COUNTER_THRD) {
                LOG_ERROR << "Multiple GNSS timeouts, attempting to reset GNSS communication...Threshold: " << gnss_handler.ERROR_TIMEOUT_COUNTER_THRD;
                resetGNSSHardware();
                LOG_ERROR << "GNSS reset due to timeout";
                gnss_error_timout_counter = 0; // Reset the counter after corrective action
            }
        }
    }
}
