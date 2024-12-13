#include "RF_TXTask.hpp"
#include "Logger.hpp"

using namespace AT86RF215;


uint16_t RF_TXTask::calculatePllChannelFrequency09(uint32_t frequency) {
    uint32_t N = (frequency - 377000) * 65536 / 6500;
    return N >> 8;
}

/*
* The frequency cannot be lower than 377000 as specified in section 6.3.2. The frequency range related
* to Fine Resolution Channel Scheme CNM.CM = 1 is from 389.5MHz to 510MHz
*/
uint8_t RF_TXTask::calculatePllChannelNumber09(uint32_t frequency) {
    uint32_t N = (frequency - 377000) * 65536 / 6500;
    return N & 0xFF;
}


uint8_t RF_TXTask::checkTheSPI() {
    uint8_t spi_error = 0;
    DevicePartNumber dpn = transceiver.get_part_number(error);
    etl::string<LOGGER_MAX_MESSAGE_SIZE> output;
    if (dpn == DevicePartNumber::AT86RF215) {
        output = "SPI IS OK";
        Logger::log(Logger::debug, output);
    }

    else {
        spi_error = 1;
        LOG_DEBUG << "SPI ERROR";
        transceiver.chip_reset(error);
    }
    return spi_error;
}

void RF_TXTask::execute() {
    vTaskDelay(10000);
    transceiver.setTXConfig(TXConfig::DefaultTXConfig());
    uint8_t read_reg = transceiver.spi_read_8(AT86RF215::RF09_TXDFE, error);
    LOG_DEBUG << "RF09_TXDFE = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_TXCUTC, error);
    LOG_DEBUG << "RF09_TXCUTC = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_PAC, error);
    LOG_DEBUG << "RF09_PAC = " << read_reg;

    transceiver.txConfig.setTXDFE(TxRelativeCutoffFrequency::FCUT_1, 1, TransmitterSampleRate::FS_2000);
    transceiver.setup(error);
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_TXDFE, error);
    LOG_DEBUG << "RF09_TXDFE = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_TXCUTC, error);
    LOG_DEBUG << "RF09_TXCUTC = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_PAC, error);
    LOG_DEBUG << "RF09_PAC = " << read_reg;
}
