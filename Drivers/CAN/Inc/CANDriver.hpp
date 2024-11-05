#pragma once
#include <cstdint>
#include "stm32h7xx_hal_fdcan.h"

#include "Frame.hpp"
#include "etl/vector.h"

const uint8_t CANMessageSize = 64;

namespace CAN {
    /**
     * A basic driver for the CAN Peripherals. This driver includes encoding/decoding
     * functions for the Data Length Code as well as the ID when used in normal addressing mode. There are two interrupt
     * callback functions, one for registering a transmission plus logging an error if needed, and one for passing the
     * incoming CAN::Frames to the higher level services in the application. This driver handles both peripherals,
     * while keeping only one active through a parameter for transmission, and disabling interrupts for receipt.
     *
     * @note CAN Normal Addressing requires the NodeID to be at most 11 bits long.
     *
     * @note There are two peripherals for the CAN Bus. The main bus is considered as MCAN1, since that is the one
     * available on the development board.
     *
     * @example @code
     * uint32_t id = 0x4; // Specify the sending Node ID.
     * etl::vector<uint8_t, 8> data = {0,1,2,3,4,5,6,7}; // Specify an array of data, up to 64 bytes.
     * CAN::Frame message = {id, data}; // Create a CAN::Frame object.
     * CAN::Driver::send(message, Application::Main); // Use the included send function to send a message on the Main bus.
     * @endcode
     */

    typedef etl::vector<uint8_t, CANMessageSize> CANBuffer_t;

    /**
     * Buffer that stores a message that is ready to be sent
     */

    inline FDCAN_TxHeaderTypeDef txHeader;
    inline CANBuffer_t txFifo;

    /**
     * Buffer that stores a received message that is to be processed.
     * In this setup, rxFifo0 is used to store TP Messages that usually span across multiple CAN::Frames
     */

    inline FDCAN_RxHeaderTypeDef rxHeader0;
    inline CANBuffer_t rxFifo0;

    /**
     * Buffer that stores a received message that is to be processed.
     * In this setup, rxFifo1 is used to store normal messages that are contained in a single CAN::Frame
     */

    inline FDCAN_RxHeaderTypeDef rxHeader1;
    inline CANBuffer_t rxFifo1;


    /**
     * Configures the CAN filter parameters
     */
    void configCANFilter(uint32_t rx_fifo);

    /**
     * Possible states for the peripheral, to be used as arguments in the callback functions.
     */
    enum AppStates : uint8_t {
        Receive,
        Transmit,
    };

    /**
     * The available buses to use.
     */
    enum ActiveBus : uint8_t {
        Redundant = 0x0,
        Main = 0x1
    };

    /**
     * Sets the appropriate interrupt handlers, configures the storage area for incoming/outgoing buffers and
     * keeps only a single peripheral's interrupts active.
     */
    void initialize(uint8_t fifo_select);


    /**
     * Logs messages that are in the Rx buffer
     * @param rxBuf The RX Buffer that the messages are stored
     */
    void logMessage(const CAN::CANBuffer_t& rxBuf, FDCAN_RxHeaderTypeDef RxHeader, ActiveBus incomingBus);

    void logMessage(const CAN::Frame frame);

    /**
     * Decodes the data length code to get the largest expected size of the message.
     *
     * @param dlc The data length code
     * @return The length of the message in bytes
     */
    uint8_t convertDlcToLength(uint32_t dlc);

    /**
     * Encodes the length of the message body to the smallest data length code that it fits in.
     *
     * @param length The length of the message in bytes
     * @return The data length code
     */
    void convertLengthToDLC(uint8_t length);

    /**
     * Transforms a frame ID to conform to the CAN Standard.
     * The implementation shifts non-extended IDs by 18 bits to ease debugging with example projects.
     * @param id The ID as specified in the standard.
     * @return The ID that's sent over the network.
     *
     * @note All IDs should be at most 11 bits long as the non-extended ID scheme is used. The function gets/returns
     * a 32-bit ID to accomodate the shift.
     */
    inline uint32_t writeId(uint32_t id) {
        return id << 18;
    }

    /**
     * Transforms the ID from the CAN Standard to match a frame ID.
     * @param id The ID that was received from the network.
     * @return The ID as specified in the standard.
     *
     * @note All IDs should be at most 11 bits long as the non-extended ID scheme is used. The function gets/returns
     * a 32-bit ID to accomodate the shift.
     */
    inline uint32_t readId(uint32_t id) {
        return id >> 18;
    }

    /**
     * Immediately ends a CAN Message
     * @param message The message to be sent.
     */
    void send(const CAN::Frame& message, const CAN::ActiveBus activeBus);

    void configureTxHeader();

    /**
     * Converts an CANBuffer_t & FDCAN_Header objects into a CAN::Frame.
     * @param rxBuffer The incoming buffer.
     * @param RxHeader The incoming header.
     * @return A CAN::Frame.
     */
    CAN::Frame getFrame(const CAN::CANBuffer_t* data, uint32_t id);

} // namespace CAN
