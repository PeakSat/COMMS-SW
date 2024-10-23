#include "TransceiverTask.hpp"
#include "Logger.hpp"


using namespace AT86RF215;

extern SPI_HandleTypeDef hspi4;

using namespace AT86RF215;

AT86RF215::At86rf215 TransceiverTask::transceiver = AT86RF215::At86rf215(&hspi4, AT86RF215::AT86RF215Configuration());

uint8_t TransceiverTask::checkTheSPI() {
	uint8_t spi_error = 0;
	DevicePartNumber dpn = transceiver.get_part_number(error);
	if (dpn == DevicePartNumber::AT86RF215)
		LOG_DEBUG << "SPI OK";
	else {
		spi_error = 1;
		LOG_DEBUG << "SPI ERROR";
		transceiver.chip_reset(error);
	}
	return spi_error;
}

TransceiverTask::PacketType TransceiverTask::createRandomPacket(uint16_t length) {
	PacketType packet;
	packet[0] = packet_id;
	for (std::size_t i = 1; i < length; i++) {
		packet[i] = i;
	}
	return packet;
}

void TransceiverTask::setConfiguration(uint16_t pllFrequency09, uint8_t pllChannelNumber09) {
	CustomConfig.pllFrequency09 = pllFrequency09;
	CustomConfig.pllChannelNumber09 = pllChannelNumber09;
	CustomConfig.pllChannelMode09 = AT86RF215::PLLChannelMode::FineResolution450;
	CustomConfig.transceiverSampleRate09 = AT86RF215::TransmitterSampleRate::FS_400;
	CustomConfig.continuousTransmit09 = false;
	CustomConfig.baseBandEnable09 = true;
	CustomConfig.physicalLayerType09 = PhysicalLayerType::BB_MRFSK;
	CustomConfig.frameCheckSequenceType09 = FrameCheckSequenceType::FCS_32;
	CustomConfig.txOutPower09 = 0;
	transceiver.config = CustomConfig;
}

void TransceiverTask::receiverConfig(bool agc_enable) {
	transceiver.setup_rx_frontend(AT86RF215::RF09,
	                              false,
	                              false,
	                              AT86RF215::ReceiverBandwidth::RF_BW200KHZ_IF250KHZ,
	                              AT86RF215::RxRelativeCutoffFrequency::FCUT_0375,
	                              AT86RF215::ReceiverSampleRate::FS_400,
	                              false,
	                              AT86RF215::AverageTimeNumberSamples::AVGS_8,
	                              false,
	                              AT86RF215::AutomaticGainTarget::DB30,
	                              23,
	                              error);
	// ENABLE AGC
	RegisterAddress regagcc = AT86RF215::RF09_AGCC;
	uint8_t reg = transceiver.spi_read_8(regagcc, error);
	reg = reg | (static_cast<uint8_t>(agc_enable));
	transceiver.spi_write_8(regagcc, reg, error);

	transceiver.setup_rssi(AT86RF215::RF09,
	                       AT86RF215::EnergyDetectionMode::RF_EDAUTO,
	                       16,
	                       AT86RF215::EnergyDetectionTimeBasis::RF_8MS,
	                       error);
}

/*
* The frequency cannot be lower than 377000 as specified in section 6.3.2. The frequency range related
* to Fine Resolution Channel Scheme CNM.CM=1 is from 389.5MHz to 510MHz
*/
uint16_t TransceiverTask::calculatePllChannelFrequency09(uint32_t frequency) {
	uint32_t N = (frequency - 377000) * 65536 / 6500;
	return N >> 8;
}

/*
* The frequency cannot be lower than 377000 as specified in section 6.3.2. The frequency range related
* to Fine Resolution Channel Scheme CNM.CM = 1 is from 389.5MHz to 510MHz
*/
uint8_t TransceiverTask::calculatePllChannelNumber09(uint32_t frequency) {
	uint32_t N = (frequency - 377000) * 65536 / 6500;
	return N & 0xFF;
}

void TransceiverTask::directModConfigAndPreEmphasisFilter(bool enableDM, bool enablePE, bool recommended) {
	if (enableDM) {
		transceiver.set_direct_modulation(RF09, enableDM, error);
		transceiver.spi_write_8(BBC0_FSKDM, 0b00000001, error);
		if (enablePE) {
			transceiver.spi_write_8(BBC0_FSKDM, 0b00000011, error);
			if (recommended) {
				transceiver.spi_write_8(BBC0_FSKPE0, 0x2, error);
				transceiver.spi_write_8(BBC0_FSKPE1, 0x3, error);
				transceiver.spi_write_8(BBC0_FSKPE2, 0xFC, error);
			}
		}
	}
}

void TransceiverTask::txSRandTxFilter() {
	AT86RF215::RegisterAddress regtxdfe;
	regtxdfe = AT86RF215::RF09_TXDFE;

	uint8_t reg = transceiver.spi_read_8(regtxdfe, error);
	// RCUT CONFIG //
	transceiver.spi_write_8(regtxdfe, reg | (static_cast<uint8_t>(0x01) << 5), error);
	// SR CONFIG //
	reg = transceiver.spi_read_8(regtxdfe, error);
	transceiver.spi_write_8(regtxdfe, reg | (static_cast<uint8_t>(0xA)), error);
}

void TransceiverTask::txAnalogFrontEnd() {
	AT86RF215::RegisterAddress regtxcutc;
	regtxcutc = AT86RF215::RF09_TXCUTC;

	uint8_t reg = transceiver.spi_read_8(regtxcutc, error);
	// PARAMP Config
	transceiver.spi_write_8(regtxcutc, reg | (static_cast<uint8_t>(AT86RF215::PowerAmplifierRampTime::RF_PARAMP32U) << 6), error);
	// LPFCUT Config
	transceiver.spi_write_8(regtxcutc, reg | (static_cast<uint8_t>(AT86RF215::TransmitterCutOffFrequency::RF_FLC80KHZ)), error);
}

void TransceiverTask::modulationConfig() {
	// BT = 1 , MIDXS = 1, MIDX = 1, MOR = B-FSK
	transceiver.spi_write_8(BBC0_FSKC0, 86, error);
	directModConfigAndPreEmphasisFilter(true, false, false);
}

void TransceiverTask::execute() {
	// ENABLE THE RF 5V
	vTaskDelay(pdMS_TO_TICKS(10000));
	HAL_GPIO_WritePin(P5V_RF_EN_GPIO_Port, P5V_RF_EN_Pin, GPIO_PIN_SET);
	LOG_DEBUG << "RF 5V ENABLED ";
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

	uint8_t temp = transceiver.spi_read_8(RF09_AUXS, error);
	transceiver.spi_write_8(RF09_AUXS, temp | (1 << 6), error);
	transceiver.spi_write_8(RF09_AUXS, temp | (0 << 5), error);

	setConfiguration(calculatePllChannelFrequency09(FrequencyUHF), calculatePllChannelNumber09(FrequencyUHF));
	transceiver.chip_reset(error);
	transceiver.setup(error);
	txAnalogFrontEnd();
	txSRandTxFilter();
	modulationConfig();
	receiverConfig(true);

	uint16_t currentPacketLength = MaxPacketLength;
	PacketType packet = createRandomPacket(MaxPacketLength);

	uint8_t low_length_byte = 0, high_length_byte = 0;
	uint16_t received_length = 0;
	uint32_t ok_packets = 0, wrong_packets = 0, sent_packets = 0;
	uint32_t current_ticks, elapsed_time, initial_ticks;

	const uint32_t idle_interval = 10000; // 54 seconds for thermal balance // 1 seconds for the extreme temps
	const uint32_t rx_interval = 3000;    // 3 seconds for thermal balance // 150000 for the extreme temps
	const uint32_t tx_interval = 3000;    // 3 seconds for thermal balance // 150000 for the extreme temps

	TransceiverState transceiver_state = IDLE;
	txrx = -1;
	initial_ticks = HAL_GetTick();
	while (true) {
		//        LOG_DEBUG << "{START OF" << this->TaskName << "}" ;
		current_ticks = HAL_GetTick();
		elapsed_time = current_ticks - initial_ticks;
		switch (transceiver_state) {
			case IDLE:
				if (elapsed_time >= idle_interval) {
					txrx = 1;
					transceiver_state = RX;
					initial_ticks = current_ticks;
					LOG_DEBUG << "Switching to RX mode";
					// ENABLE THE RX AMP
					HAL_GPIO_WritePin(EN_UHF_AMP_RX_GPIO_Port, EN_UHF_AMP_RX_Pin, GPIO_PIN_SET);
					LOG_DEBUG << "RX AMP ENABLED ";
					// DISABLE THE TX AMP
					HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_SET);
					LOG_DEBUG << "TX AMP DISABLED";
					setConfiguration(calculatePllChannelFrequency09(FrequencyUHF), calculatePllChannelNumber09(FrequencyUHF));
					transceiver.chip_reset(error);
					transceiver.setup(error);
					txAnalogFrontEnd();
					txSRandTxFilter();
					modulationConfig();
					receiverConfig(true);
					transceiver.set_state(AT86RF215::RF09, State::RF_TXPREP, error);
					vTaskDelay(pdMS_TO_TICKS(10));
					transceiver.set_state(AT86RF215::RF09, State::RF_RX, error);
					if (transceiver.get_state(AT86RF215::RF09, error) == (AT86RF215::State::RF_RX))
						LOG_DEBUG << " STATE = RX ";
				}
				break;
			case RX:
				if (elapsed_time >= rx_interval) {
					txrx = 0;
					transceiver_state = TX;
					initial_ticks = current_ticks;
					LOG_DEBUG << "Switching to TX mode";
					// DISABLE THE RX AMP
					HAL_GPIO_WritePin(EN_UHF_AMP_RX_GPIO_Port, EN_UHF_AMP_RX_Pin, GPIO_PIN_RESET);
					LOG_DEBUG << "RX AMP DISABLED";
					// ENABLE THE TX AMP
					HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_RESET);
					LOG_DEBUG << "TX AMP ENABLED";
					transceiver.TransmitterFrameEnd_flag = true;
					setConfiguration(calculatePllChannelFrequency09(FrequencyUHF), calculatePllChannelNumber09(FrequencyUHF));
					transceiver.chip_reset(error);
					transceiver.setup(error);
					txAnalogFrontEnd();
					txSRandTxFilter();
					modulationConfig();
				}
				break;
			case TX:
				if (elapsed_time >= tx_interval) {
					txrx = -1;
					transceiver_state = IDLE;
					initial_ticks = current_ticks;
					LOG_DEBUG << "Switching to IDLE mode";
					// DISABLE THE RX AMP
					HAL_GPIO_WritePin(EN_UHF_AMP_RX_GPIO_Port, EN_UHF_AMP_RX_Pin, GPIO_PIN_RESET);
					LOG_DEBUG << "RX AMP DISABLED ";
					// DISABLE THE TX AMP
					HAL_GPIO_WritePin(EN_PA_UHF_GPIO_Port, EN_PA_UHF_Pin, GPIO_PIN_SET);
					LOG_DEBUG << "TX AMP DISABLED ";
				}
				break;
		}
		if ((transceiverTask->txrx == 1) && transceiver.ReceiverFrameEnd_flag) {
			transceiver.ReceiverFrameEnd_flag = false;
			// Filtering the received packets //
			low_length_byte = transceiver.spi_read_8(AT86RF215::BBC0_RXFLL, error);
			high_length_byte = transceiver.spi_read_8(AT86RF215::BBC0_RXFLH, error);
			received_length = (high_length_byte << 8) | low_length_byte;

			if (transceiver.spi_read_8(AT86RF215::BBC0_FBRXS, error) == packet_id && received_length == MaxPacketLength) {
				ok_packets++;
				LOG_DEBUG << "PACKET RECEPTION OK, " << ok_packets;
			} else {
				wrong_packets++;
				LOG_DEBUG << "ERROR IN PACKET RECEPTION, " << wrong_packets;
				for (int i = 0; i < MaxPacketLength; i++)
					LOG_DEBUG << transceiver.spi_read_8((AT86RF215::BBC0_FBRXS) + i, error);
			}

			transceiver.set_state(AT86RF215::RF09, State::RF_TXPREP, error);
			vTaskDelay(pdMS_TO_TICKS(10));
			transceiver.set_state(AT86RF215::RF09, State::RF_RX, error);
		}
		if (transceiverTask->txrx == 0 && transceiver.TransmitterFrameEnd_flag) {
			sent_packets++;
			transceiver.transmitBasebandPacketsTx(AT86RF215::RF09, packet.data(), currentPacketLength, error);
			vTaskDelay(pdMS_TO_TICKS(200));
			transceiver.set_state(AT86RF215::RF09, State::RF_TX, error);
			transceiver.TransmitterFrameEnd_flag = false;
			LOG_DEBUG << "PACKET IS SENT " << sent_packets;
		}
		//        LOG_DEBUG << "{END OF" << this->TaskName << "}" ;
	}
}