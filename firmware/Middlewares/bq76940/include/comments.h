/**
 * Checks if temperatures are within the limits, otherwise disables CHG/DSG FET
 *
 * This function is necessary as bq769x0 doesn't support temperature protection
 */
// static void bq_check_cell_temp(BQ *bq);

// static void bq_update_balancing(BQ *bq);

// void bq_update(BQ *bq)
// {
//     bq_read_voltages(bq);
//     bq_read_current(bq);
//     bq_soc_update(bq);
//     bq_read_temperatures(bq);
//     bq_check_cell_temp(bq); // bq769x0 doesn't support temperature settings
//     bq_update_error_flags(bq);
//     bq_update_balancing(bq);
// }

// void bq_set_error_flag(BQ *bq, uint32_t flag, bool value)
// {
//     // check if error flag changed
//     if ((bq->status.error_flags & (1UL << flag)) != ((uint32_t)value <<
//     flag)) {
//         if (value) {
//             bq->status.error_flags |= (1UL << flag);
//         }
//         else {
//             bq->status.error_flags &= ~(1UL << flag);
//         }

//         LOG_DEBUG("Error flag %u changed to: %d", flag, value);
//     }
// }

// void bq_check_cell_temp(BQ *bq)
// {
//     float hyst;

//     hyst = (bq->status.error_flags & (1UL << BMS_ERR_CHG_OVERTEMP)) ?
//     bq->conf.t_limit_hyst : 0; bool chg_overtemp = bq->status.bat_temp_max
//     > bq->conf.chg_ot_limit - hyst;

//     hyst = (bq->status.error_flags & (1UL << BMS_ERR_CHG_UNDERTEMP)) ?
//     bq->conf.t_limit_hyst : 0; bool chg_undertemp = bq->status.bat_temp_min
//     < bq->conf.chg_ut_limit + hyst;

//     hyst = (bq->status.error_flags & (1UL << BMS_ERR_DIS_OVERTEMP)) ?
//     bq->conf.t_limit_hyst : 0; bool dis_overtemp = bq->status.bat_temp_max
//     > bq->conf.dis_ot_limit - hyst;

//     hyst = (bq->status.error_flags & (1UL << BMS_ERR_DIS_OVERTEMP)) ?
//     bq->conf.t_limit_hyst : 0; bool dis_undertemp = bq->status.bat_temp_min
//     < bq->conf.dis_ut_limit + hyst;

//     if (chg_overtemp != (bool)(bq->status.error_flags & (1UL <<
//     BMS_ERR_CHG_OVERTEMP))) {
//         bq_set_error_flag(bq, BMS_ERR_CHG_OVERTEMP, chg_overtemp);
//     }

//     if (chg_undertemp != (bool)(bq->status.error_flags & (1UL <<
//     BMS_ERR_CHG_UNDERTEMP))) {
//         bq_set_error_flag(bq, BMS_ERR_CHG_UNDERTEMP, chg_undertemp);
//     }

//     if (dis_overtemp != (bool)(bq->status.error_flags & (1UL <<
//     BMS_ERR_DIS_OVERTEMP))) {
//         bq_set_error_flag(bq, BMS_ERR_DIS_OVERTEMP, dis_overtemp);
//     }

//     if (dis_undertemp != (bool)(bq->status.error_flags & (1UL <<
//     BMS_ERR_DIS_UNDERTEMP))) {
//         bq_set_error_flag(bq, BMS_ERR_DIS_UNDERTEMP, dis_undertemp);
//     }
// }

// static void bq_update_balancing(BQ *bq)
// {
//     long idle_secs = uptime() - bq->status.no_idle_timestamp;
//     int num_sections = BOARD_NUM_CELLS_MAX / 5;

//     // check for millisecond-timer overflow
//     if (idle_secs < 0) {
//         bq->status.no_idle_timestamp = 0;
//         idle_secs = uptime();
//     }

//     // check if balancing allowed
//     if (idle_secs >= bq->conf.bal_idle_delay
//         && bq->status.cell_voltage_max > bq->conf.bal_cell_voltage_min
//         && (bq->status.cell_voltage_max - bq->status.cell_voltage_min)
//                > bq->conf.bal_cell_voltage_diff)
//     {
//         bq->status.balancing_status = 0; // current status will be set in
//         following loop

//         int balancing_flags;
//         int balancing_flags_target;

//         for (int section = 0; section < num_sections; section++) {
//             // find cells which should be balanced and sort them by voltage
//             descending int cell_list[5]; int cell_counter = 0; for (int i =
//             0; i < 5; i++) {
//                 if ((bq->status.cell_voltages[section * 5 + i] -
//                 bq->status.cell_voltage_min)
//                     > bq->conf.bal_cell_voltage_diff)
//                 {
//                     int j = cell_counter;
//                     while (j > 0
//                            && bq->status.cell_voltages[section * 5 +
//                            cell_list[j - 1]]
//                                   < bq->status.cell_voltages[section * 5 +
//                                   i])
//                     {
//                         cell_list[j] = cell_list[j - 1];
//                         j--;
//                     }
//                     cell_list[j] = i;
//                     cell_counter++;
//                 }
//             }

//             balancing_flags = 0;
//             for (int i = 0; i < cell_counter; i++) {
//                 // try to enable balancing of current cell
//                 balancing_flags_target = balancing_flags | (1 <<
//                 cell_list[i]);

//                 // check if attempting to balance adjacent cells
//                 bool adjacent_cell_collision = ((balancing_flags_target << 1)
//                 & balancing_flags)
//                                                || ((balancing_flags << 1) &
//                                                balancing_flags_target);

//                 if (adjacent_cell_collision == false) {
//                     balancing_flags = balancing_flags_target;
//                 }
//             }

//             LOG_DEBUG("Setting CELLBAL%d register to: %s", section + 1,
//             byte2bitstr(balancing_flags));

//             bq->status.balancing_status |= balancing_flags << section * 5;

//             // set balancing register for this section
//             bq769x0_write_byte(BQ769X0_CELLBAL1 + section, balancing_flags);

//         } // section loop
//     }
//     else if (bq->status.balancing_status > 0) {
//         // clear all CELLBAL registers
//         for (int section = 0; section < num_sections; section++) {
//             LOG_DEBUG("Clearing Register CELLBAL%d\n", section + 1);
//             bq769x0_write_byte(BQ769X0_CELLBAL1 + section, 0x0);
//         }

//         bq->status.balancing_status = 0;
//     }
// }

// void bq_update_error_flags(BQ *bq)
// {
//     SYS_STAT_Type sys_stat;
//     sys_stat.byte = bq769x0_read_byte(BQ769X0_SYS_STAT);

//     uint32_t error_flags_temp = 0;
//     if (sys_stat.UV)
//         error_flags_temp |= 1U << BMS_ERR_CELL_UNDERVOLTAGE;
//     if (sys_stat.OV)
//         error_flags_temp |= 1U << BMS_ERR_CELL_OVERVOLTAGE;
//     if (sys_stat.SCD)
//         error_flags_temp |= 1U << BMS_ERR_SHORT_CIRCUIT;
//     if (sys_stat.OCD)
//         error_flags_temp |= 1U << BMS_ERR_DIS_OVERCURRENT;

//     if (bq->status.pack_current > bq->conf.chg_oc_limit) {
//         // ToDo: consider bq->conf.chg_oc_delay
//         error_flags_temp |= 1U << BMS_ERR_CHG_OVERCURRENT;
//     }

//     if (bq->status.bat_temp_max > bq->conf.chg_ot_limit
//         || (bq->status.error_flags & (1UL << BMS_ERR_CHG_OVERTEMP)))
//     {
//         error_flags_temp |= 1U << BMS_ERR_CHG_OVERTEMP;
//     }

//     if (bq->status.bat_temp_min < bq->conf.chg_ut_limit
//         || (bq->status.error_flags & (1UL << BMS_ERR_CHG_UNDERTEMP)))
//     {
//         error_flags_temp |= 1U << BMS_ERR_CHG_UNDERTEMP;
//     }

//     if (bq->status.bat_temp_max > bq->conf.dis_ot_limit
//         || (bq->status.error_flags & (1UL << BMS_ERR_DIS_OVERTEMP)))
//     {
//         error_flags_temp |= 1U << BMS_ERR_DIS_OVERTEMP;
//     }

//     if (bq->status.bat_temp_min < bq->conf.dis_ut_limit
//         || (bq->status.error_flags & (1UL << BMS_ERR_DIS_UNDERTEMP)))
//     {
//         error_flags_temp |= 1U << BMS_ERR_DIS_UNDERTEMP;
//     }

//     bq->status.error_flags = error_flags_temp;
// }

// void bq_handle_errors(BQ *bq)
// {
//     static uint16_t error_status = 0;
//     static uint32_t sec_since_error = 0;

//     // ToDo: Handle also temperature and chg errors (incl. temp hysteresis)
//     SYS_STAT_Type sys_stat;
//     sys_stat.byte = bq769x0_read_byte(BQ769X0_SYS_STAT);
//     error_status = sys_stat.byte;

//     if (!bq769x0_alert_flag() && error_status == 0) {
//         return;
//     }
//     else if (sys_stat.byte & BQ769X0_SYS_STAT_ERROR_MASK) {

//         if (bq769x0_alert_flag() == true) {
//             sec_since_error = 0;
//         }

//         unsigned int sec_since_interrupt = uptime() -
//         bq769x0_alert_timestamp();

//         if (abs((long)(sec_since_interrupt - sec_since_error)) > 2) {
//             sec_since_error = sec_since_interrupt;
//         }

//         // called only once per second
//         if (sec_since_interrupt >= sec_since_error) {
//             if (sys_stat.DEVICE_XREADY) {
//                 // datasheet recommendation: try to clear after waiting a few
//                 seconds if (sec_since_error % 3 == 0) {
//                     LOG_DEBUG("Attempting to clear XR error");
//                     bq769x0_write_byte(BQ769X0_SYS_STAT,
//                     BQ769X0_SYS_STAT_DEVICE_XREADY); bq_chg_switch(bq,
//                     true); bq_dis_switch(bq, true);
//                 }
//             }
//             if (sys_stat.OVRD_ALERT) {
//                 if (sec_since_error % 10 == 0) {
//                     LOG_DEBUG("Attempting to clear Alert error");
//                     bq769x0_write_byte(BQ769X0_SYS_STAT,
//                     BQ769X0_SYS_STAT_OVRD_ALERT); bq_chg_switch(bq, true);
//                     bq_dis_switch(bq, true);
//                 }
//             }
//             if (sys_stat.UV) {
//                 bq_read_voltages(bq);
//                 if (bq->status.cell_voltage_min > bq->conf.cell_uv_reset) {
//                     LOG_DEBUG("Attempting to clear UV error");
//                     bq769x0_write_byte(BQ769X0_SYS_STAT,
//                     BQ769X0_SYS_STAT_UV); bq_dis_switch(bq, true);
//                 }
//             }
//             if (sys_stat.OV) {
//                 bq_read_voltages(bq);
//                 if (bq->status.cell_voltage_max < bq->conf.cell_ov_reset) {
//                     LOG_DEBUG("Attempting to clear OV error");
//                     bq769x0_write_byte(BQ769X0_SYS_STAT,
//                     BQ769X0_SYS_STAT_OV); bq_chg_switch(bq, true);
//                 }
//             }
//             if (sys_stat.SCD) {
//                 if (sec_since_error % 60 == 0) {
//                     LOG_DEBUG("Attempting to clear SCD error");
//                     bq769x0_write_byte(BQ769X0_SYS_STAT,
//                     BQ769X0_SYS_STAT_SCD); bq_dis_switch(bq, true);
//                 }
//             }
//             if (sys_stat.OCD) {
//                 if (sec_since_error % 60 == 0) {
//                     LOG_DEBUG("Attempting to clear OCD error");
//                     bq769x0_write_byte(BQ769X0_SYS_STAT,
//                     BQ769X0_SYS_STAT_OCD); bq_dis_switch(bq, true);
//                 }
//             }
//             sec_since_error++;
//         }
//     }
//     else {
//         error_status = 0;
//     }
// }

// void bq_print_register(uint16_t addr)
// {
//     uint8_t reg = bq769x0_read_byte((uint8_t)addr);
//     printf("0x%.2X: 0x%.2X = %s\n", addr, reg, byte2bitstr(reg));
// }

// void bq_print_registers()
// {
//     printf("0x00 SYS_STAT:  %s\n",
//     byte2bitstr(bq769x0_read_byte(BQ769X0_SYS_STAT))); printf("0x01 CELLBAL1:
//     %s\n", byte2bitstr(bq769x0_read_byte(BQ769X0_CELLBAL1))); printf("0x04
//     SYS_CTRL1: %s\n", byte2bitstr(bq769x0_read_byte(BQ769X0_SYS_CTRL1)));
//     printf("0x05 SYS_CTRL2: %s\n",
//     byte2bitstr(bq769x0_read_byte(BQ769X0_SYS_CTRL2))); printf("0x06
//     PROTECT1:  %s\n", byte2bitstr(bq769x0_read_byte(BQ769X0_PROTECT1)));
//     printf("0x07 PROTECT2:  %s\n",
//     byte2bitstr(bq769x0_read_byte(BQ769X0_PROTECT2))); printf("0x08 PROTECT3:
//     %s\n", byte2bitstr(bq769x0_read_byte(BQ769X0_PROTECT3))); printf("0x09
//     OV_TRIP:   %s\n", byte2bitstr(bq769x0_read_byte(BQ769X0_OV_TRIP)));
//     printf("0x0A UV_TRIP:   %s\n",
//     byte2bitstr(bq769x0_read_byte(BQ769X0_UV_TRIP))); printf("0x0B CC_CFG:
//     %s\n", byte2bitstr(bq769x0_read_byte(BQ769X0_CC_CFG))); printf("0x32
//     CC_HI:     %s\n", byte2bitstr(bq769x0_read_byte(BQ769X0_CC_HI_BYTE)));
//     printf("0x33 CC_LO:     %s\n",
//     byte2bitstr(bq769x0_read_byte(BQ769X0_CC_LO_BYTE)));
//     /*
//     printf("0x50 BQ769X0_ADCGAIN1:  %s\n",
//     byte2bitstr(bq769x0_read_byte(BQ769X0_ADCGAIN1))); printf("0x51
//     BQ769X0_ADCOFFSET: %s\n",
//     byte2bitstr(bq769x0_read_byte(BQ769X0_ADCOFFSET))); printf("0x59
//     BQ769X0_ADCGAIN2:  %s\n",
//     byte2bitstr(bq769x0_read_byte(BQ769X0_ADCGAIN2)));
//     */
// }

// /**
//  * Possible BQ states
//  */
// enum BQState {
//   BQ_STATE_OFF,      ///< Off state (charging and discharging disabled)
//   BQ_STATE_CHG,      ///< Charging state (discharging disabled)
//   BQ_STATE_DIS,      ///< Discharging state (charging disabled)
//   BQ_STATE_NORMAL,   ///< Normal operating mode (both charging and
//   discharging
//                       ///< enabled)
//   BQ_STATE_SHUTDOWN, ///< BQ starting shutdown sequence
// };

// /**
//  * Battery cell types
//  */
// enum CellType {
//   CELL_TYPE_CUSTOM = 0, ///< Custom settings
//   CELL_TYPE_LFP,        ///< LiFePO4 Li-ion cells (3.3 V nominal)
//   CELL_TYPE_NMC,        ///< NMC/Graphite Li-ion cells (3.7 V nominal)
//   CELL_TYPE_NMC_HV,     ///< NMC/Graphite High Voltage Li-ion cells (3.7 V
//                         ///< nominal, 4.35 V max)
//   CELL_TYPE_LTO         ///< NMC/Titanate (2.4 V nominal)
// };

// /**
//  * BQ configuration values, stored in RAM. The configuration is not
//  * automatically applied after values are changed!
//  */
// typedef struct {
//   /// Effective resistance of the current measurement shunt(s) on the PCB
//   /// (milli-Ohms)
//   float shunt_res_mOhm;

//   /// Beta value of the used thermistor. Typical value for Semitec 103AT-5
//   /// thermistor: 3435
//   uint16_t thermistor_beta;

//   // Current limits
//   float dis_sc_limit;       ///< Discharge short circuit limit (A)
//   uint32_t dis_sc_delay_us; ///< Discharge short circuit delay (us)
//   float dis_oc_limit;       ///< Discharge over-current limit (A)
//   uint32_t dis_oc_delay_ms; ///< Discharge over-current delay (ms)
//   float chg_oc_limit;       ///< Charge over-current limit (A)
//   uint32_t chg_oc_delay_ms; ///< Charge over-current delay (ms)

//   // Cell voltage limits
//   float cell_chg_voltage;    ///< Cell target charge voltage (V)
//   float cell_dis_voltage;    ///< Cell discharge voltage limit (V)
//   float cell_ov_limit;       ///< Cell over-voltage limit (V)
//   float cell_ov_reset;       ///< Cell over-voltage error reset threshold (V)
//   uint32_t cell_ov_delay_ms; ///< Cell over-voltage delay (ms)
//   float cell_uv_limit;       ///< Cell under-voltage limit (V)
//   float cell_uv_reset;       ///< Cell under-voltage error reset threshold
//   (V) uint32_t cell_uv_delay_ms; ///< Cell under-voltage delay (ms)

//   // Temperature limits (°C)
//   float dis_ot_limit; ///< Discharge over-temperature (DOT) limit (°C)
//   float dis_ut_limit; ///< Discharge under-temperature (DUT) limit (°C)
//   float chg_ot_limit; ///< Charge over-temperature (COT) limit (°C)
//   float chg_ut_limit; ///< Charge under-temperature (CUT) limit (°C)
//   float t_limit_hyst; ///< Temperature limit hysteresis (°C)

//   // Balancing settings
//   float bal_cell_voltage_diff; ///< Balancing cell voltage target difference
//   (V) float bal_cell_voltage_min;  ///< Minimum cell voltage to start
//   balancing (V) float bal_idle_current;      ///< Current threshold to be
//   considered idle (A) uint16_t bal_idle_delay;     ///< Minimum idle duration
//   before balancing (s)
// } BQConfig;

// /**
//  * Current BQ status including measurements and error flags
//  */
// typedef struct {
//   uint16_t state;  ///< Current state of the battery
//   bool chg_enable; ///< Manual enable/disable setting for charging
//   bool dis_enable; ///< Manual enable/disable setting for discharging

//   uint16_t connected_cells; ///< \brief Actual number of cells connected
//   (might
//                             ///< be less than BOARD_NUM_CELLS_MAX)

//   float cell_voltages[BOARD_NUM_CELLS_MAX]; ///< Single cell voltages (V)
//   float cell_voltage_max;                   ///< Maximum cell voltage (V)
//   float cell_voltage_min;                   ///< Minimum cell voltage (V)
//   float cell_voltage_avg;                   ///< Average cell voltage (V)
//   float pack_voltage;                       ///< Battery pack voltage (V)

//   float pack_current; ///< \brief Battery pack current, charging direction
//                       ///< has positive sign (A)

//   float bat_temps[BOARD_NUM_THERMISTORS_MAX]; ///< Battery temperatures (°C)
//   float bat_temp_max; ///< Maximum battery temperature (°C)
//   float bat_temp_min; ///< Minimum battery temperature (°C)
//   float bat_temp_avg; ///< Average battery temperature (°C)
//   float mosfet_temp;  ///< MOSFET temperature (°C)
//   float ic_temp;      ///< Internal BQ IC temperature (°C)
//   float mcu_temp;     ///< MCU temperature (°C)

//   bool full;  ///< CV charging to cell_chg_voltage finished
//   bool empty; ///< Battery is discharged below cell_dis_voltage

//   float soc; ///< Calculated State of Charge (%)

//   uint32_t balancing_status; ///< holds on/off status of balancing switches

//   uint32_t error_flags; ///< Bit array for different BQErrorFlag errors
// } BQStatus;

// /**
//  * BQ error flags
//  */
// enum BQErrorFlag {
//   BQ_ERR_CELL_UNDERVOLTAGE = 0, ///< Cell undervoltage flag
//   BQ_ERR_CELL_OVERVOLTAGE = 1,  ///< Cell undervoltage flag
//   BQ_ERR_SHORT_CIRCUIT = 2,     ///< Pack short circuit (discharge direction)
//   BQ_ERR_DIS_OVERCURRENT = 3,   ///< Pack overcurrent (discharge direction)
//   BQ_ERR_CHG_OVERCURRENT = 4,   ///< Pack overcurrent (charge direction)
//   BQ_ERR_OPEN_WIRE = 5,         ///< Cell open wire
//   BQ_ERR_DIS_UNDERTEMP = 6,     ///< Temperature below discharge minimum
//   limit BQ_ERR_DIS_OVERTEMP = 7,      ///< Temperature above discharge
//   maximum limit BQ_ERR_CHG_UNDERTEMP = 8,     ///< Temperature below charge
//   maximum limit BQ_ERR_CHG_OVERTEMP = 9,      ///< Temperature above charge
//   maximum limit BQ_ERR_INT_OVERTEMP = 10, ///< Internal temperature above
//   limit (e.g. BQ IC) BQ_ERR_CELL_FAILURE = 11, ///< Cell failure (too high
//   voltage difference) BQ_ERR_DIS_OFF = 12, ///< Discharge FET is off even
//   though it should be on BQ_ERR_CHG_OFF = 13, ///< Charge FET is off even
//   though it should be on BQ_ERR_FET_OVERTEMP = 14, ///< MOSFET temperature
//   above limit
// };

// typedef struct {
//   BQConfig conf;
//   BQStatus status;
// } BQ;

// /**
//  * Initialization of BQStatus with suitable default values.
//  *
//  * @param bg Pointer to BQ object.
//  */
// void bg_init_status(BQ *bg);

// /**
//  * Initialization of BQConfig with typical default values for the given cell
//  * type.
//  *
//  * @param bg Pointer to BQ object.
//  * @param type One of enum CellType (defined as int so that it can be set via
//  * Kconfig).
//  * @param nominal_capacity Nominal capacity of the battery pack.
//  */
// void bg_init_config(BQ *bg, int type, float nominal_capacity);

// /**
//  * Initialization of BQ incl. setup of communication. This function does not
//  * yet set any config.
//  *
//  * @param bg Pointer to BQ object.
//  *
//  * @returns 0 on success, otherwise negative error code.
//  */
// int bg_init_hardware(BQ *bg);

// /**
//  * Main BQ state machine
//  *
//  * @param bg Pointer to BQ object.
//  */
// void bg_state_machine(BQ *bg);

// /**
//  * Update measurements and check for errors before calling the state machine
//  *
//  * Should be called at least once every 250 ms to get correct coulomb
//  counting
//  *
//  * @param bg Pointer to BQ object.
//  */
// void bg_update(BQ *bg);

// /**
//  * BQ IC start-up delay indicator
//  *
//  * @returns true if we should wait for the BQ IC to start up
//  */
// bool bg_startup_inhibit();

// /**
//  * Shut down BQ IC and entire PCB power supply
//  */
// void bg_shutdown(BQ *bg);

// /**
//  * Enable/disable charge MOSFET
//  *
//  * @param bg Pointer to BQ object.
//  * @param enable Desired status of the MOSFET.
//  *
//  * @returns 0 on success, otherwise negative error code.
//  */
// int bg_chg_switch(BQ *bg, bool enable);

// /**
//  * Enable/disable discharge MOSFET
//  *
//  * @param bg Pointer to BQ object.
//  * @param enable Desired status of the MOSFET.
//  *
//  * @returns 0 on success, otherwise negative error code.
//  */
// int bg_dis_switch(BQ *bg, bool enable);

// /**
//  * Charging error flags check
//  *
//  * @returns true if any charging error flag is set
//  */
// bool bg_chg_error(uint32_t error_flags);

// /**
//  * Discharging error flags check
//  *
//  * @returns true if any discharging error flag is set
//  */
// bool bg_dis_error(uint32_t error_flags);

// /**
//  * Check if charging is allowed
//  *
//  * @returns true if no charging error flags are set
//  */
// bool bg_chg_allowed(BQ *bg);

// /**
//  * Check if discharging is allowed
//  *
//  * @param bg Pointer to BQ object.
//  *
//  * @returns true if no discharging error flags are set
//  */
// bool bg_dis_allowed(BQ *bg);

// /**
//  * Balancing limits check
//  *
//  * @param bg Pointer to BQ object.
//  *
//  * @returns if balancing is allowed
//  */
// bool bg_balancing_allowed(BQ *bg);

// /**
//  * Reset SOC to specified value or calculate based on average cell open
//  circuit
//  * voltage
//  *
//  * @param bg Pointer to BQ object.
//  * @param percent 0-100 %, -1 for calculation based on OCV
//  */
// void bg_soc_reset(BQ *bg, int percent);

// /**
//  * Update SOC based on most recent current measurement
//  *
//  * Function should be called each time after a new current measurement was
//  * obtained.
//  *
//  * @param bg Pointer to BQ object.
//  */
// void bg_soc_update(BQ *bg);

// /**
//  * Apply charge/discharge temperature limits.
//  *
//  * @param conf BQ configuration containing the limit settings
//  *
//  * @returns 0 on success, otherwise negative error code.
//  */
// int bg_apply_temp_limits(BQ *bg);

// /**
//  * Apply discharge short circuit protection (SCP) current threshold and
//  delay.
//  *
//  * If the setpoint does not exactly match a possible setting in the BQ IC, it
//  * is rounded to the closest allowed value and this value is written back to
//  the
//  * BQ config.
//  *
//  * @param bg Pointer to BQ object.
//  *
//  * @returns 0 on success, otherwise negative error code.
//  */
// int bg_apply_dis_scp(BQ *bg);

// /**
//  * Apply discharge overcurrent protection (OCP) threshold and delay.
//  *
//  * If the setpoint does not exactly match a possible setting in the BQ IC, it
//  * is rounded to the closest allowed value and this value is written back to
//  the
//  * BQ config.
//  *
//  * @param bg Pointer to BQ object.
//  *
//  * @returns 0 on success, otherwise negative error code.
//  */
// int bg_apply_dis_ocp(BQ *bg);

// /**
//  * Apply charge overcurrent protection (OCP) threshold and delay.
//  *
//  * If the setpoint does not exactly match a possible setting in the BQ IC, it
//  * is rounded to the closest allowed value and this value is written back to
//  the
//  * BQ config.
//  *
//  * @param bg Pointer to BQ object.
//  *
//  * @returns 0 on success, otherwise negative error code.
//  */
// int bg_apply_chg_ocp(BQ *bg);

// /**
//  * Apply cell undervoltage protection (UVP) threshold and delay.
//  *
//  * @param bg Pointer to BQ object.
//  *
//  * @returns 0 on success, otherwise negative error code.
//  */
// int bg_apply_cell_uvp(BQ *bg);

// /**
//  * Apply cell overvoltage protection (OVP) threshold and delay.
//  *
//  * @param bg Pointer to BQ object.
//  *
//  * @returns 0 on success, otherwise negative error code.
//  */
// int bg_apply_cell_ovp(BQ *bg);

// /**
//  * Apply balancing settings
//  *
//  * Balancing is automatically enabled by the chip or by the update function
//  as
//  * soon as if voltage, temperature and idle citerions are met.
//  *
//  * @param bg Pointer to BQ object.
//  *
//  * @returns 0 on success, otherwise negative error code.
//  */
// int bg_apply_balancing_conf(BQ *bg);

// /**
//  * Reads all cell voltages to array cell_voltages[NUM_CELLS], updates
//  * battery_voltage and updates ids of cells with min/max voltage
//  *
//  * @param bg Pointer to BQ object.
//  */
// void bg_read_voltages(BQ *bg);

// /**
//  * Reads pack current and updates coloumb counter and SOC
//  *
//  * @param bg Pointer to BQ object.
//  */
// void bg_read_current(BQ *bg);

// /**
//  * Reads all temperature sensors
//  *
//  * @param bg Pointer to BQ object.
//  */
// void bg_read_temperatures(BQ *bg);

// /**
//  * Reads error flags from IC or updates them based on measurements
//  *
//  * @param bg Pointer to BQ object.
//  */
// void bg_update_error_flags(BQ *bg);

// /**
//  * Tries to handle / resolve errors
//  *
//  * @param bg Pointer to BQ object.
//  */
// void bg_handle_errors(BQ *bg);

// /**
//  * Print BQ IC register
//  *
//  * @param addr Address of the register
//  */
// void bg_print_register(uint16_t addr);

// /**
//  * Print all BQ IC registers
//  */
// void bg_print_registers();