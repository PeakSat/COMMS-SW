#pragma once
#include "at86rf215definitions.hpp"
#include <cstdint>

namespace AT86RF215 {

    struct RXConfig {
        // RFn_RXBWC
        ReceiverBandwidth receiverBandwidth09, receiverBandwidth24;
        bool ifInversion09, ifInversion24;
        bool ifShift09, ifShift24;
        // RFn_RXDFE
        RxRelativeCutoffFrequency rxRelativeCutoffFrequency09, rxRelativeCutoffFrequency24;
        ReceiverSampleRate receiverSampleRate09, receiverSampleRate24;
        // RFn_EDC
        EnergyDetectionTimeBasis energyDetectionBasis09, energyDetectionBasis24;
        EnergyDetectionMode energyDetectionMode09, energyDetectionMode24;
        uint8_t energyDetectFactor09, energyDetectFactor24;
        ReceiverEnergyDetectionAveragingDuration receiverEnergyDetectionAveragingDuration09, receiverEnergyDetectionAveragingDuration24;
        // RFn_AGCC
        bool agcInput09, agcInput24;
        AverageTimeNumberSamples averageTimeNumberSamples09, averageTimeNumberSamples24;
        bool agcEnabled09, agcEnabled24;
        AutomaticGainTarget automaticGainTarget09, automaticGainTarget24;
        uint8_t gainControlWord09, gainControlWord24;

        // default rx config
        static RXConfig DefaultRXConfig() {
            return {
                // RFn_RXBWC
                .receiverBandwidth09 = ReceiverBandwidth::RF_BW160KHZ_IF250KHZ,
                .receiverBandwidth24 = ReceiverBandwidth::RF_BW160KHZ_IF250KHZ,
                .ifInversion09 = false,
                .ifInversion24 = false,
                .ifShift09 = false,
                .ifShift24 = false,
                // RFn_RXDFE
                .rxRelativeCutoffFrequency09 = RxRelativeCutoffFrequency::FCUT_0375,
                .rxRelativeCutoffFrequency24 = RxRelativeCutoffFrequency::FCUT_0375,
                .receiverSampleRate09 = ReceiverSampleRate::FS_400,
                .receiverSampleRate24 = ReceiverSampleRate::FS_400,
                // RFn_EDC
                .energyDetectionBasis09 = EnergyDetectionTimeBasis::RF_8MS,
                .energyDetectionBasis24 = EnergyDetectionTimeBasis::RF_8MS,
                .energyDetectionMode09 = EnergyDetectionMode::RF_EDAUTO,
                .energyDetectionMode24 = EnergyDetectionMode::RF_EDAUTO,
                .energyDetectFactor09 = 0x10,
                .energyDetectFactor24 = 0x10,
                .receiverEnergyDetectionAveragingDuration09 = ReceiverEnergyDetectionAveragingDuration::REDAD_8U,
                .receiverEnergyDetectionAveragingDuration24 = ReceiverEnergyDetectionAveragingDuration::REDAD_8U,
                // RFn_AGCC
                .agcInput09 = false,
                .agcInput24 = false,
                .averageTimeNumberSamples09 = AverageTimeNumberSamples::AVGS_8,
                .averageTimeNumberSamples24 = AverageTimeNumberSamples::AVGS_8,
                .agcEnabled09 = true,
                .agcEnabled24 = false,
                // RF_AGCS
                .automaticGainTarget09 = AutomaticGainTarget::DB30,
                .automaticGainTarget24 = AutomaticGainTarget::DB30,
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
                    uint8_t detectFactor09, ReceiverEnergyDetectionAveragingDuration averaging09) {
            energyDetectionBasis09 = timeBasis09;
            energyDetectionMode09 = mode09;
            energyDetectFactor09 = detectFactor09;
            receiverEnergyDetectionAveragingDuration09 = averaging09;
        }

        //
        void setAGCC(bool input09, AverageTimeNumberSamples avgSamples09, bool enabled09,
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
        bool directModulation09, directModulation24;
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
                .txRelativeCutoffFrequency24 = TxRelativeCutoffFrequency::FCUT_0375,
                .directModulation09 = true,
                .directModulation24 = false,
                .transceiverSampleRate09 = TransmitterSampleRate::FS_400,
                .transceiverSampleRate24 = TransmitterSampleRate::FS_400,
                // RFn_TXCUTC
                .powerAmplifierRampTime09 = PowerAmplifierRampTime::RF_PARAMP4U,
                .powerAmplifierRampTime24 = PowerAmplifierRampTime::RF_PARAMP4U,
                .transmitterCutOffFrequency09 = TransmitterCutOffFrequency::RF_FLC100KHZ,
                .transmitterCutOffFrequency24 = TransmitterCutOffFrequency::RF_FLC100KHZ,
                // RF_n_PAC
                .powerAmplifierCurrentControl09 = PowerAmplifierCurrentControl::PA_NO,
                .powerAmplifierCurrentControl24 = PowerAmplifierCurrentControl::PA_NO,
                .txOutPower09 = 0x00,
                .txOutPower24 = 0x00};
        }
        void setTXDFE(TxRelativeCutoffFrequency cutoffFrequency09, bool modulation09, TransmitterSampleRate sampleRate09) {
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

        // BBCn_FSKO
        // BBCn_FSK1
        // BBCn_FSK2
        // BBCn_FSK3
        // BBCn_FSK4


        static BasebandCoreConfig DefaultBasebandCoreConfig() {
            return {
                // BBCn_PC
                .continuousTransmit09 = false,
                .continuousTransmit24 = false,
                .frameCheckSequenceFilterEn09 = false,
                .frameCheckSequenceFilterEn24 = false,
                .transmitterAutoFrameCheckSequence09 = true,
                .transmitterAutoFrameCheckSequence24 = true,
                .frameCheckSequenceType09 = FrameCheckSequenceType::FCS_32,
                .frameCheckSequenceType24 = FrameCheckSequenceType::FCS_32,
                .baseBandEnable09 = true,
                .baseBandEnable24 = false,
                .physicalLayerType09 = PhysicalLayerType::BB_MRFSK,
                .physicalLayerType24 = PhysicalLayerType::BB_OFF};
        }

        void setBBC_PC(bool ct09, bool fcsfEn09, bool tautoFcs09, FrameCheckSequenceType fcsType09, bool bbEn09, PhysicalLayerType plType09) {
            continuousTransmit09 = ct09;
            frameCheckSequenceFilterEn09 = fcsfEn09;
            transmitterAutoFrameCheckSequence09 = tautoFcs09;
            frameCheckSequenceType09 = fcsType09;
            baseBandEnable09 = bbEn09;
            physicalLayerType09 = plType09;
        }
    };

    struct FrequencySynthesizer {
        // Cached frequency for easy access
        uint32_t frequency; // Frequency in kHz
        // RF_n CS
        uint8_t channelSpacing09, channelSpacing24;
        // RFn_CCFOL - Channel Center Frequency F0 Low Byte
        // RFn_CCFOH - Channel Center Frequency F0 High Byte
        // combined RFn_CCF0L, RFn_CCFOH : channelCenterFrequency09
        uint16_t channelCenterFrequency09;
        // RFn_CNL
        uint8_t channelNumber09;
        // RFn_CNM
        PLLChannelMode channelMode09, channelMode24;
        // RFn_PLL
        PLLBandwidth loopBandwidth09, loopBandwidth24;
        // RFn_PLLCF

        static FrequencySynthesizer DefaultFrequencySynthesizerConfig() {
            FrequencySynthesizer fs{
                .frequency = 401000,
                // spacing = channelSpacing * 25kHz
                .channelSpacing09 = 0x08,
                .channelSpacing24 = 0xA,
                .channelMode09 = PLLChannelMode::FineResolution450,
                .channelMode24 = PLLChannelMode::FineResolution2443,
                .loopBandwidth09 = PLLBandwidth::BWDefault,
                .loopBandwidth24 = PLLBandwidth::BWDefault,
            };
            fs.setFrequency_FineResolution_CMN_1(fs.frequency);
            return fs;
        }
        // Helper to calculate N_channel
        uint32_t calculateN_FineResolution_CMN_1(uint32_t freq) {
            return (freq - 377000) * 65536 / 6500;
        }
        // Set frequency and calculate corresponding CCF0 and CNL
        void setFrequency_FineResolution_CMN_1(uint32_t freq) {
            frequency = freq;
            uint32_t N = calculateN_FineResolution_CMN_1(frequency);
            // Combine CCF0H and CCF0L into a single 16-bit value
            channelCenterFrequency09 = (N >> 8) & 0xFFFF; // Take bits 8-23
            channelNumber09 = N & 0xFF;                   // Extract the lowest byte (bits 0-7)
        }
        // Get frequency based on CCF0 and CNL
        uint32_t getFrequency_FineResolution_CMN_1() {
            uint32_t N_channel = ((uint32_t) channelCenterFrequency09 << 8) | channelNumber09; // Reconstruct full N_channel
            return 377000 + (6500 * N_channel) / 65536;
        }
    };

    struct AuxilarySettings {
        // RFn_AUXS
        ExternalLNABypass externalLNABypass09, externalLNABypass24;
        AutomaticGainControlMAP automaticGainControlMAP09, automaticGainControlMAP24;
        AnalogVoltageEnable analogVoltageEnable09, analogVoltageEnable24;
        AutomaticVoltageExternal automaticVoltageExternal09, automaticVoltageExternal24;
        PowerAmplifierVoltageControl powerAmplifierVoltageControl09, powerAmplifierVoltageControl24;
        // RFn_PADFE
        //

        static AuxilarySettings DefaultAuxilarySettings() {
            return {
                .automaticGainControlMAP24 = AutomaticGainControlMAP::AGC_BACKOFF_12,
                .analogVoltageEnable09 = AnalogVoltageEnable::DISABLED,
                .analogVoltageEnable24 = AnalogVoltageEnable::DISABLED,
                .automaticVoltageExternal09 = AutomaticVoltageExternal::DISABLED,
                .automaticVoltageExternal24 = AutomaticVoltageExternal::DISABLED,
                .powerAmplifierVoltageControl09 = PowerAmplifierVoltageControl::PAVC_2V4,
                .powerAmplifierVoltageControl24 = PowerAmplifierVoltageControl::PAVC_2V4,
                //
            };
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
        // BBCn_IRQM
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
                .frameBufferLevelIndication24 = true,
                .agcRelease09 = true,
                .agcRelease24 = true,
                .agcHold09 = true,
                .agcHold24 = true,
                .transmitterFrameEnd09 = true,
                .transmitterFrameEnd24 = true,
                .receiverExtendedMatch09 = true,
                .receiverExtendedMatch24 = true,
                .receiverAddressMatch09 = true,
                .receiverAddressMatch24 = true,
                .receiverFrameEnd09 = true,
                .receiverFrameEnd24 = true,
                .receiverFrameStart09 = true,
                .receiverFrameStart24 = true};
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
                .transceiverError09 = true,
                .batteryLow09 = true,
                .energyDetectionCompletion09 = true,
                .transceiverReady09 = true,
                .wakeup09 = true,
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

    struct AT86RF215Configuration {

        // Battery Monitor
        BatteryMonitorVoltage batteryMonitorVoltage =
            BatteryMonitorVoltage::BMHR_270_180;
        BatteryMonitorHighRange batteryMonitorHighRange =
            BatteryMonitorHighRange::LOW_RANGE;

        // Crystal oscillator
        CrystalTrim crystalTrim = CrystalTrim::TRIM_00;
        bool fastStartUp = false;

        // RFn_CFG
        bool irqMaskMode = true;
        IRQPolarity irqPolarity = IRQPolarity::ACTIVE_HIGH;
        PadDriverStrength padDriverStrength = PadDriverStrength::RF_DRV4;
    };

} // namespace AT86RF215
