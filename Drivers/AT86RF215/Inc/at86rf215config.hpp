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

        static RXConfig getDefaultRXConfig() {
            return DefaultRXConfig();
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
        static TXConfig getDefaultTXConfig() {
            return DefaultTXConfig();
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

    struct AT86RF215Configuration {

        SFDSearchSpace sfdSearchSpace = SFDSearchSpace::NSFD1;
        RXSpuriousCompensation rxSpuriousCompensation =
            RXSpuriousCompensation::DISABLED;
        ReducePowerConsumption reducePowerConsumption =
            ReducePowerConsumption::DISABLED;
        EnableProprietaryModes enableProprietaryModes =
            EnableProprietaryModes::NOT_SUPPORTED;
        FCSType fcsType = FCSType::FCS32;
        ReceiveMode receiveMode = ReceiveMode::OQPSKONLY;
        OQPSKPulseShapingFilter oqpskPulseShapingFilter =
            OQPSKPulseShapingFilter::BB_RC08;
        OQPSKChipFrequency oqpskChipFrequency = OQPSKChipFrequency::BB_FCHIP100;
        RXOOverride rxoOverride = RXOOverride::ENABLED;
        RXOLEGOverride rxoLEGOverride = RXOLEGOverride::ENABLED;

        // Battery Monitor
        BatteryMonitorVoltage batteryMonitorVoltage =
            BatteryMonitorVoltage::BMHR_270_180;
        BatteryMonitorHighRange batteryMonitorHighRange =
            BatteryMonitorHighRange::LOW_RANGE;

        // Crystal oscillator
        CrystalTrim crystalTrim = CrystalTrim::TRIM_00;
        bool fastStartUp = false;

        // PLL
        uint16_t pllFrequency09 = 0x8D20;
        uint16_t pllFrequency24 = 0x0CF8;
        uint16_t pllChannelNumber09 = 0x0003;
        uint16_t pllChannelNumber24 = 0x0000;
        PLLChannelMode pllChannelMode09 = PLLChannelMode::IEECompliant;
        PLLChannelMode pllChannelMode24 = PLLChannelMode::IEECompliant;
        PLLBandwidth pllBandwidth09 = PLLBandwidth::BWDefault;
        PLLBandwidth pllBandwidth24 = PLLBandwidth::BWDefault;
        uint8_t channelSpacing09 = 0x30;
        uint8_t channelSpacing24 = 0x08;

        ExternalLNABypass externalLNABypass09 = ExternalLNABypass::FALSE;
        ExternalLNABypass externalLNABypass24 = ExternalLNABypass::FALSE;
        AutomaticGainControlMAP automaticGainControlMAP09 =
            AutomaticGainControlMAP::INTERNAL_AGC;
        AutomaticGainControlMAP automaticGainControlMAP24 =
            AutomaticGainControlMAP::INTERNAL_AGC;
        AnalogVoltageEnable analogVoltageEnable09 = AnalogVoltageEnable::DISABLED;
        AnalogVoltageEnable analogVoltageEnable24 = AnalogVoltageEnable::DISABLED;
        AutomaticVoltageExternal automaticVoltageExternal09 =
            AutomaticVoltageExternal::DISABLED;
        AutomaticVoltageExternal automaticVoltageExternal24 =
            AutomaticVoltageExternal::DISABLED;
        PowerAmplifierVoltageControl powerAmplifierVoltageControl09 =
            PowerAmplifierVoltageControl::PAVC_2V4;
        PowerAmplifierVoltageControl powerAmplifierVoltageControl24 =
            PowerAmplifierVoltageControl::PAVC_2V4;

        ReceiverSampleRate receiverSampleRate09 = ReceiverSampleRate::FS_4000;
        ReceiverSampleRate receiverSampleRate24 = ReceiverSampleRate::FS_4000;

        // IQ Interface
        ExternalLoopback externalLoopback = ExternalLoopback::DISABLED;
        IQOutputCurrent iqOutputCurrent = IQOutputCurrent::CURR_2_MA;
        IQmodeVoltage iqmodeVoltage = IQmodeVoltage::MODE_200_MV;
        IQmodeVoltageIEE iqmodeVoltageIEE = IQmodeVoltageIEE::CMV;
        EmbeddedControlTX embeddedControlTX = EmbeddedControlTX::DISABLED;
        ChipMode chipMode = ChipMode::RF_MODE_BBRF;
        SkewAlignment skewAlignment = SkewAlignment::SKEW3906NS;

        // Baseband Core

        bool continuousTransmit09 = false;
        bool continuousTransmit24 = false;
        bool frameCheckSequenceFilter09 = false;
        bool frameCheckSequenceFilter24 = false;
        bool transmitterAutoFrameCheckSequence09 = false;
        bool transmitterAutoFrameCheckSequence24 = false;
        FrameCheckSequenceType frameCheckSequenceType09 = FrameCheckSequenceType::FCS_32;
        FrameCheckSequenceType frameCheckSequenceType24 = FrameCheckSequenceType::FCS_32;
        bool baseBandEnable09 = true;
        bool baseBandEnable24 = true;
        PhysicalLayerType physicalLayerType09 = PhysicalLayerType::BB_MRFSK;
        PhysicalLayerType physicalLayerType24 = PhysicalLayerType::BB_OFF;

        // Enabled Interrupts

        // Baseband IRQ
        // All interrupts enabled
        bool frameBufferLevelIndication09 = true;
        bool frameBufferLevelIndication24 = true;
        bool agcRelease09 = true;
        bool agcRelease24 = true;
        bool agcHold09 = true;
        bool agcHold24 = true;
        bool transmitterFrameEnd09 = true;
        bool transmitterFrameEnd24 = true;
        bool receiverExtendedMatch09 = true;
        bool receiverExtendedMatch24 = true;
        bool receiverAddressMatch09 = true;
        bool receiverAddressMatch24 = true;
        bool receiverFrameEnd09 = true;
        bool receiverFrameEnd24 = true;
        bool receiverFrameStart09 = true;
        bool receiverFrameStart24 = true;

        // Radio IRQ
        bool iqIfSynchronizationFailure09 = true;
        bool iqIfSynchronizationFailure24 = true;
        bool trasnceiverError09 = true;
        bool trasnceiverError24 = true;
        bool batteryLow09 = true;
        bool batteryLow24 = true;
        bool energyDetectionCompletion09 = true;
        bool energyDetectionCompletion24 = true;
        bool transceiverReady09 = true;
        bool transceiverReady24 = true;
        bool wakeup09 = true;
        bool wakeup24 = true;

        // IRQ Config
        bool irqMaskMode = true;
        IRQPolarity irqPolarity = IRQPolarity::ACTIVE_HIGH;
        PadDriverStrength padDriverStrength = PadDriverStrength::RF_DRV4;
    };

} // namespace AT86RF215
