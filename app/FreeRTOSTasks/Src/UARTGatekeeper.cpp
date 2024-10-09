#include "UARTGatekeeperTask.hpp"
#include "stm32h7xx_hal.h"

extern UART_HandleTypeDef huart4;

UARTGatekeeperTask::UARTGatekeeperTask() : Task("UARTGatekeeperTask") {
    xUartQueue = xQueueCreateStatic(UARTQueueSize, sizeof(etl::string<LOGGER_MAX_MESSAGE_SIZE>), this->ucQueueStorageArea, &(this->xStaticQueue));
}

void UARTGatekeeperTask::execute() {
    etl::string<LOGGER_MAX_MESSAGE_SIZE> output;
    while (true) {
        xQueueReceive(this->xUartQueue, &output, portMAX_DELAY);
        HAL_UART_Transmit(&huart4, reinterpret_cast<const uint8_t *>(output.data()), output.size(), 500);
    }
}
