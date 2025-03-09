#pragma once


#include <cstdint>
#include "etl/String.hpp"


namespace LogSource {
    /**
    * The maximum number of letters in a string representation of a subsystem's name.
    */
    inline constexpr uint8_t MaximumLettersInSubsystemName = 8;

    /**
     * The maximum number of Logging Sources
     */
    inline constexpr uint8_t NumberOfLogSources = 5;

    /**
     * The subsystem to be used, if no other source is defined using the stream operator.
     */
    extern String<MaximumLettersInSubsystemName> currentSubsystem;
} // namespace LogSource

namespace CAN {
    /**
     * CAN Nodes, as specified in DDJF_OBDH.
     */
    enum NodeIDs : uint8_t {
        OBC = 0x0,
        TTC = 0x1,
        ADCS = 0x2,
        SU = 0x3,
        ExternalMCU = 0x4
    };
} // namespace CAN