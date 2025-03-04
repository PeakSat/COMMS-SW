#include "Logger.hpp"
#include "TestTask.hpp"
#include "eMMC.hpp"
#include "PlatformParameters.hpp"
#include "ParameterService.hpp"
#include "ApplicationLayer.hpp"
#include "COBS.hpp"

#include <GNSS.hpp>
#include <GNSSDefinitions.hpp>
#include <HousekeepingService.hpp>
#include <RF_TXTask.hpp>
#include <ServicePool.hpp>

void TestTask::execute() {
    LOG_DEBUG << "TestTask::execute()";
    vTaskDelay(5000);
    GNSSDefinitions::StoredGNSSData data{};

    // for (int i=0; i<2600; i++) {
    //     auto status = eMMC::storeItem(eMMC::memoryMap[eMMC::GNSSData], reinterpret_cast<uint8_t*>(&data), EMMC_PAGE_SIZE, i, 1);
    // }

    while (true) {


        // if (eMMCDataTailPointer > 0) {
        //     auto status = eMMC::getItem(eMMC::memoryMap[eMMC::GNSSData], reinterpret_cast<uint8_t*>(&data), 512, eMMCDataTailPointer - 1, 1);
        // }
        uint64_t usFromEpoch_NofSat;
        int32_t latitudeI;
        int32_t longitudeI;
        int32_t altitudeI;
        uint64_t timeFromEpoch;
        uint8_t numberOfSatellites;
        GNSSReceiver::sendGNSSData(1, 10, 5);

        uint16_t numberOfData = ((uint16_t) GNSS_TMbuffer[0]) << 8 | static_cast<uint16_t>(GNSS_TMbuffer[1]);
        for (int i = 0; i < numberOfData; i++) {
            //get time + num of satellites
            uint8_t* TimeStartBytesStoredDataPointer1 = reinterpret_cast<uint8_t*>(&usFromEpoch_NofSat);
            uint8_t* dataPointer = (uint8_t*) &GNSS_TMbuffer[5 + (5 * i)];
            usFromEpoch_NofSat = 0;
            usFromEpoch_NofSat |= static_cast<uint64_t>(GNSS_TMbuffer[2]);
            usFromEpoch_NofSat = usFromEpoch_NofSat << 8;
            usFromEpoch_NofSat |= static_cast<uint64_t>(GNSS_TMbuffer[3]);
            usFromEpoch_NofSat = usFromEpoch_NofSat << 8;
            usFromEpoch_NofSat |= static_cast<uint64_t>(GNSS_TMbuffer[4]);
            usFromEpoch_NofSat = usFromEpoch_NofSat << 8;
            usFromEpoch_NofSat = usFromEpoch_NofSat << 32;
            for (int j = 0; j < sizeof(uint64_t) - 3; j++) {
                TimeStartBytesStoredDataPointer1[j] = dataPointer[j];
                // usFromEpoch_NofSat=usFromEpoch_NofSat<<8;
                // usFromEpoch_NofSat |= dataPointer[j];
            }
            timeFromEpoch = usFromEpoch_NofSat >> 5;
            numberOfSatellites = (uint8_t) (usFromEpoch_NofSat & 0x1F);

            //get lat
            uint8_t* LatStartBytesStoredDataPointer1 = reinterpret_cast<uint8_t*>(&latitudeI);
            dataPointer = (uint8_t*) &GNSS_TMbuffer[5 + (5 * numberOfData)];
            latitudeI = 0;
            for (int j = 0; j < sizeof(int32_t); j++) {
                LatStartBytesStoredDataPointer1[j] = dataPointer[j + (4 * i)];
                // latitudeI=latitudeI<<8;
                // latitudeI |= dataPointer[j];
            }

            //get lon
            uint8_t* LonStartBytesStoredDataPointer1 = reinterpret_cast<uint8_t*>(&longitudeI);
            dataPointer = (uint8_t*) &GNSS_TMbuffer[(numberOfData * (5 + 4)) + 5];
            longitudeI = 0;
            for (int j = 0; j < sizeof(int32_t); j++) {
                LonStartBytesStoredDataPointer1[j] = dataPointer[j + (4 * i)];
                // longitudeI=longitudeI<<8;
                // longitudeI |= dataPointer[j];
            }

            //get alt
            uint8_t* AltStartBytesStoredDataPointer1 = reinterpret_cast<uint8_t*>(&altitudeI);
            dataPointer = (uint8_t*) &GNSS_TMbuffer[(numberOfData * (5 + 4 + 4)) + 5];
            altitudeI = 0;
            for (int j = 0; j < sizeof(int32_t); j++) {
                AltStartBytesStoredDataPointer1[j] = dataPointer[j + (4 * i)];
                // altitudeI=altitudeI<<8;
                // altitudeI |= dataPointer[j];
            }
            __NOP();
        }

        // GNSS_TMbuffer[0];

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
