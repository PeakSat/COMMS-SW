#pragma once

#include <cstdint>


#pragma once

#include "Definitions.hpp"

namespace CAN {
    /**
     * The ID for the current node as described in DDJF_OBDH
     */
    inline const CAN::NodeIDs NodeID = TTC;

    /**
     * The maximum of the length of the queue for incoming/outgoing CAN frames.
     */
    inline const uint8_t FrameQueueSize = 20;

    /**
     * The maximum size for the data field of a CAN-TP message.
     */
    inline const uint16_t TPMessageMaximumSize = 256;

    /**
     * The maximum numbers of parameters, function arguments etc. inside a single CAN-TP Message.
     */
    inline const uint8_t TPMessageMaximumArguments = 10;

} // namespace CAN

/**
 * Used to control COBS Encoding for Log Messages in the UART Gatekeeper task.
 */
inline const bool LogsAreCOBSEncoded = false;