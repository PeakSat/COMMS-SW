#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* App includes. */
#include "app_main.h"
#include "TransceiverTask.hpp"
#include "UARTGatekeeperTask.hpp"
#include "eMMCTask.hpp"

void app_main(void) {
	transceiverTask.emplace();
	uartGatekeeperTask.emplace();
	eMMCTask.emplace();

	transceiverTask->createTask();
	uartGatekeeperTask->createTask();
	eMMCTask->createTask();

	/* Start the scheduler. */
	vTaskStartScheduler();

	/* Should not get here. */
	for (;;)
		;
}
/*-----------------------------------------------------------*/

extern "C" [[maybe_unused]] void EXTI1_IRQHandler(void) {
	HAL_GPIO_EXTI_IRQHandler(RF_IRQ_Pin);
	transceiverTask->transceiver.handle_irq();
}