#include "COMMS_ECSS_Configuration.hpp"

#ifdef SERVICE_PARAMETER

#include "Services/ParameterService.hpp"
#include "PlatformParameters.hpp"

void ParameterService::initializeParameterMap() {
    parameters = {
          {COMMSParameters::COMMSUHFBandPATemperature, COMMSParameters::commsUHFBandPATemperature},
          {COMMSParameters::COMMSGNSSTemperature, COMMSParameters::commsGNSSTemperature},
          {COMMSParameters::COMMSPCBTemperature, COMMSParameters::commsPCBTemperature},
          {COMMSParameters::COMMSAntennaDeploymentStatus, COMMSParameters::commsAntennaDeploymentStatus},
          {COMMSParameters::COMMSDataRateUHFTX, COMMSParameters::commsDataRateUHFTX},
          {COMMSParameters::COMMSDataRateUHFRX, COMMSParameters::commsDataRateUHFRX},
          {COMMSParameters::COMMSSymbolRateSBand, COMMSParameters::commsSymbolRateSBand},
          {COMMSParameters::COMMSCWInterval, COMMSParameters::commsCWInterval},
          {COMMSParameters::COMMSGMSKBeaconInterval, COMMSParameters::commsGMSKBeaconInterval},
          {COMMSParameters::COMMSUHFBandTXPower, COMMSParameters::commsUHFBandTXPower},
          {COMMSParameters::COMMSSBandTXPower, COMMSParameters::commsSBandTXPower},
          {COMMSParameters::COMMSChannelNumberUHFBand, COMMSParameters::commsChannelNumberUHFBand},
          {COMMSParameters::COMMSChannelNumberSBand, COMMSParameters::commsChannelNumberSBand},
          {COMMSParameters::COMMSLNAGain, COMMSParameters::commsLNAGain},
          {COMMSParameters::COMMSPAGainUHFBand, COMMSParameters::commsPAGainUHFBand},
          {COMMSParameters::COMMSPAGainSBand, COMMSParameters::commsPAGainSBand},
          {COMMSParameters::COMMSVGAGain, COMMSParameters::commsVGAGain},
          {COMMSParameters::COMMSRSSI, COMMSParameters::commsRSSI},
          {COMMSParameters::COMMSUHFBandTXOnOff, COMMSParameters::commsUHFBandTXOnOff},
          {COMMSParameters::COMMSUHFBandRXOnOff, COMMSParameters::commsUHFBandRXOnOff},
          {COMMSParameters::COMMSSBandTXOnOff, COMMSParameters::commsSBandTXOnOff},
          {COMMSParameters::COMMSPacketsRejectedCOMMS, COMMSParameters::commsPacketsRejectedCOMMS},
          {COMMSParameters::COMMSInvalidHMAC, COMMSParameters::commsInvalidHMAC},
          {COMMSParameters::COMMSInvalidPacketStructure, COMMSParameters::commsInvalidPacketStructure},
          {COMMSParameters::COMMSInvalidSpacecraftID, COMMSParameters::commsInvalidSpacecraftID},
          {COMMSParameters::COMMSFrameSequenceCounter, COMMSParameters::commsFrameSequenceCounter},
          {COMMSParameters::COMMSPCBTemperature1, COMMSParameters::commsPCBTemperature1},
          {COMMSParameters::COMMSPCBTemperature2, COMMSParameters::commsPCBTemperature2},
          {COMMSParameters::COMMSMCUTemperature, COMMSParameters::commsMCUTemperature},
          {COMMSParameters::COMMSMCUInputVoltage, COMMSParameters::commsMCUInputVoltage},
          {COMMSParameters::COMMSMCUBootCounter, COMMSParameters::commsMCUBootCounter},
          {COMMSParameters::COMMSOnBoardTime, COMMSParameters::commsOnBoardTime},
          {COMMSParameters::COMMSNANDCurrentlyUsedMemoryPartition, COMMSParameters::commsNANDCurrentlyUsedMemoryPartition},
          {COMMSParameters::COMMSLastFailedEvent, COMMSParameters::commsLastFailedEvent},
          {COMMSParameters::COMMSMCUSystick, COMMSParameters::commsMCUSystick},
          {COMMSParameters::COMMSFlashInt, COMMSParameters::commsFlashInt},
          {COMMSParameters::COMMSSRAMInt, COMMSParameters::commsSRAMInt},
    };
}

#endif