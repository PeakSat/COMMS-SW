#include "FunctionManagementWrappers.hpp"

#include <GNSS.hpp>

void GNSS_Data_Download_Wrapper(Message& msg) {

    uint16_t functionID = (msg.data[0] << 8) | msg.data[1];
    //arguments
    uint32_t period = 0;
    uint32_t secondsPrior = 0;
    uint32_t numberOfSamples = 0;

    if (msg.dataSize < sizeof(functionID) + sizeof(period) + sizeof(secondsPrior) + sizeof(numberOfSamples)) {
        LOG_ERROR << "Function Management: wrong message size";
        // todo: handle the error
        return;
    }

    int dataPointer = sizeof(functionID);
    for (int i = 0; i < sizeof(period); i++) {
        period = period << 8;
        period = period & msg.data[dataPointer + i];
    }
    dataPointer += sizeof(period);

    for (int i = 0; i < sizeof(secondsPrior); i++) {
        period = period << 8;
        period = period & msg.data[dataPointer + i];
    }
    dataPointer += sizeof(secondsPrior);

    for (int i = 0; i < sizeof(numberOfSamples); i++) {
        period = period << 8;
        period = period & msg.data[dataPointer + i];
    }
    dataPointer += sizeof(numberOfSamples);


    GNSSReceiver::sendGNSSData(period, secondsPrior, numberOfSamples);
}