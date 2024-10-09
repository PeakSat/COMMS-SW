#pragma once

#include "Task.hpp"
#include "main.h"
#include "at86rf215.hpp"
#include "at86rf215config.hpp"
#include "queue.h"
#include <etl/optional.h>
#include <etl/array.h>

extern SPI_HandleTypeDef hspi4;

class TransceiverTask : public Task {
public:
    constexpr static uint16_t MaxPacketLength = 1024;
    using PacketType = etl::array<uint8_t, MaxPacketLength>;
    static AT86RF215::At86rf215 transceiver;
    uint8_t packet_id = 250;
    /**
     *  0 for TX, 1 for RX
     */
    int8_t txrx = -1;

    enum TransceiverState {
        IDLE,
        RX,
        TX
    };
    /**
     * Function to check the SPI functionality.
     * In fine working functionality, we get Part Number At86rf21.
     * @return 0 if it works fine, 1 otherwise.
     */
    uint8_t checkTheSPI();
    /**
     * This function creates random packets until we have full functionality.
     */
    PacketType createRandomPacket(uint16_t length);

    /**
    * This function calculates the PllChannelFrequency value using the formula given in the datasheet
    * for Fine Resolution Channel Scheme CNM.CM = 1 (section 6.3.2)
    */
    uint16_t calculatePllChannelFrequency09(uint32_t frequency);
    /**
     * This function calculates the PllChannelNumber value using the formula given in the datasheet
     * for Fine Resolution Channel Scheme CNM.CM = 1 (section 6.3.2)
     */
    uint8_t calculatePllChannelNumber09(uint32_t frequency);
    /**
     * This function sets the correct parameters for the PLL, PA ramp time and TX cut off frequency
     * @param pllFrequency09 first two bytes of the N (formula at section 6.3.2)
     * @param pllChannelNumber09 last byte of the N
     */
    void setConfiguration(uint16_t pllFrequency09, uint8_t pllChannelNumber09);
    /**
     *
     * @param enable true for turning ON the direct modulation
     */
    void directModConfigAndPreEmphasisFilter(bool enableDM, bool enablePE, bool recommended);
    /**
     * Sets the modulation parameters (index, order and B*T) and sets the direct modulation ON
     */
    void modulationConfig();
    /**
     * Sets the receiver front end parameters for modulation index 1 according to table 6-62.
     */
    void receiverConfig(bool agc_enabled);

    void txSRandTxFilter();

    void txAnalogFrontEnd();



    TransceiverTask() : Task("Transceiver signal transmission") {
    }

    void execute();

    void createTask() {
        xTaskCreateStatic(vClassTask < TransceiverTask > , this->TaskName,
                          TransceiverTask::TaskStackDepth, this, tskIDLE_PRIORITY + 1,
                          this->taskStack, &(this->taskBuffer));
    }

private:
    AT86RF215::AT86RF215Configuration CustomConfig;
    constexpr static uint16_t DelayMs = 1;
    constexpr static uint16_t TaskStackDepth = 6000;
    constexpr static uint32_t FrequencyUHF = 401000;

    AT86RF215::Error error;
    StackType_t taskStack[TaskStackDepth];
};

inline etl::optional<TransceiverTask> transceiverTask;