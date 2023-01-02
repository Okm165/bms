#ifndef _BQ76940_H_
#define _BQ76940_H_

/** @file
 *
 * @brief
 * Battery Management System (BQ) module for different analog frontends
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "registers.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define BOARD_NUM_CELLS_MAX 15

/**
 * Battery cell ids
 */
enum CellId {
  BQ_CELL_1 = BQ769X0_VC1_HI_BYTE,
  BQ_CELL_2 = BQ769X0_VC2_HI_BYTE,
  BQ_CELL_3 = BQ769X0_VC3_HI_BYTE,
  BQ_CELL_4 = BQ769X0_VC4_HI_BYTE,
  BQ_CELL_5 = BQ769X0_VC5_HI_BYTE,
  BQ_CELL_6 = BQ769X0_VC6_HI_BYTE,
  BQ_CELL_7 = BQ769X0_VC7_HI_BYTE,
  BQ_CELL_8 = BQ769X0_VC8_HI_BYTE,
  BQ_CELL_9 = BQ769X0_VC9_HI_BYTE,
  BQ_CELL_10 = BQ769X0_VC10_HI_BYTE,
  BQ_CELL_11 = BQ769X0_VC11_HI_BYTE,
  BQ_CELL_12 = BQ769X0_VC12_HI_BYTE,
  BQ_CELL_13 = BQ769X0_VC13_HI_BYTE,
  BQ_CELL_14 = BQ769X0_VC14_HI_BYTE,
  BQ_CELL_15 = BQ769X0_VC15_HI_BYTE
};

/**
 * TSx_HI and TSx_LO temperature source
 */
enum TS_SRC {
  TS_SRC_Internal = 0x0,
  TS_SRC_External = 0x1,
};

/**
 * Internal Termistors ids
 */
enum TS_INT_Id {
  BQ_TS_INT_1 = BQ769X0_TS2_HI_BYTE,
  BQ_TS_INT_2 = BQ769X0_TS3_HI_BYTE,
};

/**
 * External Termistors ids
 */
enum TS_EXT_Id {
  BQ_TS_EXT_1 = BQ769X0_TS1_HI_BYTE,
  BQ_TS_EXT_2 = BQ769X0_TS2_HI_BYTE,
  BQ_TS_EXT_3 = BQ769X0_TS3_HI_BYTE,
};

/**
 * Short circuit in discharge threshold setting
 */
enum SCD_T {
  SCD_T_44mV = 0x0,
  SCD_T_67mV = 0x1,
  SCD_T_89mV = 0x2,
  SCD_T_111mV = 0x3,
  SCD_T_133mV = 0x4,
  SCD_T_155mV = 0x5,
  SCD_T_178mV = 0x6,
  SCD_T_200mV = 0x7
};

/**
 * Short circuit in discharge delay setting
 */
enum SCD_D {
  SCD_D_70us = 0x0,
  SCD_D_100us = 0x1,
  SCD_D_200us = 0x2,
  SCD_D_400us = 0x3
};

/**
 * Overcurrent in discharge threshold setting
 */
enum OCD_T {
  OCD_T_17mV = 0x0,
  OCD_T_22mV = 0x1,
  OCD_T_28mV = 0x2,
  OCD_T_33mV = 0x3,
  OCD_T_39mV = 0x4,
  OCD_T_44mV = 0x5,
  OCD_T_50mV = 0x6,
  OCD_T_56mV = 0x7,
  OCD_T_61mV = 0x8,
  OCD_T_67mV = 0x9,
  OCD_T_72mV = 0xA,
  OCD_T_78mV = 0xB,
  OCD_T_83mV = 0xC,
  OCD_T_89mV = 0xD,
  OCD_T_94mV = 0xE,
  OCD_T_100mV = 0xF,
};

/**
 * Overcurrent in discharge delay setting
 */
enum OCD_D {
  OCD_D_8ms = 0x0,
  OCD_D_20ms = 0x1,
  OCD_D_40ms = 0x2,
  OCD_D_80ms = 0x3,
  OCD_D_160ms = 0x4,
  OCD_D_320ms = 0x5,
  OCD_D_640ms = 0x6,
  OCD_D_1280ms = 0x7,
};

/**
 * Undervoltage delay setting
 */
enum UV_D {
  UV_D_1s = 0x0,
  UV_D_4s = 0x1,
  UV_D_8s = 0x2,
  UV_D_16s = 0x3,
};

/**
 * Overvoltage delay setting
 */
enum OV_D {
  OV_D_1s = 0x0,
  OV_D_2s = 0x1,
  OV_D_4s = 0x2,
  OV_D_8s = 0x3,
};

/**
 * BQ configuration
 */
typedef struct {
  enum SCD_T scp_threshold;
  enum SCD_D scp_delay;

  enum OCD_T ocp_threshold;
  enum OCD_D ocp_delay;

  double uv_threshold;
  enum UV_D uv_delay;

  double ov_threshold;
  enum OV_D ov_delay;

  enum TS_SRC temp_source;
} BQConfig;

/**
 * Read temperature in Kelvin from internal thermistor
 * Remeber: bq769x0_change_temperature_source should be called if necessary
 * @param id - internal thermistor identifier
 *
 * When switching between external and internal temperature monitoring, a 2-s
 * latency may be incurred due to the natural scheduler update interval.
 *
 * @returns temperature value in Kelvin
 */
double bq769x0_read_temperature_internal(enum TS_INT_Id id);

/**
 * Read temperature in Kelvin from external thermistor
 * Remeber: bq769x0_change_temperature_source should be called if necessary
 * @param id - external thermistor identifier
 * @param thermistor_beta - thermistor beta coefficient
 * @param thermistor_nominal_resistance - thermistor resistance at standard
 * conditions (25 C)
 *
 * When switching between external and internal temperature monitoring, a 2-s
 * latency may be incurred due to the natural scheduler update interval.
 *
 * @returns temperature value in Kelvin
 */
double bq769x0_read_temperature_external(enum TS_EXT_Id id,
                                         double thermistor_beta,
                                         double thermistor_nominal_resistance);

/**
 * Read voltage in milivolts related with ohms law to current flow
 * @param milivolts - pointer that will be overwriten with value
 *
 * @returns True on success False otherwise
 */
bool bq769x0_read_current(double *milivolts);

/**
 * Read battery cell voltage
 * @param id - cell identifier
 *
 * @returns voltage of cell [V]
 */
double bq769x0_read_cell_voltage(enum CellId id);

/**
 * Read battery cell voltage
 * @param num_active_cells - num of connected cells for best error correction
 *
 * @returns voltage of pack [V]
 */
double bq769x0_read_pack_voltage(int num_active_cells);

/**
 * Toggle charging mosfet
 * @param enable - desired state
 *
 * @returns True on success False otherwise
 */
bool bq_chg_switch(bool enable);

/**
 * Toggle discharging mosfet
 * @param enable - desired state
 *
 * @returns True on success False otherwise
 */
bool bq_dis_switch(bool enable);

/**
 * Set short circuit protection during dicharge
 * @param threshold - short circuit in discharge threshold
 * @param delay - short circuit in discharge delay
 *
 * @returns True on success False otherwise
 */
bool bq_set_dis_scp(enum SCD_T threshold, enum SCD_D delay);

/**
 * Set over-current protection during dicharge
 * @param threshold - over-current in discharge threshold
 * @param delay - over-current in discharge delay
 *
 * @returns True on success False otherwise
 */
bool bq_set_dis_ocp(enum OCD_T threshold, enum OCD_D delay);

/**
 * Set under-voltage protection
 * @param uv_threshold - under-voltage [V]
 * @param uv_delay - under-voltage delay
 *
 * @returns True on success False otherwise
 */
bool bq_set_uvp(double uv_threshold, enum UV_D uv_delay);

/**
 * Set over-voltage protection
 * @param ov_threshold - over-voltage [V]
 * @param ov_delay - over-voltage delay
 *
 * @returns True on success False otherwise
 */
bool bq_set_ovp(double ov_threshold, enum OV_D ov_delay);

/**
 * Returns system faults object
 * @returns SYS_STAT_Type
 */
SYS_STAT_Type bq769x0_system_faults();

/**
 * Clears system faults register
 * @returns void
 */
void bq769x0_system_clear_faults(SYS_STAT_Type faults);

/**
 * Sets balancing fets according to flags in balancingFlags
 * each bit represents flag for one cell starting from 0 ending at 14 index
 * @returns true if balancing setup is passible (no adjacent cells are balanced
 * at the same time) false otherwise
 */
bool bq769x0_set_cell_balancing(uint16_t balancingFlags);

/**
 * Initialization of bq769x0 IC
 *
 * - Determines I2C address
 * - Sets ALERT pin interrupt
 */
int bq769x0_init(BQConfig *bg_conf);

/**
 * Put bq769x0 IC in shutdown mode
 * - Can be waken with boot button
 */
void bq_shutdown();

#ifdef __cplusplus
}
#endif

#endif // BQ_H
