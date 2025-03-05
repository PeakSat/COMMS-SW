#pragma once

#include <etl/string.h>
#include "etl/vector.h"
#include "GNSSMessage.hpp"
#include "GNSSDefinitions.hpp"
#include <ECSS_Definitions.hpp>

using namespace GNSSDefinitions;
inline uint8_t GNSS_TMbuffer[ECSSMaxMessageSize]; // todo: is this the max TM size?

class GNSSReceiver {
public:
    GNSSReceiver();

    static void storeDataToEMMC(uint8_t* data, uint32_t block);
    static void getDataFromEMMC(uint8_t* data, uint32_t block);

    static bool isDataValid(int8_t year, int8_t month, int8_t day);

    static uint32_t findTailPointer();
    static void sendGNSSData(uint32_t period, uint32_t secondsPrior, uint32_t numberOfSamples);
    static void constructGNSSTM(GNSSDefinitions::StoredGNSSData* storedData1, GNSSDefinitions::StoredGNSSData* storedData2, uint32_t numberOfData);

    // Tested Functions

    static GNSSMessage configureMessageType(ConfigurationType type, Attributes attributes);

    static GNSSMessage configureSystemPowerMode(PowerMode mode, Attributes attributes);

    static GNSSMessage configureSystemPositionRate(PositionRate rate, Attributes attributes);

    static GNSSMessage configureNMEATalkerID(TalkerIDType type, Attributes attributes);

    static GNSSMessage configureGNSSNavigationMode(NavigationMode mode, Attributes attributes);

    static GNSSMessage configureNMEAStringInterval(etl::string<3>, uint8_t interval, Attributes attributes);

    static GNSSMessage querySoftwareVersion(SoftwareType softwareType);

    static GNSSMessage query1PPSTiming();

    static GNSSMessage queryInterferenceDetectionStatus();

    static GNSSMessage setFactoryDefaults(DefaultType type);

    // untested

    static GNSSMessage configureDOPMask(DOPModeSelect mode, uint16_t PDOPvalue, uint16_t HDOPvalue, uint16_t GDOPvalue,
                                        Attributes attributes);

    static GNSSMessage configureElevationAndCNRMask(ElevationAndCNRModeSelect mode, uint8_t elevationMask, uint8_t CNRMask,
                                                    Attributes attributes);

    static GNSSMessage configurePositionPinning(PositionPinning positionPinning, Attributes attributes);

    static GNSSMessage configurePositionPinningParameters(uint16_t pinningSpeed, uint16_t pinningCnt, uint16_t unpiningSpeed,
                                                          uint16_t unpiningCnt, uint16_t unpinningDistance,
                                                          Attributes attributes);

    static GNSSMessage configure1PPSCableDelay(uint32_t cableDelay, Attributes attributes);

    static GNSSMessage
    configure1PPSTiming(TimingMode mode, uint16_t surveyLength, uint16_t standardDeviation, uint16_t latitude,
                        uint16_t longtitude, uint16_t altitude, Attributes attributes);

    static GNSSMessage configure1PPSOutputMode(OutputMode mode, AlignSource source, Attributes attributes);

    // Messages with Sub ID
    static GNSSMessage configureSBAS(EnableSBAS enable, RangingSBAS ranging, uint8_t URAMask, CorrectionSBAS correction,
                                     uint8_t numberOfTrackingChannels, uint8_t subsystemMask, Attributes attributes);

    static GNSSMessage configureQZSS(EnableQZSS enable, uint8_t noOfTrackingChannels, Attributes attributes);

    static GNSSMessage
    configureSBASAdvanced(EnableSBAS enable, RangingSBAS ranging, uint8_t rangingURAMask, CorrectionSBAS correction,
                          uint8_t numTrackingChannels,
                          uint8_t subsystemMask, uint8_t waasPRN, uint8_t egnosPRN, uint8_t msasPRN, uint8_t gaganPRN,
                          uint8_t sdcmPRN, uint8_t bdsbasPRN, uint8_t southpanPRN, uint8_t attributes);

    static GNSSMessage configureSAEE(EnableSAEE enable, Attributes attributes);

    static GNSSMessage configureExtendedNMEAMessageInterval(const etl::vector<uint8_t, 12>& intervals, Attributes attributes);

    static GNSSMessage configureInterferenceDetection(InterferenceDetectControl control, Attributes attributes);

    static GNSSMessage
    configurePositionFixNavigationMap(FirstFixNavigationMask firstFix, SubsequentFixNavigationMask subsequentFix,
                                      Attributes attributes);

    static GNSSMessage configureUTCReferenceTimeSyncToGPSTime(EnableSyncToGPSTime enable, uint16_t utcYear, uint8_t utcMonth,
                                                              uint8_t utcDay, Attributes attributes);

    static GNSSMessage configureGNSSConstellationTypeForNavigationSolution(uint8_t constellationType, Attributes attributes);

    static GNSSMessage SoftwareImageDownloadUsingROMExternalLoader(BaudRate baud, FlashType flashType, uint16_t flashID,
                                                                   BufferUsedIndex bufferUsedIndex,
                                                                   Attributes attributes);
    static GNSSMessage configureGNSSDozeMode();

    static GNSSMessage configurePSTIMessageInterval(uint8_t PSTI_ID, uint8_t interval, Attributes attributes);

    static GNSSMessage configureGNSSDatumIndex(uint16_t datumIndex, Attributes attributes);

    static GNSSMessage
    configureGPSUTCLeapSecondsInUTC(uint16_t utcYear, uint8_t utcMonth, uint8_t leapSeconds, uint8_t insertSeconds,
                                    Attributes attributes);

    static GNSSMessage configureNavigationDataMessageInterval(uint8_t navigationDataMessageInterval, Attributes attributes);

    static GNSSMessage configureGNSSGEOFencingDatabyPolygon();

    static GNSSMessage SoftwareImageDownloadUsingInternalLoader(BaudRate baud, FlashType flashType, uint16_t flashID,
                                                                BufferUsedIndex bufferUsedIndex,
                                                                Attributes attributes);

    static GNSSMessage SoftwareImageDownloadUsingExternalLoader(BaudRate baud, FlashType flashType, uint16_t flashID,
                                                                BufferUsedIndex bufferUsedIndex,
                                                                Attributes attributes);
    // Query

    static GNSSMessage querySoftwareCRC(SoftwareType softwareType);

    static GNSSMessage queryPositionUpdateRate();

    static GNSSMessage queryPowerMode();

    static GNSSMessage QueryDOPMask();

    static GNSSMessage QueryElevationandCNRMask();

    static GNSSMessage queryPositionPinning();

    static GNSSMessage query1PPSCableDelay();

    static GNSSMessage queryNMEATalkerID();

    static GNSSMessage querySBASStatus();

    static GNSSMessage queryQZSSStatus();

    // Query with Sub ID

    static GNSSMessage querySBASAdvanced();

    static GNSSMessage querySAEEStatus();

    static GNSSMessage queryGNSSBootStatus();

    static GNSSMessage queryExtendedNMEAMesaageInterval();

    static GNSSMessage queryGNSSParameterSearchEngineNumber();

    static GNSSMessage queryPositionFixNavigationMap();

    static GNSSMessage QueryUTCReferenceTimeSyncToGPSTime();

    static GNSSMessage queryGNSSNavigationMode();

    static GNSSMessage queryGNSSConstellationTypeForNavigationSolution();

    static GNSSMessage queryGPSTime();

    static GNSSMessage queryPSTIMessageInterval();

    static GNSSMessage queryRequestedPSTIMessageInterval();

    static GNSSMessage queryNavigationDataMessageInterval();

    static GNSSMessage queryVersionExtentionString();

    static GNSSMessage queryGNSSGEOFencingDatabyPolygon();

    static GNSSMessage queryGNSSMultiPolygonGEOFencingResult();

    static GNSSMessage queryNMEAStringInterval();

    static GNSSMessage queryRequestedNMEAStringInterval();

    // Sub ID

    static GNSSMessage getGPSEphemeris(uint8_t SV);

    static GNSSMessage getGLONASSEphemeris(uint8_t SV);

    static GNSSMessage getGLONASSTimeCorrectionParameters();
};