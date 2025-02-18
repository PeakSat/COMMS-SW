#include <COBS.hpp>
#include <Message.hpp>
#include <MessageParser.hpp>
#include <TMHandlingTask.hpp>
#include <at86rf215definitions.hpp>
#include <eMMC.hpp>


[[noreturn]] void TMHandling::execute() {
    LOG_INFO << "TMHandlingTask::execute()";
    while (true) {
        uint32_t received_events;
        CAN::StoredPacket TM_PACKET;
        uint8_t TM_BUFFER[2048];
        while (true) {
            if (xTaskNotifyWaitIndexed(NOTIFY_INDEX_RECEIVED_TM, pdFALSE, pdTRUE, &received_events, portMAX_DELAY) == pdTRUE) {
                LOG_INFO << "parsing of the TM...";
                // TODO parse the TM
                while (uxQueueMessagesWaiting(incomingTMQueue)) {
                    xQueueReceive(incomingTMQueue, &TM_PACKET, portMAX_DELAY);
                    getItem(eMMC::memoryMap[eMMC::RECEIVED_TM], TM_BUFFER, 2048, TM_PACKET.pointerToeMMCItemData, 4);
                    auto cobsDecodedMessage = COBSdecode<1024>(TM_BUFFER, TM_PACKET.size);
                    LOG_DEBUG << "New TM Message received from OBC with length : " << TM_PACKET.size;

                    // // appends the remaining bits to complete a byte0.
                    // Message message = MessageParser::parse(TM_BUFFER, TM_PACKET.size);
                    // message.finalize();
                    // etl::format_spec formatSpec;
                    // auto serviceType = String<1024>("");
                    // auto messageType = String<1024>("");
                    //
                    // etl::to_string(message.serviceType, serviceType, formatSpec, false);
                    // etl::to_string(message.messageType, messageType, formatSpec, false);
                    //
                    // LOG_DEBUG << "New TM Message received from OBC";
                    //
                    // auto output = String<ECSSMaxMessageSize>("New ");
                    // (message.packetType == Message::TM) ? output.append("TM[") : output.append("TC[");
                    // output.append(serviceType);
                    // output.append(",");
                    // output.append(messageType);
                    // output.append("] message! ");

                    // auto data = String<CCSDSMaxMessageSize>("");
                    // String<CCSDSMaxMessageSize> createdPacket = MessageParser::compose(message);
                    // for (unsigned int i = 0; i < createdPacket.size(); i++) {
                    //     etl::to_string(createdPacket[i], data, formatSpec, true);
                    //     data.append(" ");
                    // }
                    // output.append(data.c_str());
                    // LOG_DEBUG << output.c_str();

                }
            }
        }
    }
}