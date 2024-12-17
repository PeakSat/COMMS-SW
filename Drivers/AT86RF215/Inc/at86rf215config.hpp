#pragma once
#include "at86rf215definitions.hpp"
#include <cstdint>
#include <etl/array.h>

namespace AT86RF215 {

    struct RXConfig {
        // RFn_RXBWC
        ReceiverBandwidth receiverBandwidth09, receiverBandwidth24;
        bool ifInversion09, ifInversion24;
        bool ifShift09, ifShift24;
        // RFn_RXDFE
        RxRelativeCutoffFrequency rxRelativeCutoffFrequency09, rxRelativeCutoffFrequency24;
        ReceiverSampleRate receiverSampleRate09, receiverSampleRate24;
        /// RFn_AGCC
        bool agcInput09, agcInput24;
        AverageTimeNumberSamples averageTimeNumberSamples09, averageTimeNumberSamples24;
        AGCReset agcReset_09, agcReset_24;
        AGCFreezeControl agcFreezeControl_09, agcFreezeControl_24;
        AGCEnable agcEnabled09, agcEnabled24;
        AutomaticGainTarget automaticGainTarget09, automaticGainTarget24;
        uint8_t gainControlWord09, gainControlWord24;
        /// RFn_EDC RFn_RDD
        EnergyDetectionMode energyDetectionMode09, energyDetectionMode24;
        uint8_t energyDetectDurationFactor09, energyDetectDurationFactor24;
        EnergyDetectionTimeBasis energyDetectionBasis09, energyDetectionBasis24;
        static RXConfig DefaultRXConfig() {
            return {
                /// RFn_RXBWC
                .receiverBandwidth09 = ReceiverBandwidth::RF_BW160KHZ_IF250KHZ,
                .receiverBandwidth24 = ReceiverBandwidth::RF_BW160KHZ_IF250KHZ,
                .ifInversion09 = false,
                .ifInversion24 = false,
                .ifShift09 = false,
                .ifShift24 = false,
                /// RFn_RXDFE
                .rxRelativeCutoffFrequency09 = RxRelativeCutoffFrequency::FCUT_0375,
                .rxRelativeCutoffFrequency24 = RxRelativeCutoffFrequency::FCUT_0375,
                .receiverSampleRate09 = ReceiverSampleRate::FS_400,
                .receiverSampleRate24 = ReceiverSampleRate::FS_400,
                /// RFn_AGCC
                .agcInput09 = false,
                .agcInput24 = false,
                .averageTimeNumberSamples09 = AverageTimeNumberSamples::AVGS_16,
                .averageTimeNumberSamples24 = AverageTimeNumberSamples::AVGS_8,
                .agcReset_09 = AGCReset::default_agc_reset,
                .agcReset_24 = AGCReset::default_agc_reset,
                .agcFreezeControl_09 = AGCFreezeControl::no_freeze,
                .agcFreezeControl_24 = AGCFreezeControl::no_freeze,
                .agcEnabled09 = AGCEnable::agc_enabled,
                .agcEnabled24 = AGCEnable::agc_disabled,
                /// RF_AGCS
                .automaticGainTarget09 = AutomaticGainTarget::DB30,
                .automaticGainTarget24 = AutomaticGainTarget::DB30,
                /// Maximum Receive Gain
                .gainControlWord09 = 23,
                /// RFn_EDC // RFn_EDD //
                .energyDetectionMode09 = EnergyDetectionMode::RF_EDSINGLE,
                .energyDetectionMode24 = EnergyDetectionMode::RF_EDAUTO,
                .energyDetectDurationFactor09 = 0x10,
                .energyDetectDurationFactor24 = 0x10,
                .energyDetectionBasis09 = EnergyDetectionTimeBasis::RF_8MS,
                .energyDetectionBasis24 = EnergyDetectionTimeBasis::RF_8MS,

            };
        }

        // update params
        void setRXBWC(ReceiverBandwidth bw09, bool inversion09, bool shift09) {
            receiverBandwidth09 = bw09;
            ifInversion09 = inversion09;
            ifShift09 = shift09;
        }

        //
        void setRXDFE(RxRelativeCutoffFrequency cutoff09, ReceiverSampleRate sampleRate09) {
            rxRelativeCutoffFrequency09 = cutoff09;
            receiverSampleRate09 = sampleRate09;
        }

        //
        void setEDC(EnergyDetectionTimeBasis timeBasis09, EnergyDetectionMode mode09,
                    uint8_t detectFactor09) {
            energyDetectionBasis09 = timeBasis09;
            energyDetectionMode09 = mode09;
            energyDetectDurationFactor09 = detectFactor09;
        }

        //
        void setAGCC(bool input09, AverageTimeNumberSamples avgSamples09, AGCEnable enabled09,
                     AutomaticGainTarget target09) {
            agcInput09 = input09;
            averageTimeNumberSamples09 = avgSamples09;
            agcEnabled09 = enabled09;
            automaticGainTarget09 = target09;
        }
    };

    struct TXConfig {
        // RFn_TXDFE
        TxRelativeCutoffFrequency txRelativeCutoffFrequency09, txRelativeCutoffFrequency24;
        Direct_Mod_Enable_FSKDM directModulation09, directModulation24;
        TransmitterSampleRate transceiverSampleRate09, transceiverSampleRate24;
        // RFn_TXCUTC
        PowerAmplifierRampTime powerAmplifierRampTime09, powerAmplifierRampTime24;
        TransmitterCutOffFrequency transmitterCutOffFrequency09, transmitterCutOffFrequency24;
        // RFn_PAC
        PowerAmplifierCurrentControl powerAmplifierCurrentControl09, powerAmplifierCurrentControl24;
        uint8_t txOutPower09, txOutPower24;

        static TXConfig DefaultTXConfig() {
            return {
                // RFn_TXDFE
                .txRelativeCutoffFrequency09 = TxRelativeCutoffFrequency::FCUT_0375,
                .directModulation09 = Direct_Mod_Enable_FSKDM::direct_mod_enabled,
                .transceiverSampleRate09 = TransmitterSampleRate::FS_400,
                // RFn_TXCUTC
                .powerAmplifierRampTime09 = PowerAmplifierRampTime::RF_PARAMP4U,
                .transmitterCutOffFrequency09 = TransmitterCutOffFrequency::RF_FLC100KHZ,
                // RF_n_PAC
                .powerAmplifierCurrentControl09 = PowerAmplifierCurrentControl::PA_NO,
                .txOutPower09 = 0x00};
        }
        void setTXDFE(TxRelativeCutoffFrequency cutoffFrequency09, Direct_Mod_Enable_FSKDM modulation09, TransmitterSampleRate sampleRate09) {
            txRelativeCutoffFrequency09 = cutoffFrequency09;
            directModulation09 = modulation09;
            transceiverSampleRate09 = sampleRate09;
        }
        void setTXCUTC(PowerAmplifierRampTime rampTime09, TransmitterCutOffFrequency cutoffFrequency09) {
            powerAmplifierRampTime09 = rampTime09;
            transmitterCutOffFrequency09 = cutoffFrequency09;
        }
        void setRFnPAC(PowerAmplifierCurrentControl currentControl09, uint8_t outPower09) {
            powerAmplifierCurrentControl09 = currentControl09;
            txOutPower09 = outPower09;
        }
    };

    struct BasebandCoreConfig {
        // BBCn_PC
        bool continuousTransmit09, continuousTransmit24;
        bool frameCheckSequenceFilterEn09, frameCheckSequenceFilterEn24;
        bool transmitterAutoFrameCheckSequence09, transmitterAutoFrameCheckSequence24;
        FrameCheckSequenceType frameCheckSequenceType09, frameCheckSequenceType24;
        bool baseBandEnable09, baseBandEnable24;
        PhysicalLayerType physicalLayerType09, physicalLayerType24;
        /// BBCn_FSKCO
        Bandwidth_time_product bandwidth_time_09, bandwidth_time_24;
        Mod_index_scale midxs_09, midxs_24;
        Mod_index midx_09, midx_24;
        FSK_mod_order mord_09, mord_24;
        /// BBCn_FSKC1
        Freq_Inversion freq_inv_09, freq_inv_24;
        MR_FSK_symbol_rate sr_09, sr_24;
        /// BBCn_FSKC2
        Preamble_Detection preamble_detection_09, preamble_detection_24;
        Receiver_Override receiver_override_09, receiver_override_24;
        Receiver_Preamble_Timeout receiver_preamble_timeout_09, receiver_preamble_timeout_24;
        Mode_Switch_Enable mode_switch_en_09, mode_switch_en_24;
        Preamble_Inversion preamble_inversion_09, preamble_inversion_24;
        FEC_Scheme fec_scheme_09, fec_scheme_24;
        Interleaving_Enable interleaving_enable_09, interleaving_enable_24;
        /// BBCn_FSKC3
        SFD_Detection_Threshold sfdt_09, sfdt_24;
        Preamble_Detection_Threshold prdt_09, prdt_24;
        /// BBCn_FSKC4
        SFD_Quantization sfdQuantization_09, sfdQuantization_24;
        SFD_32 sfd32_09, sfd32_24;
        Raw_Mode_Reversal_Bit rawModeReversalBit_09, rawModeReversalBit_24;
        CSFD1 csfd1_09, csfd1_24;
        CSFD0 csfd0_09, csfd0_24;
        /// BBCn_FSKPHRTX
        SFD_Used sfdUsed_09, sfdUsed_24;
        Data_Whitening dataWhitening_09, dataWhitening_24;
        /// BBCn_FSKDM
        FSK_Preamphasis_Enable fskPreamphasisEnable_09, fskPreamphasisEnable_24;
        Direct_Mod_Enable_FSKDM directModEnableFskdm_09, directModEnableFskdm_24;

        static BasebandCoreConfig DefaultBasebandCoreConfig() {
            return {
                /// BBCn_PC
                .continuousTransmit09 = false,
                .frameCheckSequenceFilterEn09 = false,
                .transmitterAutoFrameCheckSequence09 = true,
                .frameCheckSequenceType09 = FrameCheckSequenceType::FCS_32,
                .baseBandEnable09 = true,
                .baseBandEnable24 = false,
                .physicalLayerType09 = PhysicalLayerType::BB_MRFSK,
                .physicalLayerType24 = PhysicalLayerType::BB_OFF,
                /// BBCn_FSKC0
                .bandwidth_time_09 = Bandwidth_time_product::BT_1_0,
                .midxs_09 = Mod_index_scale::s_1_0,
                .midx_09 = Mod_index::bf_1_000,
                .mord_09 = FSK_mod_order::binary_fsk,
                /// BBCn_FSKC1
                .freq_inv_09 = Freq_Inversion::freq_inversion_off,
                .sr_09 = MR_FSK_symbol_rate::sr_50,
                /// BBCn_FSKC2
                .preamble_detection_09 = Preamble_Detection::preamble_det_without_rssi,
                .receiver_override_09 = Receiver_Override::restart_by_18db_stronger_frame,
                .receiver_preamble_timeout_09 = Receiver_Preamble_Timeout::timeout_disabled,
                .mode_switch_en_09 = Mode_Switch_Enable::disabled,
                .preamble_inversion_09 = Preamble_Inversion::no_inversion,
                .fec_scheme_09 = FEC_Scheme::NRNSC,
                .interleaving_enable_09 = Interleaving_Enable::enabled,
                /// BBCn_FSKC3
                .sfdt_09 = SFD_Detection_Threshold::default_sfd_IEEE,
                .prdt_09 = Preamble_Detection_Threshold::increased_preamble_sensitivity,
                /// BBCn_FSC4
                .sfdQuantization_09 = SFD_Quantization::SOFT_DECISION,
                .sfd32_09 = SFD_32::TWO_16BIT_SFD,
                .rawModeReversalBit_09 = Raw_Mode_Reversal_Bit::MSB_FIRST,
                .csfd1_09 = CSFD1::CODED_IEEE_MODE,
                .csfd0_09 = CSFD0::UNCODED_IEEE_MODE,
                /// BBCn_FSKPHRTX
                .sfdUsed_09 = SFD_Used::sfd1_used,
                .dataWhitening_09 = Data_Whitening::psdu_data_whitening_disabled,
                /// BBCn_FSKDM
                .fskPreamphasisEnable_09 = FSK_Preamphasis_Enable::preamphasis_disabled,
                .directModEnableFskdm_09 = Direct_Mod_Enable_FSKDM::direct_mod_enabled,
            };
        }
        /// BBC_PC
        void setBBC_PC(bool ct09, bool fcsfEn09, bool tautoFcs09, FrameCheckSequenceType fcsType09, bool bbEn09, PhysicalLayerType plType09) {
            continuousTransmit09 = ct09;
            frameCheckSequenceFilterEn09 = fcsfEn09;
            transmitterAutoFrameCheckSequence09 = tautoFcs09;
            frameCheckSequenceType09 = fcsType09;
            baseBandEnable09 = bbEn09;
            physicalLayerType09 = plType09;
        }
        /// BBC_FSKC0
        void setBBC_FSKC0(Bandwidth_time_product bwTime09, Mod_index_scale midxs09,
                          Mod_index midx09, FSK_mod_order mord09) {
            bandwidth_time_09 = bwTime09;
            midxs_09 = midxs09;
            midx_09 = midx09;
            mord_09 = mord09;
        }
        /// BBC_FSKC1
        void setBBC_FSKC1(Freq_Inversion freqInv09, MR_FSK_symbol_rate sr09) {
            freq_inv_09 = freqInv09;
            sr_09 = sr09;
        }
        /// BBC_FSKC2
        void setBBC_FSKC2(Preamble_Detection preambleDet09, Receiver_Override recOverride09,
                          Receiver_Preamble_Timeout recPreambleTimeout09, Mode_Switch_Enable modeSwitchEn09,
                          Preamble_Inversion preambleInv09, FEC_Scheme fecScheme09,
                          Interleaving_Enable interleavingEn09) {
            preamble_detection_09 = preambleDet09;
            receiver_override_09 = recOverride09;
            receiver_preamble_timeout_09 = recPreambleTimeout09;
            mode_switch_en_09 = modeSwitchEn09;
            preamble_inversion_09 = preambleInv09;
            fec_scheme_09 = fecScheme09;
            interleaving_enable_09 = interleavingEn09;
        }
        /// BBC_FSKC3
        void setBBC_FSKC3(SFD_Detection_Threshold sfdDetectionThreshold, Preamble_Detection_Threshold preambleDetectionThreshold) {
            sfdt_09 = sfdDetectionThreshold;
            prdt_09 = preambleDetectionThreshold;
        }
        /// BBC_FSKC4
        void setBBC_FSKC4(SFD_Quantization sfdQuantization, SFD_32 sfd32,
                          Raw_Mode_Reversal_Bit rawModeReversalBit,
                          CSFD1 csfd1, CSFD0 csfd2) {
            // Set values for the 09 band
            sfdQuantization_09 = sfdQuantization;
            sfd32_09 = sfd32;
            rawModeReversalBit_09 = rawModeReversalBit;
            csfd1_09 = csfd1;
            csfd0_09 = csfd2;
        }
        /// BBCn_FSKPHRTX
        void set_BBC_FSKPHRTX(SFD_Used sfdused, Data_Whitening dataWhitening) {
            sfdUsed_09 = sfdused;
            dataWhitening_09 = dataWhitening;
        }
        /// BBCn_FSKPHRTX
        void set_BBC_FSKDM(FSK_Preamphasis_Enable fskPreamphasisEnable, Direct_Mod_Enable_FSKDM directModEnableFskdm) {
            fskPreamphasisEnable_09 = fskPreamphasisEnable;
            directModEnableFskdm_09 = directModEnableFskdm;
        }
    };

    struct FrequencySynthesizer {
        /// Cached frequency for easy access
        uint32_t frequency; // Frequency in kHz
        /// RF_n CS
        uint8_t channelSpacing09, channelSpacing24;
        /// RFn_CCFOL - Channel Center Frequency F0 Low Byte
        /// RFn_CCFOH - Channel Center Frequency F0 High Byte
        /// combined RFn_CCF0L, RFn_CCFOH : channelCenterFrequency09
        uint16_t channelCenterFrequency09;
        /// RFn_CNL
        uint8_t channelNumber09;
        /// RFn_CNM
        PLLChannelMode channelMode09, channelMode24;
        /// RFn_PLL
        PLLBandwidth loopBandwidth09, loopBandwidth24;
        /// RFn_PLLCF

        static FrequencySynthesizer DefaultFrequencySynthesizerConfig() {
            FrequencySynthesizer fs{
                .frequency = 401000,
                // spacing = channelSpacing * 25kHz
                .channelSpacing09 = 0x2,
                .channelSpacing24 = 0xA,
                .channelMode09 = PLLChannelMode::FineResolution450,
                .channelMode24 = PLLChannelMode::FineResolution2443,
                .loopBandwidth09 = PLLBandwidth::BWLarger,
                .loopBandwidth24 = PLLBandwidth::BWDefault,
            };
            fs.setFrequency_FineResolution_CMN_1(fs.frequency);
            return fs;
        }
        /// Helper to calculate N_channel
        uint32_t calculateN_FineResolution_CMN_1(uint32_t freq) {
            return (freq - 377000) * 65536 / 6500;
        }
        /// Set frequency and calculate corresponding CCF0 and CNL
        void setFrequency_FineResolution_CMN_1(uint32_t freq) {
            frequency = freq;
            uint32_t N = calculateN_FineResolution_CMN_1(frequency);
            /// Combine CCF0H and CCF0L into a single 16-bit value
            channelCenterFrequency09 = (N >> 8) & 0xFFFF; // Take bits 8-23
            channelNumber09 = N & 0xFF;                   // Extract the lowest byte (bits 0-7)
        }

        // Get frequency based on CCF0 and CNL
        double getFrequency_FineResolution_CMN_1() {
            uint32_t N_channel = ((uint32_t) channelCenterFrequency09 << 8) | channelNumber09; // Reconstruct full N_channel
            return 377000.0 + (6500.0 * N_channel) / 65536.0;
        }
        //
        etl::array<uint8_t, 3> getFrequency_in_bytes() {
            etl::array<uint8_t, 3> arr{};                                   // Use etl::array for embedded compatibility
            arr[2] = channelNumber09;                                       // Channel Number
            arr[1] = static_cast<uint8_t>(channelCenterFrequency09 >> 8);   // Upper byte
            arr[0] = static_cast<uint8_t>(channelCenterFrequency09 & 0xFF); // Lower byte
            return arr;                                                     // Safe return by value
        }
    };

    struct ExternalFrontEndConfig {
        /// RFn_AUXS
        ExternalLNABypass externalLNABypass09, externalLNABypass24;
        AutomaticGainControlMAP automaticGainControlMAP09, automaticGainControlMAP24;
        AnalogVoltageEnable analogVoltageEnable09, analogVoltageEnable24;
        AutomaticVoltageExternal automaticVoltageExternal09, automaticVoltageExternal24;
        PowerAmplifierVoltageControl powerAmplifierVoltageControl09, powerAmplifierVoltageControl24;
        /// RFn_PADFE
        ExternalFrontEndControl externalFrontEnd_09, externalFrontEnd_24;
        static ExternalFrontEndConfig DefaultExternalFrontEndConfig() {
            return {
                .automaticGainControlMAP09 = AutomaticGainControlMAP::AGC_BACKOFF_12,
                .analogVoltageEnable09 = AnalogVoltageEnable::ENABLED,
                .automaticVoltageExternal09 = AutomaticVoltageExternal::DISABLED,
                .powerAmplifierVoltageControl09 = PowerAmplifierVoltageControl::PAVC_2V4,
                .externalFrontEnd_09 = ExternalFrontEndControl::front_end_config_txrx_switch};
        }
        void set_RFn_AUXS(
            ExternalLNABypass extLNA09,            // externalLNABypass09
            AutomaticGainControlMAP agcMap09,      // automaticGainControlMAP09
            AnalogVoltageEnable avEn09,            // analogVoltageEnable09
            AutomaticVoltageExternal avExt09,      // automaticVoltageExternal09
            PowerAmplifierVoltageControl pavCtrl09 // powerAmplifierVoltageControl09
        ) {
            externalLNABypass09 = extLNA09;
            automaticGainControlMAP09 = agcMap09;
            analogVoltageEnable09 = avEn09;
            automaticVoltageExternal09 = avExt09;
            powerAmplifierVoltageControl09 = pavCtrl09;
        }
    };

    struct IQInterfaceConfig {
        // IQ Interface
        // RF_IQIFC0
        ExternalLoopback externalLoopback;
        IQOutputCurrent iqOutputCurrent;
        IQmodeVoltage iqmodeVoltage;
        IQmodeVoltageIEE iqmodeVoltageIEE;
        EmbeddedControlTX embeddedControlTX;
        // RF_IQIFC1
        ChipMode chipMode;
        SkewAlignment skewAlignment;

        static IQInterfaceConfig DefaultIQInterfaceConfig() {
            return {
                // RF_IQIFC0
                .externalLoopback = ExternalLoopback::DISABLED,
                .iqOutputCurrent = IQOutputCurrent::CURR_2_MA,
                .iqmodeVoltage = IQmodeVoltage::MODE_200_MV,
                .iqmodeVoltageIEE = IQmodeVoltageIEE::CMV,
                .embeddedControlTX = EmbeddedControlTX::DISABLED,
                // RF_IQIFC1
                .chipMode = ChipMode::RF_MODE_BBRF,
                .skewAlignment = SkewAlignment::SKEW3906NS};
        }
    };

    struct InterruptsConfig {
        /// BBCn_IRQM
        bool frameBufferLevelIndication09, frameBufferLevelIndication24;
        bool agcRelease09, agcRelease24;
        bool agcHold09, agcHold24;
        bool transmitterFrameEnd09, transmitterFrameEnd24;
        bool receiverExtendedMatch09, receiverExtendedMatch24;
        bool receiverAddressMatch09, receiverAddressMatch24;
        bool receiverFrameEnd09, receiverFrameEnd24;
        bool receiverFrameStart09, receiverFrameStart24;

        static InterruptsConfig DefaultInterruptsConfig() {
            return {
                .frameBufferLevelIndication09 = true,
                .frameBufferLevelIndication24 = false,
                .agcRelease09 = true,
                .agcRelease24 = false,
                .agcHold09 = false,
                .agcHold24 = false,
                .transmitterFrameEnd09 = true,
                .transmitterFrameEnd24 = false,
                .receiverExtendedMatch09 = true,
                .receiverExtendedMatch24 = false,
                .receiverAddressMatch09 = true,
                .receiverAddressMatch24 = false,
                .receiverFrameEnd09 = true,
                .receiverFrameEnd24 = false,
                .receiverFrameStart09 = true,
                .receiverFrameStart24 = false,
            };
        }
        void setupInterruptsConfig(bool fbl,
                                   bool ar,
                                   bool ah,
                                   bool tfe,
                                   bool rem,
                                   bool ram,
                                   bool rfe,
                                   bool rfs) {
            frameBufferLevelIndication09 = fbl;
            agcRelease09 = ar;
            agcHold09 = ah;
            transmitterFrameEnd09 = tfe;
            receiverExtendedMatch09 = rem;
            receiverAddressMatch09 = ram;
            receiverFrameEnd09 = rfe;
            receiverFrameStart09 = rfs;
        }
    };

    struct RadioInterruptsConfig {
        // RFn_IRQM
        bool iqIfSynchronizationFailure09, iqIfSynchronizationFailure24;
        bool transceiverError09, transceiverError24;
        bool batteryLow09, batteryLow24;
        bool energyDetectionCompletion09, energyDetectionCompletion24;
        bool transceiverReady09, transceiverReady24;
        bool wakeup09, wakeup24;

        static RadioInterruptsConfig DefaultRadioInterruptsConfig() {
            return {
                // RFn_IRQM
                .iqIfSynchronizationFailure09 = true,
                .iqIfSynchronizationFailure24 = false,
                .transceiverError09 = true,
                .transceiverError24 = false,
                .batteryLow09 = true,
                .batteryLow24 = false,
                .energyDetectionCompletion09 = false,
                .energyDetectionCompletion24 = false,
                .transceiverReady09 = true,
                .transceiverReady24 = false,
                .wakeup09 = true,
                .wakeup24 = false,
            };
        }
        // Setup function to configure values
        void setupRadioInterruptsConfig(bool syncFail,
                                        bool txErr,
                                        bool batLow,
                                        bool edComp,
                                        bool txReady,
                                        bool wake) {
            iqIfSynchronizationFailure09 = syncFail;
            transceiverError09 = txErr;
            batteryLow09 = batLow;
            energyDetectionCompletion09 = edComp;
            transceiverReady09 = txReady;
            wakeup09 = wake;
        }
    };

    struct GeneralConfiguration {

        // RFn_CFG
        bool irqMaskMode;
        IRQPolarity irqPolarity;
        PadDriverStrength padDriverStrength;

        // Battery Monitor
        BatteryMonitorVoltage batteryMonitorVoltage =
            BatteryMonitorVoltage::BMHR_270_180;
        BatteryMonitorHighRange batteryMonitorHighRange =
            BatteryMonitorHighRange::LOW_RANGE;

        // Crystal oscillator
        CrystalTrim crystalTrim = CrystalTrim::TRIM_00;
        bool fastStartUp = false;

        static struct GeneralConfiguration DefaultGeneralConfig() {
            return {
                .irqMaskMode = true,
                .irqPolarity = IRQPolarity::ACTIVE_HIGH,
                .padDriverStrength = PadDriverStrength::RF_DRV4,
            };
        }
    };
} // namespace AT86RF215
