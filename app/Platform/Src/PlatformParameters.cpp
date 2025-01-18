#include "COMMS_ECSS_Configuration.hpp"


#ifdef SERVICE_PARAMETER


#include "Services/ParameterService.hpp"

#include "PlatformParameters.hpp"


void ParameterService::initializeParameterMap() {

    parameters = {

        {PAYParameters::xID, PAYParameters::x},
        {PAYParameters::yID, PAYParameters::y},
        {PAYParameters::zID, PAYParameters::z},
        {PAYParameters::uptimeID, PAYParameters::uptime},
        {PAYParameters::timeID, PAYParameters::time},
        {PAYParameters::psu_12vID, PAYParameters::psu_12v},
        {PAYParameters::psu_5vID, PAYParameters::psu_5v},
        {PAYParameters::psu_33vID, PAYParameters::psu_33v},
        {PAYParameters::mcu_die_temperatureID, PAYParameters::mcu_die_temperature},
        {PAYParameters::main_board_temperatureID, PAYParameters::main_board_temperature},
        {PAYParameters::seed_ld_output_powerID, PAYParameters::seed_ld_output_power},
        {PAYParameters::fsm_chamber_temperatureID, PAYParameters::fsm_chamber_temperature},
        {PAYParameters::fsm_chamber_pressureID, PAYParameters::fsm_chamber_pressure},
        {PAYParameters::fsmd_voltage_converter_temperatureID, PAYParameters::fsmd_voltage_converter_temperature},
        {PAYParameters::fsmd_drivers_temperatureID, PAYParameters::fsmd_drivers_temperature},
        {PAYParameters::camera_pcb_temperatureID, PAYParameters::camera_pcb_temperature},
        {PAYParameters::camera_sensor_temperatureID, PAYParameters::camera_sensor_temperature},
        {PAYParameters::fso_aux_temperature_1ID, PAYParameters::fso_aux_temperature_1},
        {PAYParameters::fso_aux_temperature_2ID, PAYParameters::fso_aux_temperature_2},
        {PAYParameters::oad_temperatureID, PAYParameters::oad_temperature},
        {PAYParameters::optical_amplifier_combiner_heater_temperatureID, PAYParameters::optical_amplifier_combiner_heater_temperature},
        {PAYParameters::optical_amplifier_fiber_mirror_temperatureID, PAYParameters::optical_amplifier_fiber_mirror_temperature},
        {PAYParameters::optical_amplifier_circulator_heater_temperatureID, PAYParameters::optical_amplifier_circulator_heater_temperature},
        {PAYParameters::ldd_temperatureID, PAYParameters::ldd_temperature},
        {PAYParameters::ld_temperatureID, PAYParameters::ld_temperature},
        {PAYParameters::ldd_12v_currentID, PAYParameters::ldd_12v_current},
        {PAYParameters::ldd_psu_12vID, PAYParameters::ldd_psu_12v},
        {PAYParameters::ldd_converter_input_currentID, PAYParameters::ldd_converter_input_current},
        {PAYParameters::ldd_ld_vID, PAYParameters::ldd_ld_v},
        {PAYParameters::ldd_ld_dac_set_vID, PAYParameters::ldd_ld_dac_set_v},
        {PAYParameters::fsm_driver_12v_currentID, PAYParameters::fsm_driver_12v_current},
        {PAYParameters::flashes_33v_currentID, PAYParameters::flashes_33v_current},
        {PAYParameters::fpga_5v_currentID, PAYParameters::fpga_5v_current},
        {PAYParameters::sdd_33v_currentID, PAYParameters::sdd_33v_current},
        {PAYParameters::pump_ld_powerID, PAYParameters::pump_ld_power},
        {PAYParameters::mcu_33v_currentID, PAYParameters::mcu_33v_current},
        {PAYParameters::fpga_die_temperatureID, PAYParameters::fpga_die_temperature},
        {PAYParameters::fpga_vdd1_voltageID, PAYParameters::fpga_vdd1_voltage},
        {PAYParameters::fpga_vdd18_voltageID, PAYParameters::fpga_vdd18_voltage},
        {PAYParameters::fpga_vdd25_voltageID, PAYParameters::fpga_vdd25_voltage},
        {PAYParameters::seed_ld_incoming_powerID, PAYParameters::seed_ld_incoming_power},
        {PAYParameters::amplifier_output_powerID, PAYParameters::amplifier_output_power},
        {PAYParameters::amplifier_output_reflected_powerID, PAYParameters::amplifier_output_reflected_power},
        {PAYParameters::seed_diode_bias_currentID, PAYParameters::seed_diode_bias_current},
        {PAYParameters::tec_currentID, PAYParameters::tec_current},
        {PAYParameters::ldd_output_currentID, PAYParameters::ldd_output_current},
        {PAYParameters::sd_temperature_violationsID, PAYParameters::sd_temperature_violations},
        {PAYParameters::end_uptimeID, PAYParameters::end_uptime},
        {PAYParameters::responseID, PAYParameters::response},
        {PAYParameters::deviceID, PAYParameters::device},
        {PAYParameters::firmwareID, PAYParameters::firmware},
        {PAYParameters::firmware_is_confirmedID, PAYParameters::firmware_is_confirmed},
        {PAYParameters::bitstreamID, PAYParameters::bitstream},
        {PAYParameters::softcpu_1_firmwareID, PAYParameters::softcpu_1_firmware},
        {PAYParameters::softcpu_2_firmwareID, PAYParameters::softcpu_2_firmware},
        {PAYParameters::softcpu_3_firmwareID, PAYParameters::softcpu_3_firmware},
        {PAYParameters::softcpu_4_firmwareID, PAYParameters::softcpu_4_firmware},
        {PAYParameters::softcpu_5_firmwareID, PAYParameters::softcpu_5_firmware},
        {PAYParameters::softcpu_6_firmwareID, PAYParameters::softcpu_6_firmware},
        {PAYParameters::softcpu_7_firmwareID, PAYParameters::softcpu_7_firmware},
        {PAYParameters::softcpu_8_firmwareID, PAYParameters::softcpu_8_firmware},
        {PAYParameters::boot_countID, PAYParameters::boot_count},
        {PAYParameters::transmission_countID, PAYParameters::transmission_count},
        {PAYParameters::hw_detID, PAYParameters::hw_det},
        {PAYParameters::storage0ID, PAYParameters::storage0},
        {PAYParameters::storage1ID, PAYParameters::storage1},
        {PAYParameters::storage2ID, PAYParameters::storage2},
        {PAYParameters::storage3ID, PAYParameters::storage3},
        {PAYParameters::ldd_faultID, PAYParameters::ldd_fault},
        {PAYParameters::fsm_faultID, PAYParameters::fsm_fault},
        {PAYParameters::fpga_faultID, PAYParameters::fpga_fault},
        {PAYParameters::v_cam_faultID, PAYParameters::v_cam_fault},
        {PAYParameters::sdd_faultID, PAYParameters::sdd_fault},
        {OBDHParameters::CAN_ACK_timeoutID, OBDHParameters::CAN_ACK_timeout},
        {COMMSParameters::commsUHFBandPATemperatureID, COMMSParameters::commsUHFBandPATemperature},
        {COMMSParameters::commsPCBTemperatureID, COMMSParameters::commsPCBTemperature},
        {COMMSParameters::commsGNSSTemperatureID, COMMSParameters::commsGNSSTemperature},
        {COMMSParameters::Antenna_Deployment_StatusID, COMMSParameters::Antenna_Deployment_Status}

    };
}


#endif
