#pragma once


#pragma GCC diagnostic push


#pragma GCC diagnostic ignored "-Wpsabi" // Suppress: parameter passing for argument of type 'Time::DefaultCUC' {aka 'TimeStamp<4, 0, 1, 10>'} changed in GCC 7.1


#include "Helpers/Parameter.hpp"


namespace OBDHParameters {

    enum ParameterID : uint16_t {

        CAN_ACK_timeoutID = 5340

    };

    inline Parameter<uint32_t> CAN_ACK_timeout(0);
} // namespace OBDHParameters


namespace COMMSParameters {

    enum ParameterID : uint16_t {

        commsUHFBandPATemperatureID = 10010,
        commsPCBTemperatureID = 10020,
        commsGNSSTemperatureID = 10030,
        Antenna_Deployment_StatusID = 10050

    };

    inline Parameter<float> commsUHFBandPATemperature(0);
    inline Parameter<float> commsPCBTemperature(0);
    inline Parameter<float> commsGNSSTemperature(0);
    enum Antenna_Deployment_Status_enum : uint8_t {
        Closed = 0,
        OneDoorOpen = 1,
        TwoDoorOpen = 2,
        ThreeDoorOpen = 3,
        FullyDeployed = 4
    };
    inline Parameter<Antenna_Deployment_Status_enum> Antenna_Deployment_Status(Closed);
} // namespace COMMSParameters


namespace PAYParameters {

    enum ParameterID : uint16_t {

        xID = 15010,
        yID = 15020,
        zID = 15030,
        uptimeID = 15040,
        timeID = 15050,
        psu_12vID = 15060,
        psu_5vID = 15070,
        psu_33vID = 15080,
        mcu_die_temperatureID = 15090,
        main_board_temperatureID = 15100,
        seed_ld_output_powerID = 15110,
        fsm_chamber_temperatureID = 15120,
        fsm_chamber_pressureID = 15130,
        fsmd_voltage_converter_temperatureID = 15140,
        fsmd_drivers_temperatureID = 15150,
        camera_pcb_temperatureID = 15160,
        camera_sensor_temperatureID = 15170,
        fso_aux_temperature_1ID = 15171,
        fso_aux_temperature_2ID = 15172,
        oad_temperatureID = 15180,
        optical_amplifier_combiner_heater_temperatureID = 15181,
        optical_amplifier_fiber_mirror_temperatureID = 15182,
        optical_amplifier_circulator_heater_temperatureID = 15183,
        ldd_temperatureID = 15190,
        ld_temperatureID = 15200,
        ldd_12v_currentID = 15210,
        ldd_psu_12vID = 15211,
        ldd_converter_input_currentID = 15212,
        ldd_ld_vID = 15213,
        ldd_ld_dac_set_vID = 15214,
        fsm_driver_12v_currentID = 15220,
        flashes_33v_currentID = 15230,
        fpga_5v_currentID = 15240,
        sdd_33v_currentID = 15250,
        pump_ld_powerID = 15260,
        mcu_33v_currentID = 15270,
        fpga_die_temperatureID = 15280,
        fpga_vdd1_voltageID = 15290,
        fpga_vdd18_voltageID = 15300,
        fpga_vdd25_voltageID = 15310,
        seed_ld_incoming_powerID = 15320,
        amplifier_output_powerID = 15321,
        amplifier_output_reflected_powerID = 15322,
        seed_diode_bias_currentID = 15323,
        tec_currentID = 15330,
        ldd_output_currentID = 15340,
        sd_temperature_violationsID = 15341,
        end_uptimeID = 15350,
        responseID = 15360,
        deviceID = 15390,
        firmwareID = 15400,
        firmware_is_confirmedID = 15410,
        bitstreamID = 15420,
        softcpu_1_firmwareID = 15430,
        softcpu_2_firmwareID = 15440,
        softcpu_3_firmwareID = 15450,
        softcpu_4_firmwareID = 15460,
        softcpu_5_firmwareID = 15470,
        softcpu_6_firmwareID = 15480,
        softcpu_7_firmwareID = 15490,
        softcpu_8_firmwareID = 15500,
        boot_countID = 15510,
        transmission_countID = 15520,
        hw_detID = 15530,
        storage0ID = 15540,
        storage1ID = 15550,
        storage2ID = 15560,
        storage3ID = 15570,
        ldd_faultID = 15580,
        fsm_faultID = 15590,
        fpga_faultID = 15600,
        v_cam_faultID = 15620,
        sdd_faultID = 15630

    };

    inline Parameter<int32_t> x(0);
    inline Parameter<int32_t> y(0);
    inline Parameter<int32_t> z(0);
    inline Parameter<int32_t> uptime(0);
    inline Parameter<int32_t> time(0);
    inline Parameter<int32_t> psu_12v(0);
    inline Parameter<int32_t> psu_5v(0);
    inline Parameter<int32_t> psu_33v(0);
    inline Parameter<int32_t> mcu_die_temperature(0);
    inline Parameter<int32_t> main_board_temperature(0);
    inline Parameter<int32_t> seed_ld_output_power(0);
    inline Parameter<int32_t> fsm_chamber_temperature(0);
    inline Parameter<int32_t> fsm_chamber_pressure(0);
    inline Parameter<int32_t> fsmd_voltage_converter_temperature(0);
    inline Parameter<int32_t> fsmd_drivers_temperature(0);
    inline Parameter<int32_t> camera_pcb_temperature(0);
    inline Parameter<int32_t> camera_sensor_temperature(0);
    inline Parameter<int32_t> fso_aux_temperature_1(0);
    inline Parameter<int32_t> fso_aux_temperature_2(0);
    inline Parameter<int32_t> oad_temperature(0);
    inline Parameter<int32_t> optical_amplifier_combiner_heater_temperature(0);
    inline Parameter<int32_t> optical_amplifier_fiber_mirror_temperature(0);
    inline Parameter<int32_t> optical_amplifier_circulator_heater_temperature(0);
    inline Parameter<int32_t> ldd_temperature(0);
    inline Parameter<int32_t> ld_temperature(0);
    inline Parameter<int32_t> ldd_12v_current(0);
    inline Parameter<int32_t> ldd_psu_12v(0);
    inline Parameter<int32_t> ldd_converter_input_current(0);
    inline Parameter<int32_t> ldd_ld_v(0);
    inline Parameter<int32_t> ldd_ld_dac_set_v(0);
    inline Parameter<int32_t> fsm_driver_12v_current(0);
    inline Parameter<int32_t> flashes_33v_current(0);
    inline Parameter<int32_t> fpga_5v_current(0);
    inline Parameter<int32_t> sdd_33v_current(0);
    inline Parameter<int32_t> pump_ld_power(0);
    inline Parameter<int32_t> mcu_33v_current(0);
    inline Parameter<int32_t> fpga_die_temperature(0);
    inline Parameter<int32_t> fpga_vdd1_voltage(0);
    inline Parameter<int32_t> fpga_vdd18_voltage(0);
    inline Parameter<int32_t> fpga_vdd25_voltage(0);
    inline Parameter<int32_t> seed_ld_incoming_power(0);
    inline Parameter<int32_t> amplifier_output_power(0);
    inline Parameter<int32_t> amplifier_output_reflected_power(0);
    inline Parameter<int32_t> seed_diode_bias_current(0);
    inline Parameter<int32_t> tec_current(0);
    inline Parameter<int32_t> ldd_output_current(0);
    inline Parameter<int32_t> sd_temperature_violations(0);
    inline Parameter<int32_t> end_uptime(0);
    inline Parameter<int32_t> response(0);
    inline Parameter<int32_t> device(0);
    inline Parameter<int32_t> firmware(0);
    inline Parameter<int32_t> firmware_is_confirmed(0);
    inline Parameter<int32_t> bitstream(0);
    inline Parameter<int32_t> softcpu_1_firmware(0);
    inline Parameter<int32_t> softcpu_2_firmware(0);
    inline Parameter<int32_t> softcpu_3_firmware(0);
    inline Parameter<int32_t> softcpu_4_firmware(0);
    inline Parameter<int32_t> softcpu_5_firmware(0);
    inline Parameter<int32_t> softcpu_6_firmware(0);
    inline Parameter<int32_t> softcpu_7_firmware(0);
    inline Parameter<int32_t> softcpu_8_firmware(0);
    inline Parameter<int32_t> boot_count(0);
    inline Parameter<int32_t> transmission_count(0);
    inline Parameter<int32_t> hw_det(0);
    inline Parameter<int32_t> storage0(0);
    inline Parameter<int32_t> storage1(0);
    inline Parameter<int32_t> storage2(0);
    inline Parameter<int32_t> storage3(0);
    inline Parameter<int32_t> ldd_fault(0);
    inline Parameter<int32_t> fsm_fault(0);
    inline Parameter<int32_t> fpga_fault(0);
    inline Parameter<int32_t> v_cam_fault(0);
    inline Parameter<int32_t> sdd_fault(0);
} // namespace PAYParameters


#pragma GCC diagnostic pop