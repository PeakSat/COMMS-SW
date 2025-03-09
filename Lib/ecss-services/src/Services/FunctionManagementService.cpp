#include "COMMS_ECSS_Configuration.hpp"
#include "FunctionManagementWrappers.hpp"
#ifdef SERVICE_FUNCTION

#include "FunctionManagementService.hpp"

/**
 * The number(N) and size of the arguments cannot be checked at this point.
 * Every function should do its own bound checks.
 *
 *            i= 0 1 2 3 4...             ...x
 *  msg.data[i]: B B B B B B B B B B B B B B B
 *             _/   / \                      |
 *         __/ 2B  | 1B\        (x-3)B       |
 *      / function | N |       Arguments     |
 *     |    ID     |   |                     |
 */

void FunctionManagementService::call(Message& msg) {
	uint16_t functionID = (msg.data[0] << 8) | msg.data[1];

	switch (functionID) {
		case 2: // GNSS data download
			GNSS_Data_Download_Wrapper(msg);
			break;
	}

}

void FunctionManagementService::execute(Message& message) {
	switch (message.messageType) {
		case PerformFunction:
			call(message); // TC[8,1]
			break;
		default:
			ErrorHandler::reportInternalError(ErrorHandler::OtherMessageType);
			break;
	}
}

#endif
