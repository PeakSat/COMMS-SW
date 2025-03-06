#include "UARTGatekeeperTask.hpp"
#include "stm32h7xx_hal.h"

extern UART_HandleTypeDef huart4;

UARTGatekeeperTask::UARTGatekeeperTask() : Task("UARTGatekeeperTask"), taskStack{} {
    xUartQueue = xQueueCreateStatic(UARTQueueSize, sizeof(etl::string<LOGGER_MAX_MESSAGE_SIZE>), this->ucQueueStorageArea, &(this->xStaticQueue));
}

[[noreturn]] void UARTGatekeeperTask::execute() {
    UART_Gatekeeper_Semaphore = xSemaphoreCreateBinaryStatic(&UART_Gatekeeper_SemaphoreBuffer);
    etl::string<LOGGER_MAX_MESSAGE_SIZE> output;
    xSemaphoreGive(UART_Gatekeeper_Semaphore);
    while (true) {
        xQueueReceive(this->xUartQueue, &output, portMAX_DELAY);
        xSemaphoreTake(UART_Gatekeeper_Semaphore, portMAX_DELAY);
        output.repair();
        uint8_t buffer[LOGGER_MAX_MESSAGE_SIZE];
        for (int i = 0; i < output.length(); i++) {
            buffer[i] = output.data()[i];
        }
        HAL_UART_Transmit_DMA(&huart4, buffer, output.size());
    }
}
