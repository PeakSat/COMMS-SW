#pragma once
#include "etl/vector.h"
// Task notification defines
// flags
#define GNSS_ACK (1 << 0)
#define GNSS_MESSAGE_READY (1 << 1)
// indexes
#define GNSS_INDEX_MESSAGE 1
#define GNSS_INDEX_ACK 2


namespace GNSSDefinitions {

    enum class Error {
        NoError,
        TransmissionFailed,
        Timeout,
        NACKReceived,
        MultipleCommandsFail
    };
    enum class Status{
        OK,
        ERROR
    };


    struct GNSSData {
        int32_t utc;
        int8_t day;
        int8_t month;
        int8_t year;
        int8_t hours;
        int8_t minutes;
        int8_t seconds;
        uint8_t valid;
        int32_t microseconds;
        double speed;
        double course;
        double latitudeD;
        double longitudeD;
        double altitude;
        int32_t latitudeI;
        int32_t longitudeI;
        int32_t altitudeI;
        int8_t fix_quality;
        int8_t satellites_tracked;
        int snr[4];
    };

    struct StoredGNSSData {
        int32_t utc_time;
        int32_t latitudeI;
        int32_t longitudeI;
        int32_t altitudeI;
    };


    inline constexpr uint8_t GNSSQueueSize = 1;

    /**
 * The maximum size for the data field of a GNSS Payload.
 */
    inline constexpr uint16_t GNSSPayloadSize = 1024;

    /**
 * The maximum size for the data field of a GNSS Message.
 */
    inline constexpr uint16_t GNSSMessageSize = GNSSPayloadSize + 16;

    typedef etl::vector<uint8_t, GNSSPayloadSize> Payload;

    enum GNSSMessages : uint8_t {
        SystemRestart = 0x1,
        QuerySoftwareVersion = 0x2,
        QuerySoftwareCRC = 0x3,
        SetFactoryDefaults = 0x4,
        ConfigureSerialPort = 0x5,
        ConfigureMessageType = 0x9,
        ConfigurePowerMode = 0xC,
        ConfigurePositionUpdateRate = 0xE,
        QueryPositionUpdateRate = 0x10,
        QueryPowerMode = 0x15,
        ConfigureDOPMask = 0x2A,
        ConfigureElevationAndCNRMask = 0x2B,
        QueryDOPMask = 0x2E,
        QueryElevationAndCNRMask = 0x2F,
        GetGPSEphemeris = 0x30,
        ConfigurePositionPinning = 0x39,
        QueryPositionPinning = 0x3A,
        ConfigurePositionPinningParameters = 0x3B,
        SetGPSEphemeris = 0x41,
        Query1PPSTiming = 0x44,
        Configure1PPSCableDelay = 0x45,
        Query1PPSCableDelay = 0x46,
        ConfigureNMEATalkerID = 0x4B,
        QueryNMEATalkID = 0x4F,
        Configure1PPSTiming = 0x54,
        Configure1PPSOutputMode = 0x55,
        Query1PPSOutputMode = 0x56,
        GetGLONASSEphemeris = 0x5B,
        SetGLONASSEphemeris = 0x5C,
        GetGLONASSTimeCorrectionParameters = 0x5F,
        SetGLONASSTimeCorrectionParameters = 0x60,
        IDusedForMessagesWithSubID_1 = 0x64,
        IDusedForMessagesWithSubID_2 = 0x63
    };

    enum GNSSMessagesSubID : uint16_t {
        ConfigureSBAS = (0x62 << 8) | 0x1,
        QuerySBASStatus = (0x62 << 8) | 0x2,
        ConfigureQZSS = (0x62 << 8) | 0x3,
        QueryQZSSStatus = (0x62 << 8) | 0x4,
        ConfigureSBASAdvanced = (0x62 << 8) | 0x5,
        QuerySBASAdvanced = (0x62 << 8) | 0x6,
        SBASStatus = (0x62 << 8) | 0x80,
        QZSSStatus = (0x62 << 8) | 0x81,
        SBASAdvanced = (0x62 << 8) | 0x82,
        ConfigureSAEE = (0x63 << 8) | 0x1,
        QuerySAEE = (0x63 << 8) | 0x2,
        SAEEStatus = (0x63 << 8) | 0x80,
        QueryBootStatus = (0x64 << 8) | 0x1,
        ConfigureExtendedNMEAMessageInterval = 0x2,
        QueryExtendedNMEAMessageInterval = (0x64 << 8) | 0x3,
        ConfigureInterferenceDetection = (0x64 << 8) | 0x6,
        QueryInterferenceDetectionStatus = 0x7,
        QueryGNSSParameterSearchEngineNumber = (0x64 << 8) | 0xB,
        ConfigurePositionFixNavigationMask = (0x64 << 8) | 0x11,
        QueryPositionFixNavigationMask = (0x64 << 8) | 0x12,
        ConfigureUTCReferenceTimeSyncToGPSTime = (0x64 << 8) | 0x15,
        QueryUTCReferenceTimeSyncToGPSTime = (0x64 << 8) | 0x16,
        ConfigureGNSSNavigationMode = 0x17,
        QueryGNSSNavigationMode = (0x64 << 8) | 0x18,
        ConfigureGNSSConstellationTypeForNavigationSolution = (0x64 << 8) | 0x19,
        QueryGNSSConstellationTypeForNavigationSolution = (0x64 << 8) | 0x1A,
        SoftwareImageDownloadUsingROMExternalLoader = (0x64 << 8) | 0x1B,
        ConfigureGNSSDozeMode = (0x64 << 8) | 0x1C,
        QueryGPSTime = (0x64 << 8) | 0x20,
        ConfigurePSTIMessageInterval = (0x64 << 8) | 0x21,
        QueryPSTIMessageInterval = (0x64 << 8) | 0x22,
        QueryRequestedPSTIMessageInterval = (0x64 << 8) | 0x23,
        ConfigureGNSSDatumIndex = (0x64 << 8) | 0x27,
        QueryGNSSDatumIndex = (0x64 << 8) | 0x28,
        ConfigureGPSUTCLeapSecondsInUTC = (0x64 << 8) | 0x2D,
        ConfigureNavigationDataMessageInterval = (0x64 << 8) | 0x2F,
        QueryNavigationDataMessageInterval = (0x64 << 8) | 0x30,
        ConfigureGNSSGeoFencingDataByPolygon = (0x64 << 8) | 0x34,
        QueryGNSSGeoFencingDataByPolygon = (0x64 << 8) | 0x35,
        QueryGNSSMultiPolygonGeoFencingResult = (0x64 << 8) | 0x36,
        ConfigureNMEAStringInterval = 0x3B,
        QueryNMEAStringInterval = (0x64 << 8) | 0x3C,
        QueryRequestedNMEAStringInterval = (0x64 << 8) | 0x40,
        SoftwareImageDownloadUsingInternalLoader = (0x64 << 8) | 0x4E,
        SoftwareImageDownloadUsingExternalLoader = (0x64 << 8) | 0x4F,
        QueryVersionExtensionString = (0x64 << 8) | 0x7D,
        GNSSBootStatus = (0x64 << 8) | 0x80,
        ExtendedNMEAMessageInterval = (0x64 << 8) | 0x81,
        InterferenceDetectionStatus = (0x64 << 8) | 0x83,
        GNSSParameterSearchEngineNumber = (0x64 << 8) | 0x85,
        PositionFixNavigationMask = (0x64 << 8) | 0x88,
        GPSUTCReferenceTime = (0x64 << 8) | 0x8A,
        GNSSNavigationMode = (0x64 << 8) | 0x8B,
        GNSSConstellationTypeForNavigationSolution = (0x64 << 8) | 0x8C,
        GPSTime = (0x64 << 8) | 0x8E,
        PSTIMessageInterval = (0x64 << 8) | 0x8F,
        RequestedPSTIMessageInterval = (0x64 << 8) | 0x90,
        GNSSDatumIndex = (0x64 << 8) | 0x92,
        NavigationDataMessageInterval = (0x64 << 8) | 0x98,
        GNSSGeoFencingDataByPolygon = (0x64 << 8) | 0x99,
        GNSSMultiPolygonGeoFencingResult = (0x64 << 8) | 0x9A,
        NMEAStringInterval = (0x64 << 8) | 0x9D,
        RequestedNMEAStringInterval = (0x64 << 8) | 0x9F,
        VersionExtensionString = (0x64 << 8) | 0xFE,
        Configure1PPSPulseWidth = (0x65 << 8) | 0x1,
        Query1PPSPulseWidth = (0x65 << 8) | 0x2,
        ConfigurePPS2FrequencyOutput = (0x65 << 8) | 0x3,
        QueryPPS2FrequencyOutput = (0x65 << 8) | 0x4,
        OnePPSPulseWidth = (0x65 << 8) | 0x80,
        GNSSPPS2FrequencyOutput = (0x65 << 8) | 0x81,
        SetBeidouEphemerisData = (0x67 << 8) | 0x1,
        GetBeidouEphemerisData = (0x67 << 8) | 0x2,
        BeidouEphemerisData = (0x67 << 8) | 0x80,
        ConfigureRTKModeAndOperationalFunction = (0x6A << 8) | 0x6,
        QueryRTKModeAndOperationalFunction = (0x6A << 8) | 0x7,
        ConfigureRTKSlaveBaseSerialPortBaudRate = (0x6A << 8) | 0xC,
        QueryRTKSlaveBaseSerialPortBaudRate = (0x6A << 8) | 0xD,
        ConfigureRTKPreciselyKinematicBaseSerialPortBaudRate = (0x6A << 8) | 0x13,
        QueryRTKPreciselyKinematicBaseSerialPortBaudRate = (0x6A << 8) | 0x14,
        ConfigureRTKRoverMovingBaseHeadingAndPitchOffsets = (0x6A << 8) | 0x15,
        QueryRTKRoverMovingBaseHeading = (0x6A << 8) | 0x16,
        RTKModeAndOperationalFunction = (0x6A << 8) | 0x83,
        RTKSlaveBaseSerialPortBaudRate = (0x6A << 8) | 0x85,
        RTKPreciselyKinematicBaseSerialPortBaudRate = (0x6A << 8) | 0x88,
        HeadingAndPitchOffsetsOfRTKRoverMovingBase = (0x6A << 8) | 0x89,
        SetGALILEOEphemeris = (0x6E << 8) | 0x1,
        GetGALILEOEphemeris = (0x6E << 8) | 0x2,
        GALILEOEphemerisData = (0x6E << 8) | 0x80,
        SetIRNSSEphemeris = (0x6F << 8) | 0x3,
        GetIRNSSEphemeris = (0x6F << 8) | 0x4,
        IRNSSEphemerisData = (0x6F << 8) | 0x81
    };


    enum class StartMode : uint8_t {
        Reserved,
        HotStart,
        WarmStart,
        ColdStart
    };

    enum class PositionRate : uint8_t {
        Option1Hz = 1,
        Option2Hz,
        Option4Hz = 4,
        Option5Hz,
        Option10Hz = 10,
        Option20Hz = 20
    };

    enum class SoftwareType : uint8_t {
        Reserved,
        SystemCode,
    };

    enum class DefaultType : uint8_t {
        Reserved,
        RebootAfterSettingToFactoryDefaults
    };

    enum class BaudRate : uint8_t {
        Option4800,
        Option9600,
        Option19200,
        Option38400,
        Option57600,
        Option115200,
        Option230400,
        Option460800,
        Option921600
    };

    enum class Attributes : uint8_t {
        UpdateToSRAM,
        UpdateSRAMandFLASH,
        Temporarily,
    };

    enum class ConfigurationType : uint8_t {
        NoOutput,
        NMEA,
        Binary
    };

    enum class PowerMode : uint8_t {
        Normal,
        PowerSave
    };

    enum class DOPModeSelect : uint8_t {
        Disable,
        AutoMode, //PDOP when 3D fix and GDOP when 2D Fix
        PDOPOnly,
        HDOPOnly,
        GDOPOnly
    };

    enum class ElevationAndCNRModeSelect : uint8_t {
        Disable,
        ElevationAndCNRMask,
        ElevationMaskOnly,
        CNRMaskOnly
    };

    enum class PositionPinning : uint8_t {
        Default,
        Enable,
        Disable
    };

    enum class TalkerIDType : uint8_t {
        GPMode,
        GNMode,
        AutoMode // according to NMEA 4.11 to combine GNSS system solution to output GN, GP, GL, GA, GB or appropriately.
    };

    enum class TimingMode : uint8_t {
        PVTMode,
        SurveyMode,
        StaticMode
    };

    enum class OutputMode : uint8_t {
        Reserved,
        OutputIfGNSSAvailable,
        OutputAlwaysAndAignToGNSS
    };

    enum class AlignSource {
        GNSS,
        UTC
    };

    enum class SBASConfiguration : uint8_t {
        Enable,
        Ranging,
        RangingURAMask,
        Correction,
        NumberOfTrackingChannels,
        SubsystemMask
    };

    enum class EnableSBAS : uint8_t {
        Disable,
        Enable,
    };

    enum class RangingSBAS : uint8_t {
        DoNotUse,
        Use,
        Auto
    };

    enum class CorrectionSBAS : uint8_t {
        Disable,
        Enable
    };

    enum class EnableQZSS : uint8_t {
        Disable,
        Enable
    };

    enum class EnableSAEE : uint8_t {
        SAEEModeDefault,
        SAEEModeEnable,
        SAEEModeDisable
    };

    enum class InterferenceDetectControl : uint8_t {
        Disable,
        Enable
    };

    enum class FirstFixNavigationMask : uint8_t {
        Mask3D,
        Mask2D
    };

    enum class SubsequentFixNavigationMask : uint8_t {
        Mask3D,
        Mask2D
    };

    enum class EnableSyncToGPSTime : uint8_t {
        Disable,
        Enable
    };

    enum class NavigationMode : uint8_t {
        Auto,
        Pedestrian,
        Car,
        Marine,
        Ballon,
        Airborne,
        Reserved,
        Quadcopter,
        Reserved2,
        SLRMode
    };

    enum class FlashType : uint8_t {
        DefaultAuto,
        QSPIWindbond,
        QSPIEON,
        ParallelFlashNUMONYX,
        ParallelFlashEON,
    };

    enum class BufferUsedIndex : uint8_t {
        Index8k,
        Index16k,
        Index24k,
        Index32k
    };

} // namespace GNSSDefinitions