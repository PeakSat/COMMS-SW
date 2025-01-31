#include "CANDriver.hpp"
#include "Logger.hpp"
#include "main.h"
#include "ECSS_Definitions.hpp"
#include "FreeRTOS.h"
#include <semphr.h>
#include <TPProtocol.hpp>

using namespace CAN;

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;


void CAN::configCANFilter(uint32_t rx_fifo) {
    FDCAN_FilterTypeDef sFilterConfig1;

    sFilterConfig1.IdType = FDCAN_EXTENDED_ID;      // Standard or extended id
    sFilterConfig1.FilterIndex = 0;                 // In case of configuring multiple filters adapt accordingly
    sFilterConfig1.FilterType = FDCAN_FILTER_RANGE; // Filter type
    if (rx_fifo == 0) {
        sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    } else {
        sFilterConfig1.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;
    }
    sFilterConfig1.FilterID1 = 0x380;
    sFilterConfig1.FilterID2 = 0x3FF;
    sFilterConfig1.RxBufferIndex = 0;
    if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig1) != HAL_OK) {
        /* Filter configuration Error */
        Error_Handler();
    }

    FDCAN_FilterTypeDef sFilterConfig2;

    sFilterConfig2.IdType = FDCAN_EXTENDED_ID;      // Standard or extended id
    sFilterConfig2.FilterIndex = 0;                 // In case of configuring multiple filters adapt accordingly
    sFilterConfig2.FilterType = FDCAN_FILTER_RANGE; // Filter type
    if (rx_fifo == 0) {
        sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    } else {
        sFilterConfig2.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;
    }
    sFilterConfig2.FilterID1 = 0x380;
    sFilterConfig2.FilterID2 = 0x3FF;
    sFilterConfig2.RxBufferIndex = 0;

    if (HAL_FDCAN_ConfigFilter(&hfdcan2, &sFilterConfig2) != HAL_OK) {
        /* Filter configuration Error */
        Error_Handler();
    }
}

void CAN::initialize(uint8_t fifo_select) {

    CAN_TRANSMIT_Handler.CAN_TRANSMIT_SEMAPHORE = xSemaphoreCreateMutex();
    configCANFilter(fifo_select);
    configureTxHeader();

    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_FDCAN_Start(&hfdcan2) != HAL_OK) {
        Error_Handler();
    }

    if (fifo_select == 0) {
        // Activate the notification for new data in FIFO0 for FDCAN1
        if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
            Error_Handler();
        }
        // Activate the notification for new data in FIFO1 for FDCAN2
        if (HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
            Error_Handler();
        }
    } else {
        // Activate the notification for new data in FIFO0 for FDCAN1
        if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0) != HAL_OK) {
            Error_Handler();
        }
        // Activate the notification for new data in FIFO1 for FDCAN2
        if (HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0) != HAL_OK) {
            Error_Handler();
        }
    }
}

void CAN::logMessage(const CAN::CANBuffer_t& rxBuf, FDCAN_RxHeaderTypeDef RxHeader, CAN::ActiveBus incomingBus) {
    auto message = String<ECSSMaxStringSize>("CAN Message: ");
    if (incomingBus == Main) {
        message.append("FDCAN1 ");
    } else {
        message.append("FDCAN2 ");
    }
    uint32_t id = readId(RxHeader.Identifier);
    const uint8_t msgLength = convertDlcToLength(RxHeader.DataLength);
    message.append("ID : ");
    etl::to_string(id, message, etl::format_spec().hex(), true);
    message.append(" Length : ");
    etl::to_string(msgLength, message, true);
    message.append(" Data : ");
    for (uint8_t idx = 0; idx < msgLength; idx++) {
        etl::to_string(*(rxBuf.data() + idx), message, true);
        message.append(" ");
    }
    LOG_INFO << message.c_str();
}

void CAN::logMessage(const CAN::Packet frame) {
    auto message = String<ECSSMaxStringSize>("CAN Message: ");
    message.append("ID : ");
    etl::to_string(frame.id, message, etl::format_spec().hex(), true);
    message.append(" Data : ");
    for (uint8_t idx = 0; idx < CAN::MaxPayloadLength; idx++) {
        etl::to_string(*(frame.data.data() + idx), message, true);
        message.append(" ");
    }
    LOG_INFO << message.c_str();
}

uint8_t CAN::convertDlcToLength(uint32_t dlc) {
    dlc >>= 16;
    static constexpr uint8_t msgLength[] = {0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 12U, 16U, 20U, 24U, 32U, 48U, 64U};
    return msgLength[dlc];
}

void CAN::convertLengthToDLC(uint8_t length) {
    uint32_t under8bits[] = {FDCAN_DLC_BYTES_0, FDCAN_DLC_BYTES_1, FDCAN_DLC_BYTES_2, FDCAN_DLC_BYTES_3,
                             FDCAN_DLC_BYTES_4,
                             FDCAN_DLC_BYTES_5, FDCAN_DLC_BYTES_6, FDCAN_DLC_BYTES_7, FDCAN_DLC_BYTES_8};
    if (length <= 8U) {
        CAN::txHeader.DataLength = under8bits[length];
    } else if (length <= 12U) {
        CAN::txHeader.DataLength = FDCAN_DLC_BYTES_12;
    } else if (length <= 16U) {
        CAN::txHeader.DataLength = FDCAN_DLC_BYTES_16;
    } else if (length <= 20U) {
        CAN::txHeader.DataLength = FDCAN_DLC_BYTES_20;
    } else if (length <= 24U) {
        CAN::txHeader.DataLength = FDCAN_DLC_BYTES_24;
    } else if (length <= 32U) {
        CAN::txHeader.DataLength = FDCAN_DLC_BYTES_32;
    } else if (length <= 48U) {
        CAN::txHeader.DataLength = FDCAN_DLC_BYTES_48;
    } else {
        CAN::txHeader.DataLength = FDCAN_DLC_BYTES_64;
    }
}


void CAN::send(const CAN::Packet& message, CAN::ActiveBus outgoingBus) {
    CAN::txHeader.Identifier = message.id;
    CAN::txHeader.DataLength = 8;

    memcpy(txFifo.data(), message.data.data(), CAN::MaxPayloadLength);
    if (outgoingBus == Main) {
        if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &CAN::txHeader, txFifo.data()) != HAL_OK) {
            LOG_ERROR << "CAN 1 Queue Full!";
        }
    } else {
        if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &CAN::txHeader, txFifo.data()) != HAL_OK) {
            LOG_ERROR << "CAN 2 Queue Full!";
        }
    }
}

void CAN::configureTxHeader() {
    CAN::txHeader.IdType = FDCAN_STANDARD_ID;
    CAN::txHeader.TxFrameType = FDCAN_DATA_FRAME;
    CAN::txHeader.DataLength = FDCAN_DLC_BYTES_64;
    CAN::txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    CAN::txHeader.BitRateSwitch = FDCAN_BRS_OFF;
    CAN::txHeader.FDFormat = FDCAN_FD_CAN;
    CAN::txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    CAN::txHeader.MessageMarker = 0;
}


Packet CAN::getFrame(const CAN::CANBuffer_t* data, uint32_t id) {


    CAN::Packet frame = Packet();

    frame.id = id;

    for (uint8_t idx = 0; idx < CANMessageSize; idx++) {
        frame.data.insert_at(idx, *(data->data() + idx));
    }

    return frame;
}