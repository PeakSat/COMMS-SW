#include "GNSS.hpp"
#include "etl/vector.h"
#include "etl/string.h"

#include <Logger.hpp>
#include <eMMC.hpp>
#include <ctime> // For std::tm and std::mktime

bool GNSSReceiver::isDataValid(int8_t year, int8_t month, int8_t day) {
    if (year < 20 || year > 79) { return false; }
    if (month < 0 || month > 12) { return false; }
    if (day < 0 || day > 32) { return false; }
    return true;
}

uint32_t GNSSReceiver::findTailPointer() {

    uint32_t numOfBlocks = eMMC::memoryMap[eMMC::GNSSData].size / eMMC::memoryPageSize;

    GNSSDefinitions::StoredGNSSData lastData{};
    auto status = eMMC::getItem(eMMC::memoryMap[eMMC::GNSSData], reinterpret_cast<uint8_t*>(&lastData), eMMC::memoryPageSize, 0, 1);
    if (lastData.valid != 0xAA) {
        return 0;
    }
    uint32_t latest_blockPointer = 1;
    uint64_t latest_timestamp = lastData.usFromEpoch_NofSat[GNSS_MEASUREMENTS_PER_STRUCT - 1] >> 5;


    for (int i = 1; i < numOfBlocks; i++) {
        status = eMMC::getItem(eMMC::memoryMap[eMMC::GNSSData], reinterpret_cast<uint8_t*>(&lastData), eMMC::memoryPageSize, i, 1);
        if (lastData.valid == 0xAA) {

            uint64_t lasttimestamp = lastData.usFromEpoch_NofSat[GNSS_MEASUREMENTS_PER_STRUCT - 1] >> 5;

            if (lasttimestamp >= latest_timestamp) {
                latest_timestamp = lasttimestamp;
                latest_blockPointer = i + 1;
            } else {
                return latest_blockPointer;
            }
        } else {
            return latest_blockPointer;
        }
    }
    return latest_blockPointer;
}

void GNSSReceiver::sendGNSSData(uint8_t period, uint32_t daysPrior, uint32_t numberOfSamples) {
    GNSSDefinitions::StoredGNSSData storedData{};
    GNSSDefinitions::StoredGNSSData dataToBeSent1{};
    GNSSDefinitions::StoredGNSSData dataToBeSent2{};
    uint32_t localTailPointer = eMMCDataTailPointer;
    uint32_t sampleCounter = 0;

    if (eMMCDataTailPointer == 0) {
        auto status = eMMC::getItem(eMMC::memoryMap[eMMC::GNSSData], reinterpret_cast<uint8_t*>(&storedData), eMMC::memoryPageSize, eMMCDataTailPointer, 1);
        if (storedData.valid == 0xAA) {
            // todo: handle error, no GNSS data
            LOG_ERROR << "Requested GNSS data but there are no data in memory";
        }
    }

    int32_t structIterator = GNSS_MEASUREMENTS_PER_STRUCT - 1;
    uint32_t dataToBeSentIterator = 0;
    uint64_t newerTimestamp = storedData.usFromEpoch_NofSat[GNSS_MEASUREMENTS_PER_STRUCT - 1] >> 5;
    while (sampleCounter < numberOfSamples) {
        uint64_t olderTimestamp = storedData.usFromEpoch_NofSat[structIterator] >> 5;
        if (newerTimestamp - olderTimestamp > period) {
            if (dataToBeSentIterator < GNSS_MEASUREMENTS_PER_STRUCT) {
                dataToBeSent1.altitudeI[dataToBeSentIterator] = storedData.altitudeI[dataToBeSentIterator];
            } else {
                dataToBeSent2.altitudeI[dataToBeSentIterator - GNSS_MEASUREMENTS_PER_STRUCT] = storedData.altitudeI[dataToBeSentIterator - GNSS_MEASUREMENTS_PER_STRUCT];
                if (dataToBeSentIterator >= GNSS_MEASUREMENTS_PER_STRUCT * 2) {
                    // todo: send to TM queue
                    dataToBeSentIterator = 0;
                }
            }

            dataToBeSentIterator++;
            sampleCounter++;
        }

        structIterator--;
        if (structIterator < 0) {
            structIterator = GNSS_MEASUREMENTS_PER_STRUCT - 1;
            // todo: get previous page from emmc
        }
    }
}
uint32_t GNSSReceiver::timeOfDaytoMinutes(uint32_t ToD) {
}

GNSSMessage GNSSReceiver::configureNMEATalkerID(TalkerIDType type, Attributes attributes) {
    Payload payload;
    payload.push_back(GNSSDefinitions::GNSSMessages::ConfigureNMEATalkerID);
    payload.push_back(static_cast<uint8_t>(type));
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage GNSSReceiver::querySoftwareVersion(SoftwareType softwareType) {
    Payload payload;
    payload.push_back(GNSSMessages::QuerySoftwareVersion);
    payload.push_back(static_cast<uint8_t>(softwareType));
    return GNSSMessage{static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage GNSSReceiver::setFactoryDefaults(DefaultType type) {
    Payload payload;
    payload.push_back(GNSSDefinitions::GNSSMessages::SetFactoryDefaults);
    payload.push_back(static_cast<const unsigned char>(type));
    return GNSSMessage{static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage GNSSReceiver::configureSystemPositionRate(PositionRate rate, Attributes attributes) {
    Payload payload;
    payload.push_back(GNSSDefinitions::GNSSMessages::ConfigurePositionUpdateRate);
    payload.push_back(static_cast<const unsigned char>(rate));
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage GNSSReceiver::queryInterferenceDetectionStatus() {
    Payload payload;
    payload.push_back(GNSSDefinitions::GNSSMessages::IDusedForMessagesWithSubID_1);
    payload.push_back(GNSSDefinitions::GNSSMessagesSubID::QueryInterferenceDetectionStatus);
    return GNSSMessage{static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage GNSSReceiver::configureMessageType(ConfigurationType type, Attributes attributes) {
    Payload payload;
    payload.push_back(GNSSMessages::ConfigureMessageType);
    payload.push_back(static_cast<uint8_t>(type));
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage GNSSReceiver::configureGNSSNavigationMode(NavigationMode mode, Attributes attributes) {
    Payload payload;
    payload.push_back(GNSSDefinitions::GNSSMessages::IDusedForMessagesWithSubID_1);
    payload.push_back(GNSSDefinitions::GNSSMessagesSubID::ConfigureGNSSNavigationMode);
    payload.push_back(static_cast<uint8_t>(mode));
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{static_cast<uint16_t>(payload.size()), payload};
}


GNSSMessage GNSSReceiver::configureNMEAStringInterval(etl::string<3> str, uint8_t interval, Attributes attributes) {
    Payload payload;
    payload.push_back(GNSSDefinitions::GNSSMessages::IDusedForMessagesWithSubID_1);
    payload.push_back(GNSSDefinitions::GNSSMessagesSubID::ConfigureNMEAStringInterval);
    auto first = static_cast<uint8_t>(str[0]);
    auto second = static_cast<uint8_t>(str[1]);
    auto third = static_cast<uint8_t>(str[2]);
    payload.push_back(first);
    payload.push_back(second);
    payload.push_back(third);
    payload.push_back(interval);
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage GNSSReceiver::configureExtendedNMEAMessageInterval(const etl::vector<uint8_t, 12>& intervals, Attributes attributes) {
    Payload payload;
    payload.push_back(GNSSDefinitions::GNSSMessages::IDusedForMessagesWithSubID_1);
    payload.push_back(GNSSDefinitions::GNSSMessagesSubID::ConfigureExtendedNMEAMessageInterval);
    for (uint8_t interval: intervals) {
        payload.push_back(interval);
    }
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{static_cast<uint16_t>(payload.size()), payload};
}


GNSSMessage GNSSReceiver::query1PPSTiming() {
    Payload payload;
    payload.push_back(GNSSDefinitions::GNSSMessages::Query1PPSTiming);
    return GNSSMessage{static_cast<uint16_t>(payload.size()), payload};
}


GNSSMessage GNSSReceiver::configureSystemPowerMode(PowerMode mode, Attributes attributes) {
    Payload payload;
    payload.push_back(GNSSDefinitions::GNSSMessages::ConfigurePowerMode);
    payload.push_back(static_cast<uint8_t>(mode));
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{static_cast<uint16_t>(payload.size()), payload};
}


GNSSMessage
GNSSReceiver::configureDOPMask(DOPModeSelect mode, uint16_t PDOPvalue, uint16_t HDOPvalue, uint16_t GDOPvalue,
                               Attributes attributes) {
    Payload payload;
    payload.push_back(static_cast<uint8_t>(mode));
    payload.push_back((PDOPvalue >> 8) & 0xFF);
    payload.push_back(PDOPvalue & 0xFF);
    payload.push_back((HDOPvalue >> 8) & 0xFF);
    payload.push_back(HDOPvalue & 0xFF);
    payload.push_back((GDOPvalue >> 8) & 0xFF);
    payload.push_back(GDOPvalue & 0xFF);
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessages::ConfigureDOPMask, static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage
GNSSReceiver::configureElevationAndCNRMask(ElevationAndCNRModeSelect mode, uint8_t elevationMask, uint8_t CNRMask,
                                           Attributes attributes) {
    Payload payload;
    payload.push_back(static_cast<uint8_t>(mode));
    payload.push_back(elevationMask);
    payload.push_back(CNRMask);
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessages::ConfigureElevationAndCNRMask, static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage GNSSReceiver::configurePositionPinning(PositionPinning positionPinning, Attributes attributes) {
    Payload payload;
    payload.push_back(static_cast<uint8_t>(positionPinning));
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessages::ConfigurePositionPinning, static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage
GNSSReceiver::configurePositionPinningParameters(uint16_t pinningSpeed, uint16_t pinningCnt, uint16_t unpiningSpeed,
                                                 uint16_t unpiningCnt, uint16_t unpinningDistance,
                                                 Attributes attributes) {
    Payload payload;
    payload.push_back((pinningSpeed >> 8) & 0xFF);
    payload.push_back(pinningSpeed & 0xFF);
    payload.push_back((pinningCnt >> 8) & 0xFF);
    payload.push_back(pinningCnt & 0xFF);
    payload.push_back((unpiningSpeed >> 8) & 0xFF);
    payload.push_back(unpiningSpeed & 0xFF);
    payload.push_back((unpiningCnt >> 8) & 0xFF);
    payload.push_back(unpiningCnt & 0xFF);
    payload.push_back((unpinningDistance >> 8) & 0xFF);
    payload.push_back(unpinningDistance & 0xFF);
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessages::ConfigurePositionPinningParameters, static_cast<uint16_t>(payload.size()),
                       payload};
}

GNSSMessage GNSSReceiver::configure1PPSCableDelay(uint32_t cableDelay, Attributes attributes) {
    Payload payload;
    payload.push_back((cableDelay >> 24) & 0xFF);
    payload.push_back((cableDelay >> 16) & 0xFF);
    payload.push_back((cableDelay >> 8) & 0xFF);
    payload.push_back(cableDelay & 0xFF);
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessages::Configure1PPSCableDelay, static_cast<uint16_t>(payload.size()), payload};
}


GNSSMessage
GNSSReceiver::configure1PPSTiming(TimingMode mode, uint16_t surveyLength, uint16_t standardDeviation, uint16_t latitude,
                                  uint16_t longtitude, uint16_t altitude, Attributes attributes) {
    Payload payload;
    payload.push_back(static_cast<uint8_t>(mode));
    payload.push_back((surveyLength >> 8) & 0xFF);
    payload.push_back(surveyLength & 0xFF);
    payload.push_back((standardDeviation >> 8) & 0xFF);
    payload.push_back(standardDeviation & 0xFF);
    payload.push_back((latitude >> 8) & 0xFF);
    payload.push_back(latitude & 0xFF);
    payload.push_back((longtitude >> 8) & 0xFF);
    payload.push_back(longtitude & 0xFF);
    payload.push_back((altitude >> 8) & 0xFF);
    payload.push_back(altitude & 0xFF);
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessages::Configure1PPSTiming, static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage GNSSReceiver::configure1PPSOutputMode(OutputMode mode, AlignSource source, Attributes attributes) {
    Payload payload;
    payload.push_back(static_cast<uint8_t>(mode));
    payload.push_back(static_cast<uint8_t>(source));
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessages::Configure1PPSOutputMode, static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage
GNSSReceiver::configureSBAS(EnableSBAS enable, RangingSBAS ranging, uint8_t URAMask, CorrectionSBAS correction,
                            uint8_t numberOfTrackingChannels, uint8_t subsystemMask, Attributes attributes) {
    Payload payload;
    payload.push_back(static_cast<uint8_t>(enable));
    payload.push_back(static_cast<uint8_t>(ranging));
    payload.push_back(URAMask);
    payload.push_back(static_cast<uint8_t>(correction));
    payload.push_back(numberOfTrackingChannels);
    payload.push_back(subsystemMask);
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessagesSubID::ConfigureSBAS, static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage GNSSReceiver::configureQZSS(EnableQZSS enable, uint8_t noOfTrackingChannels, Attributes attributes) {
    Payload payload;
    payload.push_back(static_cast<uint8_t>(enable));
    payload.push_back(noOfTrackingChannels);
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessagesSubID::ConfigureQZSS, static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage GNSSReceiver::configureSBASAdvanced(EnableSBAS enable, RangingSBAS ranging, uint8_t rangingURAMask,
                                                CorrectionSBAS correction, uint8_t numTrackingChannels,
                                                uint8_t subsystemMask, uint8_t waasPRN, uint8_t egnosPRN,
                                                uint8_t msasPRN, uint8_t gaganPRN, uint8_t sdcmPRN, uint8_t bdsbasPRN,
                                                uint8_t southpanPRN, uint8_t attributes) {
    Payload payload;
    payload.push_back(static_cast<uint8_t>(enable));
    payload.push_back(static_cast<uint8_t>(ranging));
    payload.push_back(rangingURAMask);
    payload.push_back(static_cast<uint8_t>(correction));
    payload.push_back(numTrackingChannels);
    payload.push_back(subsystemMask);
    payload.push_back(waasPRN);
    payload.push_back(egnosPRN);
    payload.push_back(msasPRN);
    payload.push_back(gaganPRN);
    payload.push_back(sdcmPRN);
    payload.push_back(bdsbasPRN);
    payload.push_back(southpanPRN);
    payload.push_back(attributes);
    return GNSSMessage{GNSSMessagesSubID::ConfigureSBASAdvanced, static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage GNSSReceiver::configureSAEE(EnableSAEE enable, Attributes attributes) {
    Payload payload;
    payload.push_back(static_cast<uint8_t>(enable));
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessagesSubID::ConfigureSAEE, static_cast<uint16_t>(payload.size()), payload};
}


GNSSMessage GNSSReceiver::configureInterferenceDetection(InterferenceDetectControl control, Attributes attributes) {
    Payload payload;
    payload.push_back(static_cast<uint8_t>(control));
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessagesSubID::ConfigureInterferenceDetection, static_cast<uint16_t>(payload.size()),
                       payload};
}

GNSSMessage GNSSReceiver::configurePositionFixNavigationMap(FirstFixNavigationMask firstFix,
                                                            SubsequentFixNavigationMask subsequentFix,
                                                            Attributes attributes) {
    Payload payload;
    payload.push_back(static_cast<uint8_t>(firstFix));
    payload.push_back(static_cast<uint8_t>(subsequentFix));
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessagesSubID::ConfigurePositionFixNavigationMask, static_cast<uint16_t>(payload.size()),
                       payload};
}

GNSSMessage
GNSSReceiver::configureUTCReferenceTimeSyncToGPSTime(EnableSyncToGPSTime enable, uint16_t utcYear, uint8_t utcMonth,
                                                     uint8_t utcDay, Attributes attributes) {
    Payload payload;
    payload.push_back(static_cast<uint8_t>(enable));
    payload.push_back((utcYear >> 8) & 0xFF);
    payload.push_back(utcYear & 0xFF);
    payload.push_back(utcMonth);
    payload.push_back(utcDay);
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessagesSubID::ConfigureUTCReferenceTimeSyncToGPSTime, static_cast<uint16_t>(payload.size()),
                       payload};
}


GNSSMessage
GNSSReceiver::configureGNSSConstellationTypeForNavigationSolution(uint8_t constellationType, Attributes attributes) {
    Payload payload;
    payload.push_back(0x00);
    payload.push_back(constellationType);
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessagesSubID::ConfigureGNSSConstellationTypeForNavigationSolution,
                       static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage
GNSSReceiver::SoftwareImageDownloadUsingROMExternalLoader(BaudRate baud, FlashType flashType, uint16_t flashID,
                                                          BufferUsedIndex bufferUsedIndex, Attributes attributes) {
    Payload payload;
    payload.push_back(static_cast<uint8_t>(baud));
    payload.push_back(static_cast<uint8_t>(flashType));
    payload.push_back((flashID >> 8) & 0xFF);
    payload.push_back(flashID & 0xFF);
    payload.push_back(static_cast<uint8_t>(bufferUsedIndex));
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessagesSubID::SoftwareImageDownloadUsingROMExternalLoader,
                       static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage GNSSReceiver::configureGNSSDozeMode() {
    return GNSSMessage{GNSSMessagesSubID::ConfigureGNSSDozeMode, 0, {}};
}

GNSSMessage GNSSReceiver::configurePSTIMessageInterval(uint8_t PSTI_ID, uint8_t interval, Attributes attributes) {
    Payload payload;
    payload.push_back(PSTI_ID);
    payload.push_back(interval);
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessagesSubID::ConfigurePSTIMessageInterval, static_cast<uint16_t>(payload.size()),
                       payload};
}

GNSSMessage GNSSReceiver::configureGNSSDatumIndex(uint16_t datumIndex, Attributes attributes) {
    Payload payload;
    payload.push_back((datumIndex >> 8) & 0xFF);
    payload.push_back(datumIndex & 0xFF);
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessagesSubID::ConfigureGNSSDatumIndex, static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage GNSSReceiver::configureGPSUTCLeapSecondsInUTC(uint16_t utcYear, uint8_t utcMonth, uint8_t leapSeconds,
                                                          uint8_t insertSeconds, Attributes attributes) {
    Payload payload;
    payload.push_back((utcYear >> 8) & 0xFF);
    payload.push_back(utcYear & 0xFF);
    payload.push_back(utcMonth);
    payload.push_back(leapSeconds);
    payload.push_back(insertSeconds);
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessagesSubID::ConfigureGPSUTCLeapSecondsInUTC, static_cast<uint16_t>(payload.size()),
                       payload};
}

GNSSMessage
GNSSReceiver::configureNavigationDataMessageInterval(uint8_t navigationDataMessageInterval, Attributes attributes) {
    Payload payload;
    payload.push_back(navigationDataMessageInterval);
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessagesSubID::ConfigureNavigationDataMessageInterval, static_cast<uint16_t>(payload.size()),
                       payload};
}

GNSSMessage GNSSReceiver::configureGNSSGEOFencingDatabyPolygon() {
    return GNSSMessage{GNSSMessagesSubID::ConfigureGNSSGeoFencingDataByPolygon, 0, {}};
}


GNSSMessage GNSSReceiver::SoftwareImageDownloadUsingInternalLoader(BaudRate baud, FlashType flashType, uint16_t flashID,
                                                                   BufferUsedIndex bufferUsedIndex,
                                                                   Attributes attributes) {
    Payload payload;
    payload.push_back(static_cast<uint8_t>(baud));
    payload.push_back(static_cast<uint8_t>(flashType));
    payload.push_back((flashID >> 8) & 0xFF);
    payload.push_back(flashID & 0xFF);
    payload.push_back(static_cast<uint8_t>(bufferUsedIndex));
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessagesSubID::SoftwareImageDownloadUsingInternalLoader,
                       static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage GNSSReceiver::SoftwareImageDownloadUsingExternalLoader(BaudRate baud, FlashType flashType, uint16_t flashID,
                                                                   BufferUsedIndex bufferUsedIndex,
                                                                   Attributes attributes) {
    Payload payload;
    payload.push_back(static_cast<uint8_t>(baud));
    payload.push_back(static_cast<uint8_t>(flashType));
    payload.push_back((flashID >> 8) & 0xFF);
    payload.push_back(flashID & 0xFF);
    payload.push_back(static_cast<uint8_t>(bufferUsedIndex));
    payload.push_back(static_cast<uint8_t>(attributes));
    return GNSSMessage{GNSSMessagesSubID::SoftwareImageDownloadUsingExternalLoader,
                       static_cast<uint16_t>(payload.size()), payload};
}


GNSSMessage GNSSReceiver::querySoftwareCRC(SoftwareType softwareType) {
    Payload payload;
    payload.push_back(static_cast<uint8_t>(softwareType));
    return GNSSMessage{GNSSMessages::QuerySoftwareCRC, static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage GNSSReceiver::queryPositionUpdateRate() {
    return GNSSMessage{GNSSMessages::QueryPositionUpdateRate, 0, {}};
}

GNSSMessage GNSSReceiver::queryPowerMode() {
    return GNSSMessage{GNSSMessages::QueryPowerMode, 0, {}};
}

GNSSMessage GNSSReceiver::QueryDOPMask() {
    return GNSSMessage{GNSSMessages::QueryDOPMask, 0, {}};
}

GNSSMessage GNSSReceiver::QueryElevationandCNRMask() {
    return GNSSMessage{GNSSMessages::QueryElevationAndCNRMask, 0, {}};
}

GNSSMessage GNSSReceiver::queryPositionPinning() {
    return GNSSMessage{GNSSMessages::QueryPositionPinning, 0, {}};
}


GNSSMessage GNSSReceiver::query1PPSCableDelay() {
    return GNSSMessage{GNSSMessages::Query1PPSCableDelay, 0, {}};
}

GNSSMessage GNSSReceiver::queryNMEATalkerID() {
    return GNSSMessage{GNSSMessages::QueryNMEATalkID, 0, {}};
}

GNSSMessage GNSSReceiver::querySBASStatus() {
    return GNSSMessage{GNSSMessagesSubID::QuerySBASStatus, 0, {}};
}

GNSSMessage GNSSReceiver::queryQZSSStatus() {
    return GNSSMessage{GNSSMessagesSubID::QueryQZSSStatus, 0, {}};
}

GNSSMessage GNSSReceiver::querySBASAdvanced() {
    return GNSSMessage{GNSSMessagesSubID::QuerySBASAdvanced, 0, {}};
}

GNSSMessage GNSSReceiver::querySAEEStatus() {
    return GNSSMessage{GNSSMessagesSubID::QuerySAEE, 0, {}};
}

GNSSMessage GNSSReceiver::queryGNSSBootStatus() {
    return GNSSMessage{GNSSMessagesSubID::QueryBootStatus, 0, {}};
}

GNSSMessage GNSSReceiver::queryExtendedNMEAMesaageInterval() {
    return GNSSMessage{GNSSMessagesSubID::QueryExtendedNMEAMessageInterval, 0, {}};
}


GNSSMessage GNSSReceiver::queryGNSSParameterSearchEngineNumber() {
    return GNSSMessage{GNSSMessagesSubID::QueryGNSSParameterSearchEngineNumber, 0, {}};
}

GNSSMessage GNSSReceiver::queryPositionFixNavigationMap() {
    return GNSSMessage{GNSSMessagesSubID::QueryPositionFixNavigationMask, 0, {}};
}

GNSSMessage GNSSReceiver::QueryUTCReferenceTimeSyncToGPSTime() {
    return GNSSMessage{GNSSMessagesSubID::QueryUTCReferenceTimeSyncToGPSTime, 0, {}};
}

GNSSMessage GNSSReceiver::queryGNSSNavigationMode() {
    return GNSSMessage{GNSSMessagesSubID::QueryGNSSNavigationMode, 0, {}};
}

GNSSMessage GNSSReceiver::queryGNSSConstellationTypeForNavigationSolution() {
    return GNSSMessage{GNSSMessagesSubID::QueryGNSSConstellationTypeForNavigationSolution, 0, {}};
}

GNSSMessage GNSSReceiver::queryGPSTime() {
    return GNSSMessage{GNSSMessagesSubID::QueryGPSTime, 0, {}};
}

GNSSMessage GNSSReceiver::queryPSTIMessageInterval() {
    return GNSSMessage{GNSSMessagesSubID::QueryPSTIMessageInterval, 0, {}};
}

GNSSMessage GNSSReceiver::queryRequestedPSTIMessageInterval() {
    return GNSSMessage{GNSSMessagesSubID::QueryRequestedPSTIMessageInterval, 0, {}};
}

GNSSMessage GNSSReceiver::queryNavigationDataMessageInterval() {
    return GNSSMessage{GNSSMessagesSubID::QueryNavigationDataMessageInterval, 0, {}};
}

GNSSMessage GNSSReceiver::queryVersionExtentionString() {
    return GNSSMessage{GNSSMessagesSubID::QueryVersionExtensionString, 0, {}};
}

GNSSMessage GNSSReceiver::queryGNSSGEOFencingDatabyPolygon() {
    return GNSSMessage{GNSSMessagesSubID::QueryGNSSGeoFencingDataByPolygon, 0, {}};
}

GNSSMessage GNSSReceiver::queryGNSSMultiPolygonGEOFencingResult() {
    return GNSSMessage{GNSSMessagesSubID::QueryGNSSMultiPolygonGeoFencingResult, 0, {}};
}

GNSSMessage GNSSReceiver::queryNMEAStringInterval() {
    return GNSSMessage{GNSSMessagesSubID::QueryNMEAStringInterval, 0, {}};
}

GNSSMessage GNSSReceiver::queryRequestedNMEAStringInterval() {
    return GNSSMessage{GNSSMessagesSubID::QueryRequestedNMEAStringInterval, 0, {}};
}

GNSSMessage GNSSReceiver::getGPSEphemeris(uint8_t SV) {
    Payload payload;
    payload.push_back(SV);
    return GNSSMessage{GNSSMessages::GetGPSEphemeris, static_cast<uint16_t>(payload.size()), payload};
}

GNSSMessage GNSSReceiver::getGLONASSEphemeris(uint8_t SV) {
    Payload payload;
    payload.push_back(SV);
    return GNSSMessage{GNSSMessages::GetGLONASSEphemeris, static_cast<uint16_t>(payload.size()), payload};
}


GNSSMessage GNSSReceiver::getGLONASSTimeCorrectionParameters() {
    return GNSSMessage{GNSSMessages::GetGLONASSTimeCorrectionParameters, 0, {}};
}
