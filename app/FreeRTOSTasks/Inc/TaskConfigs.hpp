#ifndef TASKCONFIGS_HPP
#define TASKCONFIGS_HPP

#pragma once
#include "Task.hpp"

const BaseType_t eMMCTaskPriority = tskIDLE_PRIORITY + 1;
const BaseType_t CANParserTaskPriority = tskIDLE_PRIORITY + 2;
const BaseType_t CANGatekeeperTaskPriority = tskIDLE_PRIORITY + 2;
const BaseType_t GNSSTaskPriority = tskIDLE_PRIORITY + 1;
const BaseType_t HeartbeatTaskPriority = tskIDLE_PRIORITY + 1;
const BaseType_t INA3221TaskPriority = tskIDLE_PRIORITY + 1;
const BaseType_t RF_RXTaskPriority = tskIDLE_PRIORITY + 3;
const BaseType_t RF_TXTaskPriority = tskIDLE_PRIORITY + 1;
const BaseType_t TCHandlingTaskPriority = tskIDLE_PRIORITY + 1;
const BaseType_t TMHandlingTaskPriority = tskIDLE_PRIORITY + 1;
const BaseType_t TestTaskPriority = tskIDLE_PRIORITY + 1;
const BaseType_t TMP117TaskPriority = tskIDLE_PRIORITY + 1;
const BaseType_t UARTGatekeeperTaskPriority = tskIDLE_PRIORITY + 1;

const uint16_t eMMCTaskStack = 4000;
const uint16_t CANParserTaskStack = 7000;
const uint16_t CANGatekeeperTaskStack = 7000;
const uint16_t GNSSTaskStack = 10000;
const uint16_t HeartbeatTaskStack = 5000;
const uint16_t INA3221TaskStack = 2000;
const uint16_t  RF_RXTaskStack = 10000;
const uint16_t  RF_TXTaskStack = 10000;
const uint16_t  TCHandlingTaskStack = 10000;
const uint16_t  TMHandlingTaskStack = 10000;
const uint16_t  TestTaskStack = 5000;
const uint16_t  TMP117TaskStack = 2000;
const uint16_t  UARTGatekeeperTaskStack = 10000;


#endif //TASKCONFIGS_HPP
