#pragma once

namespace AT86RF215 {

    #define FBLI (1 << 0)
    #define RXFE_RX (1 << 1)
    #define RXFE_TX (1 << 2)
    #define RXFS (1 << 3)
    #define AGC_HOLD (1 << 4)
    #define AGC_RELEASE (1 << 5)
    #define TXFE (1 << 6)
    /// RADIO
    #define IFSERR (1 << 7)
    #define BATL (1 << 8)
    #define TRXRDY (1 << 9)
    #define TRXERR (1 << 10)
    #define WAKEUP (1 << 11)
    #define START_TX_TASK (1 << 12)
    #define TRANSMIT (1 << 13)
    #define RXFE_STATE (1 << 14)
    #define TM_OBC (1 << 15)
    #define TM_COMMS (1 << 16)
    #define TXFE_RX (1 << 17)
    #define TC_UART (1 << 18)
    #define TC_RF_RX (1 << 19)
    #define TC_UART_TC_HANDLING_TASK (1 << 20)
    #define TM_OBC_TM_HANDLING (1 << 15)
    /// Indexes
    #define NOTIFY_INDEX_TRANSMIT 1
    #define NOTIFY_INDEX_RXFE_RX 2
    #define NOTIFY_INDEX_RXFE_RX_STATE 3
    #define NOTIFY_INDEX_AGC_RELEASE 4
    #define NOTIFY_INDEX_RXFE_TX 5
    #define NOTIFY_INDEX_TXFE_TX 6
    #define NOTIFY_INDEX_TXFE_RX 7
    #define NOTIFY_INDEX_AGC 8
    #define NOTIFY_INDEX_RXFS 9
    #define NOTIFY_INDEX_INCOMING_TC 10
    #define NOTIFY_INDEX_RECEIVED_TM 11

    enum class EnergyDetectionTimeBasis {
        RF_2MS = 0x0,
        RF_8MS = 0x1,
        RF_32MS = 0x2,
        RF_128MS = 0x3,
    };

    enum State {
        RF_NOP = 0x0,
        RF_SLEEP = 0x1,
        RF_TRXOFF = 0x2,
        RF_TXPREP = 0x3,
        RF_TX = 0x4,
        RF_RX = 0x5,
        RF_TRANSITION = 0x6,
        RF_RESET = 0x7,
        RF_INVALID = 0x8,
    };
    // BBCn_FSKC0
    enum class Bandwidth_time_product {
        BT_0_5 = 0x0,
        BT_1_0 = 0x1,
        BT_1_5 = 0x2,
        BT_2_0 = 0x3,
    };
    enum class Mod_index_scale {
        s_0_850 = 0x0,
        s_1_0 = 0x1,
        s_1_125 = 0x2,
        s_1_250 = 0x3,
    };
    enum class Mod_index {
        bf_0_375 = 0x0,
        bf_0_500 = 0x1,
        bf_0_750 = 0x2,
        bf_1_000 = 0x3,
        bf_1_250 = 0x4,
        bf_1_500 = 0x5,
        bf_1_750 = 0x6,
        bf_2_000 = 0x7,
    };

    enum class FSK_mod_order {
        binary_fsk = 0x0,
        four_fsk = 0x1,
    };
    /// BBCn_FSKC1

    enum class Freq_Inversion {
        freq_inversion_off = 0x0,
        freq_inversion_on = 0x1
    };
    enum class MR_FSK_symbol_rate {
        sr_50 = 0x0,
        sr_100 = 0x1,
        sr_150 = 0x2,
        sr_200 = 0x3,
        sr_300 = 0x4,
        sr_400 = 0x5,
    };
    /// BBC_FSKC2
    enum class Preamble_Detection {
        preamble_det_without_rssi = 0x0,
        preamble_det_with_rssi = 0x1
    };

    enum class Receiver_Override {
        restart_by_6db_stronger_frame = 0x0,  // Receiver restarted by >6dB stronger frame
        restart_by_12db_stronger_frame = 0x1, // Receiver restarted by >12dB stronger frame
        restart_by_18db_stronger_frame = 0x2, // Receiver restarted by >18dB stronger frame
        override_disabled = 0x3               // Receiver override disabled
    };

    enum class Receiver_Preamble_Timeout {
        timeout_disabled = 0x0, // Receiver preamble timeout disabled
        timeout_enabled = 0x1   // Receiver preamble timeout enabled
    };


    enum class Mode_Switch_Enable {
        disabled = 0x0, // Mode Switch disabled
        enabled = 0x1   // Mode Switch enabled
    };


    enum class Preamble_Inversion {
        no_inversion = 0x0, // No inversion of FSK preamble frequency deviation
        inversion = 0x1     // Inversion of FSK preamble frequency deviation
    };


    enum class FEC_Scheme {
        NRNSC = 0x0, // Non-recursive and non-systematic convolutional code (NRNSC)
        RSC = 0x1    // Recursive and systematic convolutional code (RSC)
    };


    enum class Interleaving_Enable {
        disabled = 0x0, // Interleaving disabled
        enabled = 0x1   // Interleaving enabled
    };

    /// BBC_FSKC3
    enum class SFD_Detection_Threshold {
        default_sfd_IEEE = 0x8,
        sfd_weak = 15,
    };
    enum class Preamble_Detection_Threshold {
        default_value = 0x5,
        increased_preamble_sensitivity = 0x0
    };
    /// BBC_FSKC4
    enum class SFD_Quantization {
        SOFT_DECISION = 0x0, // Soft decision at bit positions
        HARD_DECISION = 0x1  // Hard decision at bit positions
    };
    enum class SFD_32 {
        TWO_16BIT_SFD = 0x0,   // Search for two 16-bit SFDs
        SINGLE_32BIT_SFD = 0x1 // Search for a single 32-bit SFD
    };
    enum class Raw_Mode_Reversal_Bit {
        LSB_FIRST = 0x0, // Least Significant Bit first
        MSB_FIRST = 0x1  // Most Significant Bit first
    };
    enum class CSFD1 {
        UNCODED_IEEE_MODE = 0x0, // Uncoded IEEE mode
        UNCODED_RAW_MODE = 0x1,  // Uncoded RAW mode
        CODED_IEEE_MODE = 0x2,   // Coded IEEE mode
        CODED_RAW_MODE = 0x3     // Coded RAW mode
    };
    enum class CSFD0 {
        UNCODED_IEEE_MODE = 0x0, // Uncoded IEEE mode
        UNCODED_RAW_MODE = 0x1,  // Uncoded RAW mode
        CODED_IEEE_MODE = 0x2,   // Coded IEEE mode
        CODED_RAW_MODE = 0x3     // Coded RAW mode
    };
    /// BBCn_FSKPHRTX
    enum class SFD_Used {
        sfd0_used = 0x0,
        sfd1_used = 0x1,
    };
    enum class Data_Whitening {
        psdu_data_whitening_disabled = 0x0,
        psdu_data_whitening_enabled = 0x1,
    };
    /// BBCn_FSKDM
    enum class FSK_Preamphasis_Enable {
        preamphasis_disabled = 0x0,
        preamphasis_enabled = 0x1,
    };
    enum class Direct_Mod_Enable_FSKDM {
        direct_mod_disabled = 0x0,
        direct_mod_enabled = 0x1,
    };

    enum class InterruptMask {
        FrameBufferLevelIndication = 0x80,
        AGCRelease = 0x40,
        AGCHold = 0x20,
        TransmitterFrameEnd = 0x10,
        ReceiverExtendMatch = 0x08,
        ReceiverAddressMatch = 0x04,
        ReceiverFrameEnd = 0x02,
        ReceiverFrameStart = 0x01,
        IFSynchronization = 0x20,
        TransceiverError = 0x10,
        BatteryLow = 0x08,
        EnergyDetectionCompletion = 0x04,
        TransceiverReady = 0x02,
        Wakeup = 0x01,
    };

    enum class DevicePartNumber {
        AT86RF215_INVALID = 0x00,
        AT86RF215 = 0x34,
        AT86RF215IQ = 0x35,
        AT86RF215M = 0x36,
    };

    enum RegisterBitmasks {
        RF09_IRQS_IQIFSF = 0x20, ///< Set to 1 if I/Q sync fails
        RF09_IRQS_TRXERR = 0x10, ///< Set to 1 if transceiver error is detected
        RF09_IRQS_BATLOW = 0x08, ///< Set to 1 if EVDD voltage is below threshold
        RF09_IRQS_EDC = 0x04,    ///< Set to 1 if a single or continuous measurement is completed
        RF09_IRQS_TRXRDY = 0x02, ///< Set to 1 if TXREP is written to RFn_CMD and transceiver reached state TXPREP
        RF09_IRQS_WAKEUP = 0x01, ///< Set to 1 if the wake-up or power-up procedure is competed
    };

    enum class PLLChannelMode {
        IEECompliant = 0x0,       ///< f = (CCF0 + CN*CS)*25kHz + 1.5GHz*(transceiver == RF24)
        FineResolution450 = 0x1,  ///< 389.5-510 MHz -- 99.182Hz stepping
        FineResolution900 = 0x2,  ///< 779-1020 MHz -- 193.364Hz stepping
        FineResolution2443 = 0x3, ///< 2400-2483.5 MHz -- 396.782Hz stepping
    };

    enum class PLLBandwidth {
        BWDefault = 0x0, ///< Default loopbandwidth
        BWSmaller = 0x1, ///< 15% smaller PLL loopbandwidth
        BWLarger = 0x2,  ///< 15% larger PLL loopbandwidth
        BWInvalid = 0x3,
    };

    enum class PLLState {
        PLLNotLocked = 0x0,
        PLLLocked = 0x1
    };

    enum class ExternalLoopback {
        DISABLED = 0x0,
        ENABLED = 0x1
    };

    enum class SynchronizationFailure {
        NO_SYNCH_FAILURE,
        SYNCH_FAILURE
    };

    enum class IQOutputCurrent {
        CURR_1_MA = 0x0, ///< 1mA
        CURR_2_MA = 0x1, ///< 2mA
        CURR_3_MA = 0x2, ///< 3mA
        CURR_4_MA = 0x3  ///< 4mA
    };

    enum class IQmodeVoltage {
        MODE_150_MV = 0x0, ///< 150mV
        MODE_200_MV = 0x1, ///< 200mV
        MODE_250_MV = 0x2, ///< 250mV
        MODE_300_MV = 0x3  ///< 300mV
    };

    enum class IQmodeVoltageIEE {
        CMV = 0x0,  ///< Is set by the CMV Register of RF_IQIFC0
        IEEE = 0x1, ///< Is IEEE-compliant (1V2)
    };

    enum class ChipMode {
        RF_MODE_BBRF = 0x0,   ///< BBC0, BBC1, I/Q IF enabled
        RF_MODE_RF = 0x1,     ///< BBC0, BBC1 disabled, I/Q IF enabled
        RF_MODE_BBRF09 = 0x4, ///< BBC0 disabled , BBC1 enabled, I/Q IF enabled (sub 1GHz)
        RF_MODE_BBRF24 = 0x5, ///< BBC0 enabled , BBC1 disabled, I/Q IF enabled (2.4GHz)
    };

    enum class EmbeddedControlTX {
        DISABLED = 0x0,
        ENABLED = 0x1
    };

    enum class SkewAlignment {
        SKEW1906NS = 0x0, ///< 1.906ns
        SKEW2906NS = 0x1, ///< 2.906ns
        SKEW3906NS = 0x2, ///< 3.906ns
        SKEW4906NS = 0x3, ///< 4.906ns
    };

    enum class CrystalTrim {
        TRIM_00 = 0x0, ///< +0.0pF
        TRIM_03 = 0x1, ///< +0.3pF
        TRIM_02 = 0x2, ///< +0.6pF
        TRIM_09 = 0x3, ///< +0.9pF
        TRIM_12 = 0x4, ///< +1.2pF
        TRIM_15 = 0x5, ///< +1.5pF
        TRIM_18 = 0x6, ///< +1.8pF
        TRIM_21 = 0x7, ///< +2.1pF
        TRIM_24 = 0x8, ///< +2.4pF
        TRIM_27 = 0x7, ///< +2.7pF
        TRIM_30 = 0xa, ///< +3.0pF
        TRIM_33 = 0xb, ///< +3.3pF
        TRIM_36 = 0xc, ///< +3.6pF
        TRIM_39 = 0xd, ///< +3.9pF
        TRIM_42 = 0xe, ///< +4.2pF
        TRIM_45 = 0xf, ///< +4.5pF
        TRIM_INV = 0xff,
    };
    /*
*
* Distinguish between sub-1GHz transceiver and 2.4 GHz transceiver
*/
    enum Transceiver {
        RF09 = 0,
        RF24 = 1,
    };

    enum class PowerAmplifierRampTime {
        RF_PARAMP4U = 0x0,  ///< 4μs
        RF_PARAMP8U = 0x1,  ///< 8μs
        RF_PARAMP16U = 0x2, ///< 16μs
        RF_PARAMP32U = 0x3, ///< 32μs
    };

    enum class TransmitterCutOffFrequency {
        RF_FLC80KHZ = 0x0,   ///< fLPFCUT = 80kHz
        RF_FLC100KHZ = 0x1,  ///< fLPFCUT = 100kHz
        RF_FLC125KHZ = 0x2,  ///< fLPFCUT = 125kHz
        RF_FLC160KHZ = 0x3,  ///< fLPFCUT = 160kHz
        RF_FLC200KHZ = 0x4,  ///< fLPFCUT = 200kHz
        RF_FLC250KHZ = 0x5,  ///< fLPFCUT = 250kHz
        RF_FLC315KHZ = 0x6,  ///< fLPFCUT = 315kHz
        RF_FLC400KHZ = 0x7,  ///< fLPFCUT = 400kHz
        RF_FLC500KHZ = 0x8,  ///< fLPFCUT = 500kHz
        RF_FLC625KHZ = 0x9,  ///< fLPFCUT = 625kHz
        RF_FLC800KHZ = 0xA,  ///< fLPFCUT = 800kHz
        RF_FLC1000KHZ = 0xB, ///< fLPFCUT = 1000kHz
    };

    enum class PowerAmplifierCurrentControl {
        PA_22 = 0x0, ///< 3 dB reduction of max small signal gain
        PA_18 = 0x1, ///< 2 dB reduction of max small signal gain
        PA_11 = 0x2, ///< 1 dB reduction of max small signal gain
        PA_NO = 0x3, ///< max. transmit small signal gain
    };

    enum class EnergyDetectionMode {
        RF_EDAUTO = 0x0, ///< Energy detection measurement is automatically	triggered
        ///< if the AGC is held by the internal baseband or by setting bit FRZC.
        RF_EDSINGLE = 0x1, ///< A single energy detection measurement is started.
        RF_EDCONT = 0x2,   ///< A continuous energy detection measurements of configured interval
        ///< defined in register EDD is started.
        RF_EDOFF = 0x3, ///< Energy detection measurement is disabled.
    };

    enum class ReceiverBandwidth {
        RF_BW160KHZ_IF250KHZ = 0x0,   ///< fBW=160kHz 	fIF=250kHz
        RF_BW200KHZ_IF250KHZ = 0x1,   ///< fBW=200kHz 	fIF=250kHz
        RF_BW250KHZ_IF250KHZ = 0x2,   ///< fBW=250kHz 	fIF=250kHz
        RF_BW320KHZ_IF500KHZ = 0x3,   ///< fBW=320kHz 	fIF=500kHz
        RF_BW400KHZ_IF500KHZ = 0x4,   ///< fBW=400kHz 	fIF=500kHz
        RF_BW500KHZ_IF500KHZ = 0x5,   ///< fBW=500kHz 	fIF=500kHz
        RF_BW630KHZ_IF1000KHZ = 0x6,  ///< fBW=630kHz 	fIF=1000kHz
        RF_BW800KHZ_IF1000KHZ = 0x7,  ///< fBW=800kHz 	fIF=1000kHz
        RF_BW1000KHZ_IF1000KHZ = 0x8, ///< fBW=1000kHz 	fIF=1000kHz
        RF_BW1250KHZ_IF2000KHZ = 0x9, ///< fBW=1250kHz	fIF=2000kHz
        RF_BW1600KHZ_IF2000KHZ = 0xA, ///< fBW=1600kHz 	fIF=2000kHz
        RF_BW2000KHZ_IF2000KHZ = 0xB, ///< fBW=2000kHz   fIF=2000kHz
    };

    enum class TxRelativeCutoffFrequency {
        FCUT_025 = 0x00,  ///< 0.25*fs/2
        FCUT_0375 = 0x01, ///< 0.375*fs/2
        FCUT_05 = 0x02,   ///< 0.5*fs/2
        FCUT_075 = 0x03,  ///< 0.75*fs/2
        FCUT_1 = 0x04,    ///< fs/2
    };

    enum class RxRelativeCutoffFrequency {
        FCUT_025 = 0x00,  ///< 0.25*fs/2
        FCUT_0375 = 0x01, ///< 0.375*fs/2
        FCUT_05 = 0x02,   ///< 0.5*fs/2
        FCUT_075 = 0x03,  ///< 0.75*fs/2
        FCUT_1 = 0x04,    ///< fs/2
    };

    enum class ExternalLNABypass {
        FALSE = 0x0, ///< Bypass of external LNA not available
        TRUE = 0x1,  ///< Bypass of external LNA available
    };

    enum class TransmitterSampleRate {
        FS_4000 = 0x1,   ///< fs = 4000 kHz
        FS_2000 = 0x2,   ///< fs = 2000 kHz
        FS_4000_3 = 0x3, ///< fs = 4000/3 kHz
        FS_1000 = 0x4,   ///< fs = 1000 kHz
        FS_800 = 0x5,    ///< fs = 800 kHzA
        FS_2000_3 = 0x6, ///< fs = 2000/3 kHz
        FS_500 = 0x8,    ///< fs = 500 kHz
        FS_400 = 0xA,    ///< fs = 400 kHz
    };

    enum class ReceiverSampleRate {
        FS_4000 = 0x1,   ///< fs = 4000 kHz
        FS_2000 = 0x2,   ///< fs = 2000 kHz
        FS_4000_3 = 0x3, ///< fs = 4000/3 kHz
        FS_1000 = 0x4,   ///< fs = 1000 kHz
        FS_800 = 0x5,    ///< fs = 800 kHzA
        FS_2000_3 = 0x6, ///< fs = 2000/3 kHz
        FS_500 = 0x8,    ///< fs = 500 kHz
        FS_400 = 0xA,    ///< fs = 400 kHz
    };

    enum class AutomaticGainControlMAP {
        INTERNAL_AGC = 0x0,   ///< Internal AGC, no external LNA
        AGC_BACKOFF_9 = 0x1,  ///< AGC back-off for external LNA (~9 dB gain)
        AGC_BACKOFF_12 = 0x2, ///< AGC back-off for external LNA (~12 dB gain)
        AGC_INVALID = 0x3,
    };

    enum class AutomaticGainTarget {
        DB21 = 0x0, //-21dB
        DB24 = 0x1, //-24dB
        DB27 = 0x2, //-27dB
        DB30 = 0x3, //-30dB
        DB33 = 0x4, //-33dB
        DB36 = 0x5, //-36dB
        DB39 = 0x6, //-39dB
        DB42 = 0x7, //-42dB
    };

    enum class AutomaticVoltageExternal {
        DISABLED = 0x0, ///< Disabled internal supply voltage
        ENABLED = 0x1,
        INVALID = 0x2,
    };

    enum class AverageTimeNumberSamples {
        AVGS_8 = 0x0,
        AVGS_16 = 0x1,
        AVGS_32 = 0x2,
        AVGS_64 = 0x3,
    };

    enum class AGCReset {
        default_agc_reset = 0x0,
        reset_agc_and_set_max_gain = 0x1
    };

    enum class AGCFreezeControl {
        no_freeze = 0x0,
        freeze_to_current_value = 0x1
    };

    enum class AGCEnable {
        agc_disabled = 0x0,
        agc_enabled = 0x1
    };

    enum class AnalogVoltageEnable {
        DISABLED = 0x0,
        // If this bit is set to 1, the analog voltage regulator turns on in state TRXOFF. This setting enables faster transition from state TRXOFF to TXPREP or RX.
        ENABLED = 0x1,
    };

    enum class PowerAmplifierVoltageControl {
        PAVC_2V0 = 0x0, ///< 2.0 V
        PAVC_2V2 = 0x1, ///< 2.2 V
        PAVC_2V4 = 0x2, ///< 2.4 V
        PAVC_INVALID = 0x3,
    };

    /// RFn_PADFE
    enum class ExternalFrontEndControl {
        /// Configuration 0: no Frontend control; FEAnn and FEBnn output is always 0
        no_front_end_control = 0x0,
        /// Configuration 1: (1 pin is TX switch; 1 pin is RX switch; LNA can be bypassed)
        front_end_config_one = 0x1,
        /// Configuration 2: (1 pin is enable, 1 pin is TXRX switch; 1 | 0 additional option)
        front_end_config_txrx_switch = 0x2,
        /// Configuration 3: (1 pin is TXRX switch, 1 pin is LNA Bypass, 1 pin (MCU) is enable)
        front_end_config_three = 0x3
    };

    enum class BatteryMonitorStatus {
        LOW_ENABLED = 0x0,
        HIGH_ENABLED = 0x1,
    };

    enum class HighRateLegacyOQPSK {
        HDRL_DISABLED = 0x0, //Legacy O-QPSK according to IEEE Std 802.15.4-2006
        HDRL_ENABLED = 0x1,  //Proprietary legacy O-QPSK high data rate mode
    };

    enum class SFDSearchSpace {
        NSFD1 = 0x0,     //Search for SFD_1 only
        NSFD1_2 = 0x1,   //Search for SFD_1 and SFD_2
        NSFD1_3 = 0x2,   //Search for SFD_1 and SFD_3
        NSFD1_2_3 = 0x3, //Search for SFD_1 and SFD_2 and SFD_3
    };

    enum class BatteryMonitorVoltageThreshold {
        BMHR_255_170 = 0x0, //BMHR=1: 2.550V  | BMHR=0: 1.70V
        BMHR_262_175 = 0x1, ///< BMHR=1: 2.625V | BMHR=0: 1.75V
        BMHR_270_180 = 0x2, ///< BMHR=1: 2.700V | BMHR=0: 1.80V
        BMHR_277_185 = 0x3, ///< BMHR=1: 2.775V | BMHR=0: 1.85V
        BMHR_285_190 = 0x4, ///< BMHR=1: 2.850V | BMHR=0: 1.90V
        BMHR_292_195 = 0x5, ///< BMHR=1: 2.925V | BMHR=0: 1.95V
        BMHR_300_200 = 0x6, ///< BMHR=1: 3.000V | BMHR=0: 2.00V
        BMHR_307_205 = 0x7, ///< BMHR=1: 3.075V | BMHR=0: 2.05V
        BMHR_315_210 = 0x8, ///< BMHR=1: 3.150V | BMHR=0: 2.10V
        BMHR_322_215 = 0x9, ///< BMHR=1: 3.225V | BMHR=0: 2.15V
        BMHR_330_220 = 0xA, ///< BMHR=1: 3.300V | BMHR=0: 2.20V
        BMHR_337_225 = 0xB, ///< BMHR=1: 3.375V | BMHR=0: 2.25V
        BMHR_345_230 = 0xC, ///< BMHR=1: 3.450V | BMHR=0: 2.30V
        BMHR_352_235 = 0xD, ///< BMHR=1: 3.525V | BMHR=0: 2.35V
        BMHR_360_240 = 0xE, ///< BMHR=1: 3.600V | BMHR=0: 2.40V
        BMHR_367_245 = 0xF, ///< BMHR=1: 3.675V | BMHR=0: 2.45V
    };

    enum class BatteryMonitorHighRange {
        LOW_RANGE = 0x0,
        HIGH_RANGE = 0x1,
    };

    enum class RXSpuriousCompensation {
        DISABLED = 0x0, ///< RXSpuriousCompensation is disabled
        ENABLED = 0x1,  ///< RXSpuriousCompensation is enabled
    };

    enum class ReducePowerConsumption {
        DISABLED = 0x0, ///< Power saving is disabled
        ENABLED = 0x1,  ///< Power saving is enabled
    };

    enum class EnableProprietaryModes {
        NOT_SUPPORTED = 0x0, //Reception of proprietary rate modes is not supported
        SUPPORTED = 0x1,     //Reception of proprietary rate modes is supported
    };

    enum class FCSType {
        FCS32 = 0x0, //FCS type for legacy O-QPSK is 32-bit (this mode is a proprietary setting)
        FCS16 = 0x1, //FCS type legacy O-QPSK is 16-bit
    };

    enum class ReceiveMode {
        OQPSKONLY = 0x0,       ///< Listen for frames of MR-O-QPSK PHY only
        LEGACYOQPSKONLY = 0x1, //Listen for frames of legacy O-QPSK PHY only (if applicable)
        BOTH = 0x2,            //Listen for both, frames of MR-O-QPSK and legacy O-QPSK PHY (if applicable)
        DISABLEBOTH = 0x3,     //Disable detection for both PHYs
    };

    enum class OQPSKPulseShapingFilter {
        BB_RC08 = 0x0,  ///< RC-0.8 shaping (raised cosine, roll-off = 0.8)
        BB_RRC08 = 0x1, ///< RRC-0.8 shaping (root raised cosine, roll-off = 0.8)
    };

    enum class OQPSKChipFrequency {
        BB_FCHIP100 = 0x0,  ///< 100kchip/s
        BB_FCHIP200 = 0x1,  ///< 200kchip/s
        BB_FCHIP1000 = 0x2, ///< 1000kchip/s
        BB_FCHIP2000 = 0x3, ///< 2000kchip/s
        INVALID = 0x4
    };

    enum class RXOOverride {
        DISABLED = 0x0,
        ENABLED = 0x1,
    };

    enum class RXOLEGOverride {
        DISABLED = 0x0,
        ENABLED = 0x1,
    };

    enum class IRQPolarity {
        ACTIVE_HIGH = 0x0,
        ACTIVE_LOW = 0x1
    };

    enum class PadDriverStrength {
        RF_DRV2 = 0x0, // 2mA
        RF_DRV4 = 0x1, // 4mA
        RF_DRV6 = 0x2, // 6mA
        RF_DRV8 = 0x3, // 8mA
    };

    /// Specifies the Frame Check Sequence for CRC validation of incoming frames
    enum class FrameCheckSequenceType {
        /// 32-bit
        FCS_32 = 0x0,
        /// 16-bit
        FCS_16 = 0x1,
    };

    /// Specifies the modulation class of the physical layer
    enum class PhysicalLayerType {
        /// Supports Baseband is off
        BB_OFF = 0x0,
        /// MR-FSK specified in IEEE Std 802.15.4
        BB_MRFSK = 0x1,
        /// Supports MR-OFDM specified in IEEE Std 802.15.4g-2012 and ETSI TS 102 887-1
        BB_MROFDM = 0x2,
        /// Supports MR-O-QPSK PHY specified IEEE Std 802.15.4g-2012 and legacy O-QPSK PHY of ETSI TS 102 887-1
        BB_MROQPSK = 0x3,
    };

    enum RegisterAddress {
        RF09_IRQS = 0x00, ///< Contains radio I/Q status
        RF24_IRQS = 0x01, ///< Contains radio I/Q status
        BBC0_IRQS = 0x02,
        BBC1_IRQS = 0x03,
        RF_RST = 0x05,
        RF_CFG = 0x06,
        RF_CLKO = 0x07,
        RF_BMDVC = 0x08,
        RF_XOC = 0x09,
        RF_IQIFC0 = 0x0A,
        RF_IQIFC1 = 0x0B,
        RF_IQIFC2 = 0x0C,
        RF_PN = 0x0D, ///< Holds the Device Part Number
        RF_VN = 0x0E, ///< Holds the Version Number of the device
        RF09_IRQM = 0x100,
        RF09_AUXS = 0x101,  ///< Transceiver auxiliary settings
        RF09_STATE = 0x102, ///< Holds the state of the transceiver
        RF09_CMD = 0x103,   ///< Sets the state of the transceiver
        RF09_CS = 0x104,    ///< Sets the channel spacing of the PLL
        RF09_CCF0L = 0x105, ///< Low byte of the PLL's central frequency
        RF09_CCF0H = 0x106, ///< High byte of the PLL's central frequency
        RF09_CNL = 0x107,
        RF09_CNM = 0x108,
        RF09_RXBWC = 0x109,
        RF09_RXDFE = 0x10A,
        RF09_AGCC = 0x10B,
        RF09_AGCS = 0x10C,
        RF09_RSSI = 0x10D,
        RF09_EDC = 0x10E,
        RF09_EDD = 0x10F,
        RF09_EDV = 0x110,
        RF09_RNDV = 0x111,
        RF09_TXCUTC = 0x112, ///< TX Filter cutoff and PA ramp-up time
        RF09_TXDFE = 0x113,  ///< TX digital frontend
        RF09_PAC = 0x114,    ///< TX PA Control
        RF09_PADFE = 0x116,
        RF09_PLL = 0x121,   ///< Determines whether PLL is locked or not
        RF09_PLLCF = 0x122, ///< Holds PLL center frequency value
        RF09_TXCI = 0x125,
        RF09_TXCQ = 0x126,
        RF09_TXDACI = 0x127,
        RF09_TXDACQ = 0x128,
        RF24_IRQM = 0x200,
        BBC0_FBRXS = 0x2000,
        RF24_AUXS = 0x201,  ///< Transceiver auxiliary settings
        RF24_STATE = 0x202, ///< Holds the state of the transceiver
        RF24_CMD = 0x203,   ///< Sets the state of the transceiver
        RF24_CS = 0x204,    ///< Sets the channel spacing of the PLL
        RF24_CCF0L = 0x205, ///< Low byte of the PLL's central frequency
        RF24_CCF0H = 0x206, ///< High byte of the PLL's central frequency
        RF24_CNL = 0x207,
        RF24_CNM = 0x208,
        RF24_RXBWC = 0x209,
        RF24_RXDFE = 0x20A,
        RF24_AGCC = 0x20B,
        RF24_AGCS = 0x20C,
        RF24_RSSI = 0x20D,
        RF24_EDC = 0x20E,
        RF24_EDD = 0x20F,
        RF24_EDV = 0x210,
        RF24_RNDV = 0x211,
        RF24_TXCUTC = 0x212, ///< TX Filter cutoff and PA ramp-up time
        RF24_TXDFE = 0x213,  ///< TX Digital Frontend
        RF24_PAC = 0x214,    ///< TX PA Control
        RF24_PADFE = 0x216,
        RF24_PLL = 0x221,   ///< Determines the lockstate and loopbandwidth of PLL
        RF24_PLLCF = 0x222, ///< Holds PLL center frequency value
        RF24_TXCI = 0x225,
        RF24_TXCQ = 0x226,
        RF24_TXDACI = 0x227,
        RF24_TXDACQ = 0x228,
        BBC0_FBRXE = 0x27FE,
        BBC0_FBTXS = 0x2800,
        BBC0_FBTXE = 0x2FFE,
        BBC0_IRQM = 0x300,
        BBC1_FBRXS = 0x3000,
        BBC0_PC = 0x301,
        BBC0_PS = 0x302,
        BBC0_RXFLL = 0x304,
        BBC0_RXFLH = 0x305,
        BBC0_TXFLL = 0x306,
        BBC0_TXFLH = 0x307,
        BBC0_FBLL = 0x308,
        BBC0_FBLH = 0x309,
        BBC0_FBLIL = 0x30A,
        BBC0_FBLIH = 0x30B,
        BBC0_OFDMPHRTX = 0x30C,
        BBC0_OFDMPHRRX = 0x30D,
        BBC0_OFDMC = 0x30E,
        BBC0_OFDMSW = 0x30F,
        BBC0_OQPSKC0 = 0x310,
        BBC0_OQPSKC1 = 0x311,
        BBC0_OQPSKC2 = 0x312,
        BBC0_OQPSKC3 = 0x313,
        BBC0_OQPSKPHRTX = 0x314,
        BBC0_OQPSKPHRRX = 0x315,
        BBC0_AFC0 = 0x320,
        BBC0_AFC1 = 0x321,
        BBC0_AFFTM = 0x322,
        BBC0_AFFVM = 0x323,
        BBC0_AFS = 0x324,
        BBC0_MACEA0 = 0x325,
        BBC0_MACEA1 = 0x326,
        BBC0_MACEA2 = 0x327,
        BBC0_MACEA3 = 0x328,
        BBC0_MACEA4 = 0x329,
        BBC0_MACEA5 = 0x32A,
        BBC0_MACEA6 = 0x32B,
        BBC0_MACEA7 = 0x32C,
        BBC0_MACPID0F0 = 0x32D,
        BBC0_MACPID1F0 = 0x32E,
        BBC0_MACSHA0F0 = 0x32F,
        BBC0_MACSHA1F0 = 0x330,
        BBC0_MACPID0F1 = 0x331,
        BBC0_MACPID1F1 = 0x332,
        BBC0_MACSHA0F1 = 0x333,
        BBC0_MACSHA1F1 = 0x334,
        BBC0_MACPID0F2 = 0x335,
        BBC0_MACPID1F2 = 0x336,
        BBC0_MACSHA0F2 = 0x337,
        BBC0_MACSHA1F2 = 0x338,
        BBC0_MACPID0F3 = 0x339,
        BBC0_MACPID1F3 = 0x33A,
        BBC0_MACSHA0F3 = 0x33B,
        BBC0_MACSHA1F3 = 0x33C,
        BBC0_AMCS = 0x340,
        BBC0_AMEDT = 0x341,
        BBC0_AMAACKPD = 0x342,
        BBC0_AMAACKTL = 0x343,
        BBC0_AMAACKTH = 0x344,
        BBC0_FSKC0 = 0x360,
        BBC0_FSKC1 = 0x361,
        BBC0_FSKC2 = 0x362,
        BBC0_FSKC3 = 0x363,
        BBC0_FSKC4 = 0x364,
        BBC0_FSKPLL = 0x365,
        BBC0_FSKSFD0L = 0x366,
        BBC0_FSKSFD0H = 0x367,
        BBC0_FSKSFD1L = 0x368,
        BBC0_FSKSFD1H = 0x369,
        BBC0_FSKPHRTX = 0x36A,
        BBC0_FSKPHRRX = 0x36B,
        BBC0_FSKRPC = 0x36C,
        BBC0_FSKRPCONT = 0x36D,
        BBC0_FSKRPCOFFT = 0x36E,
        BBC0_FSKRRXFLL = 0x370,
        BBC0_FSKRRXFLH = 0x371,
        BBC0_FSKDM = 0x372,
        BBC0_FSKPE0 = 0x373,
        BBC0_FSKPE1 = 0x374,
        BBC0_FSKPE2 = 0x375,
        BBC1_FBRXE = 0x37FE,
        BBC0_PMUC = 0x380,
        BBC1_FBTXS = 0x3800,
        BBC0_PMUVAL = 0x381,
        BBC0_PMUQF = 0x382,
        BBC0_PMUI = 0x383,
        BBC0_PMUQ = 0x384,
        BBC0_CNTC = 0x390,
        BBC0_CNT0 = 0x391,
        BBC0_CNT1 = 0x392,
        BBC0_CNT2 = 0x393,
        BBC0_CNT3 = 0x394,
        BBC1_FBTXE = 0x3FFE,
        BBC1_IRQM = 0x400,
        BBC1_PC = 0x401,
        BBC1_PS = 0x402,
        BBC1_RXFLL = 0x404,
        BBC1_RXFLH = 0x405,
        BBC1_TXFLL = 0x406,
        BBC1_TXFLH = 0x407,
        BBC1_FBLL = 0x408,
        BBC1_FBLH = 0x409,
        BBC1_FBLIL = 0x40A,
        BBC1_FBLIH = 0x40B,
        BBC1_OFDMPHRTX = 0x40C,
        BBC1_OFDMPHRRX = 0x40D,
        BBC1_OFDMC = 0x40E,
        BBC1_OFDMSW = 0x40F,
        BBC1_OQPSKC0 = 0x410,
        BBC1_OQPSKC1 = 0x411,
        BBC1_OQPSKC2 = 0x412,
        BBC1_OQPSKC3 = 0x413,
        BBC1_OQPSKPHRTX = 0x414,
        BBC1_OQPSKPHRRX = 0x415,
        BBC1_AFC0 = 0x420,
        BBC1_AFC1 = 0x421,
        BBC1_AFFTM = 0x422,
        BBC1_AFFVM = 0x423,
        BBC1_AFS = 0x424,
        BBC1_MACEA0 = 0x425,
        BBC1_MACEA1 = 0x426,
        BBC1_MACEA2 = 0x427,
        BBC1_MACEA3 = 0x428,
        BBC1_MACEA4 = 0x429,
        BBC1_MACEA5 = 0x42A,
        BBC1_MACEA6 = 0x42B,
        BBC1_MACEA7 = 0x42C,
        BBC1_MACPID0F0 = 0x42D,
        BBC1_MACPID1F0 = 0x42E,
        BBC1_MACSHA0F0 = 0x42F,
        BBC1_MACSHA1F0 = 0x430,
        BBC1_MACPID0F1 = 0x431,
        BBC1_MACPID1F1 = 0x432,
        BBC1_MACSHA0F1 = 0x433,
        BBC1_MACSHA1F1 = 0x434,
        BBC1_MACPID0F2 = 0x435,
        BBC1_MACPID1F2 = 0x436,
        BBC1_MACSHA0F2 = 0x437,
        BBC1_MACSHA1F2 = 0x438,
        BBC1_MACPID0F3 = 0x439,
        BBC1_MACPID1F3 = 0x43A,
        BBC1_MACSHA0F3 = 0x43B,
        BBC1_MACSHA1F3 = 0x43C,
        BBC1_AMCS = 0x440,
        BBC1_AMEDT = 0x441,
        BBC1_AMAACKPD = 0x442,
        BBC1_AMAACKTL = 0x443,
        BBC1_AMAACKTH = 0x444,
        BBC1_FSKC0 = 0x460,
        BBC1_FSKC1 = 0x461,
        BBC1_FSKC2 = 0x462,
        BBC1_FSKC3 = 0x463,
        BBC1_FSKC4 = 0x464,
        BBC1_FSKPLL = 0x465,
        BBC1_FSKSFD0L = 0x466,
        BBC1_FSKSFD0H = 0x467,
        BBC1_FSKSFD1L = 0x468,
        BBC1_FSKSFD1H = 0x469,
        BBC1_FSKPHRTX = 0x46A,
        BBC1_FSKPHRRX = 0x46B,
        BBC1_FSKRPC = 0x46C,
        BBC1_FSKRPCONT = 0x46D,
        BBC1_FSKRPCOFFT = 0x46E,
        BBC1_FSKRRXFLL = 0x470,
        BBC1_FSKRRXFLH = 0x471,
        BBC1_FSKDM = 0x472,
        BBC1_FSKPE0 = 0x473,
        BBC1_FSKPE1 = 0x474,
        BBC1_FSKPE2 = 0x475,
        BBC1_PMUC = 0x480,
        BBC1_PMUVAL = 0x481,
        BBC1_PMUQF = 0x482,
        BBC1_PMUI = 0x483,
        BBC1_PMUQ = 0x484,
        BBC1_CNTC = 0x490,
        BBC1_CNT0 = 0x491,
        BBC1_CNT1 = 0x492,
        BBC1_CNT2 = 0x493,
        BBC1_CNT3 = 0x494,
    };

} // namespace AT86RF215
