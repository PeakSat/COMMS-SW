#include "RF_RXTask.hpp"
#include "Logger.hpp"

using namespace AT86RF215;


uint8_t RF_RXTask::checkTheSPI() {
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

void RF_RXTask::execute() {
    LOG_DEBUG << "RF_RX TASK";
    // ENABLE THE RF 5V
    vTaskDelay(pdMS_TO_TICKS(5000));
    HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_SET);
    LOG_DEBUG << "RF 5V ENABLED";
    vTaskDelay(pdMS_TO_TICKS(1000));
    HAL_GPIO_WritePin(RF_RST_GPIO_Port, RF_RST_Pin, GPIO_PIN_SET);
    LOG_DEBUG << "RF RESET ENABLED ";
    // CHECK THE SPI CONNECTION
    while (checkTheSPI() != 0) {
        vTaskDelay(10);
    };
    // ENABLE THE RX SWITCH
    HAL_GPIO_WritePin(EN_RX_UHF_GPIO_Port, EN_RX_UHF_Pin, GPIO_PIN_RESET);
    LOG_DEBUG << "RX SWITCH ENABLED ";
    vTaskDelay(pdMS_TO_TICKS(1000));

    uint8_t reg = transceiver.spi_read_8(AT86RF215::BBC0_PC, error);
    // ENABLE TXSFCS (FCS autonomously calculated)
    transceiver.spi_write_8(AT86RF215::BBC0_PC, reg | (1 << 4), error);
    // ENABLE FCS FILTER
    transceiver.spi_write_8(AT86RF215::BBC0_PC, reg | (1 << 6), error);
    reg = transceiver.spi_read_8(AT86RF215::BBC0_FSKC2, error);
    // DISABLE THE INTERLEAVING
    transceiver.spi_write_8(AT86RF215::BBC0_PC, reg & 0, error);
    //
    uint8_t temp = transceiver.spi_read_8(RF09_AUXS, error);
    transceiver.spi_write_8(RF09_AUXS, temp | (1 << 6), error);
    transceiver.spi_write_8(RF09_AUXS, temp | (0 << 5), error);

    uint8_t read_reg;
    HAL_GPIO_WritePin(EN_UHF_AMP_RX_GPIO_Port, EN_UHF_AMP_RX_Pin, GPIO_PIN_RESET);
    LOG_DEBUG << "RX AMP DISABLED ";
    // DISABLE THE TX AMP
    HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_SET);
    //
    transceiver.setup(error);
    LOG_DEBUG << "RECEIVER";
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_RXBWC, error);
    LOG_DEBUG << "RF09_RXBWC = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_RXDFE, error);
    LOG_DEBUG << "RF09_RXDFE = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_AGCC, error);
    LOG_DEBUG << "RF09_AGCC = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_AGCS, error);
    LOG_DEBUG << "RF09_AGCS = " << read_reg;
    int read_rssi = transceiver.int_spi_read_8(AT86RF215::RF09_RSSI, error);
    LOG_DEBUG << "RF09_RSSI = " << read_rssi;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_EDC, error);
    LOG_DEBUG << "RF09_EDC = " << read_reg;
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_EDD, error);
    LOG_DEBUG << "RF09_EDD = " << read_reg;
    int read_edv = transceiver.int_spi_read_8(AT86RF215::RF09_EDV, error);
    LOG_DEBUG << "RF09_EDV = " << read_edv;
    LOG_DEBUG << "DELAY";
    vTaskDelay(3000);
    transceiver.rxConfig.setRXBWC(ReceiverBandwidth::RF_BW250KHZ_IF250KHZ, 1, 1);
    transceiver.setup(error);
    read_reg = transceiver.spi_read_8(AT86RF215::RF09_RXBWC, error);
    LOG_DEBUG << "RF09_RXBWC = " << read_reg;
}
