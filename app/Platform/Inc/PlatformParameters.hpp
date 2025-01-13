#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpsabi" // Suppress: parameter passing for argument of type 'Time::DefaultCUC' {aka 'TimeStamp<4, 0, 1, 10>'} changed in GCC 7.1

#include "Helpers/Parameter.hpp"
//#include "CAN/Driver.hpp"

namespace COMMSParameters {
    enum ParameterID : uint16_t {
        // TODO: Update accordingly, once Parameter Database is updated
        COMMSUHFBandPATemperature = 2000,
        COMMSSBandPATemperature = 2001,
        COMMSPCBTemperature = 2002,
        COMMSAntennaDeploymentStatus = 2003,
        COMMSDataRateUHFTX = 2004,
        COMMSDataRateUHFRX = 2005,
        COMMSSymbolRateSBand = 2006,
        COMMSCWInterval = 2007,
        COMMSGMSKBeaconInterval = 2008,
        COMMSUHFBandTXPower = 2009,
        COMMSSBandTXPower = 2010,
        COMMSChannelNumberUHFBand = 2011,
        COMMSChannelNumberSBand = 2012,
        COMMSLNAGain = 2013,
        COMMSPAGainUHFBand = 2014,
        COMMSPAGainSBand = 2015,
        COMMSVGAGain = 2016,
        COMMSRSSI = 2017,
        COMMSUHFBandTXOnOff = 2018,
        COMMSUHFBandRXOnOff = 2019,
        COMMSSBandTXOnOff = 2020,
        COMMSPacketsRejectedCOMMS = 2021,
        COMMSInvalidHMAC = 2022,
        COMMSInvalidPacketStructure = 2023,
        COMMSInvalidSpacecraftID = 2024,
        COMMSFrameSequenceCounter = 2025,
        COMMSPCBTemperature1 = 2026,
        COMMSPCBTemperature2 = 2027,
        COMMSMCUTemperature = 2028,
        COMMSMCUInputVoltage = 2029,
        COMMSMCUBootCounter = 2030,
        COMMSOnBoardTime = 2031,
        COMMSNANDCurrentlyUsedMemoryPartition = 2032,
        COMMSLastFailedEvent = 2033,
        COMMSMCUSystick = 2034,
        COMMSFlashInt = 2035,
        COMMSSRAMInt = 2036
    };

    enum AntennaDeploymentStatus : uint8_t {
        Closed = 0,
        OneDoorOpen = 1,
        TwoDoorOpen = 2,
        ThreeDoorOpen = 3,
        FullyDeployed = 4
    };

    enum SampleRateUHFTX : uint8_t {
        Rate = 0
    };

    enum AntennaGains : uint8_t {
        Gain = 0
    };

    enum MemoryPartition : uint8_t {
        First = 0,
        Second = 1
    };

    enum CANBUSActive : uint8_t {
        Main = 0,
        Reductant = 1
    };

    inline Parameter<float> commsUHFBandPATemperature(0);
    inline Parameter<float> commsSBandPATemperature(0);
    inline Parameter<float> commsPCBTemperature(0);

    inline Parameter<AntennaDeploymentStatus> commsAntennaDeploymentStatus(Closed); // enum

    inline Parameter<SampleRateUHFTX> commsDataRateUHFTX(Rate); // enum
    inline Parameter<uint32_t> commsDataRateUHFRX(0);
    inline Parameter<uint32_t> commsSymbolRateSBand(0);
    inline Parameter<uint16_t> commsCWInterval(0);
    inline Parameter<uint16_t> commsGMSKBeaconInterval(0);
    inline Parameter<uint32_t> commsUHFBandTXPower(0);
    inline Parameter<uint32_t> commsSBandTXPower(0);
    inline Parameter<uint32_t> commsChannelNumberUHFBand(0);
    inline Parameter<uint32_t> commsChannelNumberSBand(0);

    inline Parameter<AntennaGains> commsLNAGain(Gain);       // enum
    inline Parameter<AntennaGains> commsPAGainUHFBand(Gain); // enum
    inline Parameter<AntennaGains> commsPAGainSBand(Gain);   // enum

    inline Parameter<uint8_t> commsVGAGain(0);
    inline Parameter<float> commsRSSI(0);

    inline Parameter<bool> commsUHFBandTXOnOff(0);
    inline Parameter<bool> commsUHFBandRXOnOff(0);
    inline Parameter<bool> commsSBandTXOnOff(0);

    inline Parameter<uint16_t> commsPacketsRejectedCOMMS(0);
    inline Parameter<uint16_t> commsInvalidHMAC(0);
    inline Parameter<uint16_t> commsInvalidPacketStructure(0);
    inline Parameter<uint16_t> commsInvalidSpacecraftID(0);
    inline Parameter<uint16_t> commsFrameSequenceCounter(0);

    inline Parameter<float> commsPCBTemperature1(0);
    inline Parameter<float> commsPCBTemperature2(0);
    inline Parameter<float> commsMCUTemperature(0);
    inline Parameter<float> commsMCUInputVoltage(0);

    inline Parameter<uint32_t> commsMCUBootCounter(0);
    inline Parameter<Time::DefaultCUC> commsOnBoardTime(Time::DefaultCUC(0));

    inline Parameter<MemoryPartition> commsNANDCurrentlyUsedMemoryPartition(First); // enum

    inline Parameter<uint16_t> commsLastFailedEvent(0);
    inline Parameter<uint32_t> commsMCUSystick(0);
    inline Parameter<uint32_t> commsFlashInt(0);
    inline Parameter<uint32_t> commsSRAMInt(0);

} // namespace COMMSParameters

#pragma GCC diagnostic pop