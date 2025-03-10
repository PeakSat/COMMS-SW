#pragma once
#include "EventActionService.hpp"
#include "EventReportService.hpp"
#include "HousekeepingService.hpp"
#include "LargePacketTransferService.hpp"
#include "MemoryManagementService.hpp"
#include "ParameterService.hpp"
#include "ParameterStatisticsService.hpp"
#include "RealTimeForwardingControlService.hpp"
#include "RequestVerificationService.hpp"
#include "TestService.hpp"
#include "TimeBasedSchedulingService.hpp"

/**
 * Namespace containing all the report types for every service type.
 * For use in ST[14] and ST[15]
 */
namespace AllReportTypes {
	typedef uint8_t ReportId;
	/**
	 * Vector to contain all report types defined for a service
	 */
	using ServiceDefinitionsVector = etl::vector<ReportId, ECSSMaxReportTypeDefinitions>;
	const extern ServiceDefinitionsVector ST01Reports;
	const extern ServiceDefinitionsVector ST03Reports;
	const extern ServiceDefinitionsVector ST04Reports;
	const extern ServiceDefinitionsVector ST05Reports;
	const extern ServiceDefinitionsVector ST06Reports;
	const extern ServiceDefinitionsVector ST11Reports;
	const extern ServiceDefinitionsVector ST13Reports;
	const extern ServiceDefinitionsVector ST17Reports;
	const extern ServiceDefinitionsVector ST19Reports;
	const extern ServiceDefinitionsVector ST20Reports;

	/**
	 * Map containing all the report types, per service. The key is the ServiceType and the value,
	 * an etl vector containing the report types.
	 */
	extern const etl::map<uint8_t, const ServiceDefinitionsVector&, ECSSMaxServiceTypeDefinitions> MessagesOfService;

} // namespace AllReportTypes