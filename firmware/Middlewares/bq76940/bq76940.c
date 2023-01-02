#include "bq76940.h"
#include "main.h"
#include "registers.h"

#include <math.h> // log for thermistor calculation
#include <stdio.h>
#include <stdlib.h> // for abs() function
#include <string.h>

#define BQ_I2C_MAX_DELAY 100

/* Private variables -----------------------------------------------*/

I2C_HandleTypeDef *I2CHandle = &hi2c1;

static int adc_gain = 0;   // factory-calibrated, read out from chip (uV/LSB)
static int adc_offset = 0; // factory-calibrated, read out from chip (mV)
static int bq_address = 0x08;

/* Function definitions -----------------------------------------------*/

/**
 * Writes one byte to bq769x0 IC
 */
static void bq769x0_write_byte(uint8_t reg_addr, uint8_t data) {
  uint8_t buf[2] = {reg_addr, data};
  int temp_address = bq_address << 1;
  HAL_I2C_Master_Transmit(I2CHandle, temp_address, buf, 2, BQ_I2C_MAX_DELAY);
}

/**
 * Reads one byte from bq769x0 IC
 */
static uint8_t bq769x0_read_byte(uint8_t reg_addr) {
  uint8_t buf[1];
  int temp_address = bq_address << 1;
  HAL_I2C_Master_Transmit(I2CHandle, temp_address, &reg_addr, 1,
                          BQ_I2C_MAX_DELAY);
  HAL_I2C_Master_Receive(I2CHandle, temp_address, buf, 1, BQ_I2C_MAX_DELAY);
  return buf[0];
}

/**
 * Read 16-bit word (two bytes) from bq769x0 IC
 *
 * @returns the (unsigned) word or -1 in case of CRC error
 */
static int32_t bq769x0_read_word(uint8_t reg_addr) {
  uint8_t buf[2];
  int temp_address = bq_address << 1;
  HAL_I2C_Master_Transmit(I2CHandle, temp_address, &reg_addr, 1,
                          BQ_I2C_MAX_DELAY);
  HAL_I2C_Master_Receive(I2CHandle, temp_address, buf, 2, BQ_I2C_MAX_DELAY);
  return buf[0] << 8 | buf[1];
}

/**
 * Automatically find out address of bq chip
 */
static bool bq769x0_determine_address(void) {
  bq_address = 0x08;
  bq769x0_write_byte(BQ769X0_CC_CFG, 0x19);
  if (bq769x0_read_byte(BQ769X0_CC_CFG) == 0x19) {
    return true;
  }

  bq_address = 0x18;
  bq769x0_write_byte(BQ769X0_CC_CFG, 0x19);
  if (bq769x0_read_byte(BQ769X0_CC_CFG) == 0x19) {
    return true;
  }

  return false;
}

/**
 * Temperature calculation using Beta equation
 * According to bq769x0 datasheet, only 10k thermistors should be used
 * 25°C reference temperature for Beta equation assumed
 */
static double bq769x0_thermistor_temp(double resistance, double beta,
                                      double nominal_resistance) {
  return 1.0 / (1.0 / (273.15 + 25.0) +
                log(resistance / nominal_resistance) / beta); // K
}

static int bq769x0_read_factory_adcgain(void) {
  return 365 +
         (((bq769x0_read_byte(BQ769X0_ADCGAIN1) & 0b00001100) << 1) |
          ((bq769x0_read_byte(BQ769X0_ADCGAIN2) & 0b11100000) >> 5)); // uV/LSB
}

static int bq769x0_read_factory_offset(void) {
  return (signed int)bq769x0_read_byte(BQ769X0_ADCOFFSET); // 2's complement
}

double bq769x0_change_temperature_source(enum TS_SRC src) {
  SYS_CTRL1_Type sys_ctrl1;
  sys_ctrl1.byte = bq769x0_read_byte(BQ769X0_SYS_CTRL1);
  sys_ctrl1.TEMP_SEL = src;
  bq769x0_write_byte(BQ769X0_SYS_CTRL1, sys_ctrl1.byte);
  return true;
}

double bq769x0_read_temperature_internal(enum TS_INT_Id id) {
  // When switching between external and internal temperature monitoring, a 2-s
  // latency may be incurred due to the natural scheduler update interval.
  int adc_raw =
      (bq769x0_read_byte(id) & 0b00111111) << 8 | bq769x0_read_byte(id + 1);
  return (25.0 - ((((double)adc_raw * 382.0 * 1e-6F) - 1.200) / 0.0042)) +
         273.15;
}

double bq769x0_read_temperature_external(enum TS_EXT_Id id,
                                         double thermistor_beta,
                                         double thermistor_nominal_resistance) {
  // When switching between external and internal temperature monitoring, a 2-s
  // latency may be incurred due to the natural scheduler update interval.
  // calculate R_thermistor according to bq769x0 datasheet
  int adc_raw =
      (bq769x0_read_byte(id) & 0b00111111) << 8 | bq769x0_read_byte(id + 1);
  double vtsx = (double)adc_raw * 382.0 * 1e-3F; // mV
  double rts = 10000.0 * vtsx / (3300.0 - vtsx); // Ohm
  return bq769x0_thermistor_temp(rts, thermistor_beta,
                                 thermistor_nominal_resistance);
}

bool bq769x0_read_current(double *milivolts) {
  SYS_STAT_Type sys_stat;
  sys_stat.byte = bq769x0_read_byte(BQ769X0_SYS_STAT);

  // check if new current reading available
  if (sys_stat.CC_READY == 1) {
    int adc_raw = bq769x0_read_word(BQ769X0_CC_HI_BYTE);
    *milivolts = (double)adc_raw * 8.44 * 1e-3F; // mV
    bq769x0_write_byte(BQ769X0_SYS_STAT,
                       BQ769X0_SYS_STAT_CC_READY); // clear CC ready flag
    return true;
  }
  return false;
}

double bq769x0_read_cell_voltage(enum CellId id) {
  int adc_raw =
      (bq769x0_read_byte(id) & 0b00111111) << 8 | bq769x0_read_byte(id + 1);
  return ((double)adc_raw * (double)adc_gain * 1e-3F + (double)adc_offset) *
         1e-3F;
}

double bq769x0_read_pack_voltage(int num_active_cells) {
  // read battery pack voltage
  int adc_raw = bq769x0_read_word(BQ769X0_BAT_HI_BYTE);
  return (4.0F * (double)adc_gain * (double)adc_raw * 1e-3F +
          (double)num_active_cells * (double)adc_offset) *
         1e-3F;
}

bool bq_chg_switch(bool enable) {
  SYS_CTRL2_Type sys_ctrl2;

  sys_ctrl2.byte = bq769x0_read_byte(BQ769X0_SYS_CTRL2);
  sys_ctrl2.CHG_ON = (uint8_t)enable;
  bq769x0_write_byte(BQ769X0_SYS_CTRL2, sys_ctrl2.byte);

  // check
  sys_ctrl2.byte = bq769x0_read_byte(BQ769X0_SYS_CTRL2);
  return sys_ctrl2.CHG_ON == (uint8_t)enable;
}

bool bq_dis_switch(bool enable) {
  SYS_CTRL2_Type sys_ctrl2;

  sys_ctrl2.byte = bq769x0_read_byte(BQ769X0_SYS_CTRL2);
  sys_ctrl2.DSG_ON = (uint8_t)enable;
  bq769x0_write_byte(BQ769X0_SYS_CTRL2, sys_ctrl2.byte);

  // check
  sys_ctrl2.byte = bq769x0_read_byte(BQ769X0_SYS_CTRL2);
  return sys_ctrl2.DSG_ON == (uint8_t)enable;
}

bool bq_set_dis_scp(enum SCD_T threshold, enum SCD_D delay) {
  PROTECT1_Type protect1;

  // only RSNS = 1 considered
  protect1.RSNS = 1;

  // Short circuit in discharge threshold setting
  protect1.SCD_THRESH = threshold;

  // Short circuit in discharge delay setting
  protect1.SCD_DELAY = delay;

  bq769x0_write_byte(BQ769X0_PROTECT1, protect1.byte);

  // check
  protect1.byte = bq769x0_read_byte(BQ769X0_PROTECT1);
  return protect1.RSNS == 1 && protect1.SCD_THRESH == threshold &&
         protect1.SCD_DELAY == delay;
}

bool bq_set_dis_ocp(enum OCD_T threshold, enum OCD_D delay) {
  PROTECT2_Type protect2;

  // Remark: RSNS must be set to 1 in PROTECT1 register
  // Overcurrent in discharge threshold setting.
  protect2.OCD_THRESH = threshold;

  // Overcurrent in discharge delay setting
  protect2.OCD_DELAY = delay;

  bq769x0_write_byte(BQ769X0_PROTECT2, protect2.byte);

  // check
  protect2.byte = bq769x0_read_byte(BQ769X0_PROTECT2);
  return protect2.OCD_THRESH == threshold && protect2.OCD_DELAY == delay;
}

bool bq_set_uvp(double uv_threshold, enum UV_D uv_delay) {
  PROTECT3_Type protect3;
  protect3.byte = bq769x0_read_byte(BQ769X0_PROTECT3);

  int uv_trip = (((long)((uv_threshold * 1000.0 - (double)adc_offset) * 1000.0 /
                         (double)adc_gain)) >>
                 4) &
                0x00FF;
  // Undervoltage delay setting
  protect3.UV_DELAY = uv_delay;

  bq769x0_write_byte(BQ769X0_UV_TRIP, uv_trip);
  bq769x0_write_byte(BQ769X0_PROTECT3, protect3.byte);

  // check
  uint8_t uv_trip_check = bq769x0_read_byte(BQ769X0_UV_TRIP);
  protect3.byte = bq769x0_read_byte(BQ769X0_PROTECT3);
  return uv_trip_check == uv_trip && protect3.UV_DELAY == uv_delay;
}

bool bq_set_ovp(double ov_threshold, enum OV_D ov_delay) {
  PROTECT3_Type protect3;
  protect3.byte = bq769x0_read_byte(BQ769X0_PROTECT3);

  int ov_trip = (((long)(((double)ov_threshold * 1000.0 - (double)adc_offset) *
                         1000.0 / (double)adc_gain)) >>
                 4) &
                0x00FF;
  // Overvoltage delay setting
  protect3.OV_DELAY = ov_delay;

  bq769x0_write_byte(BQ769X0_OV_TRIP, ov_trip);
  bq769x0_write_byte(BQ769X0_PROTECT3, protect3.byte);

  // check
  int ov_trip_check = bq769x0_read_byte(BQ769X0_OV_TRIP);
  protect3.byte = bq769x0_read_byte(BQ769X0_PROTECT3);
  return ov_trip_check == ov_trip && protect3.OV_DELAY == ov_delay;
}

SYS_STAT_Type bq769x0_system_faults() {
  SYS_STAT_Type sys_stat;
  sys_stat.byte =
      bq769x0_read_byte(BQ769X0_SYS_STAT) & BQ769X0_SYS_STAT_ERROR_MASK;
  return sys_stat;
}

void bq769x0_system_clear_faults(SYS_STAT_Type faults) {
  bq769x0_write_byte(BQ769X0_SYS_STAT,
                     faults.byte & BQ769X0_SYS_STAT_ERROR_MASK);
}

bool bq769x0_set_cell_balancing(uint16_t balancingFlags) {
  // check if attempting to balance adjacent cells
  bool adjacentCellCollision = (balancingFlags << 1) & balancingFlags;
  if (adjacentCellCollision) {
    return false;
  }

  uint8_t cellball1 = balancingFlags & 0b00011111;
  uint8_t cellball2 = (balancingFlags >> 5) & 0b00011111;
  uint8_t cellball3 = (balancingFlags >> 10) & 0b00011111;

  bq769x0_write_byte(BQ769X0_CELLBAL1, cellball1);
  bq769x0_write_byte(BQ769X0_CELLBAL2, cellball2);
  bq769x0_write_byte(BQ769X0_CELLBAL3, cellball3);

  // check
  uint8_t cellball1_check = bq769x0_read_byte(BQ769X0_CELLBAL1) & 0b00011111;
  uint8_t cellball2_check = bq769x0_read_byte(BQ769X0_CELLBAL2) & 0b00011111;
  uint8_t cellball3_check = bq769x0_read_byte(BQ769X0_CELLBAL3) & 0b00011111;

  return cellball1_check == cellball1 && cellball2_check == cellball2 &&
         cellball3_check == cellball3;
}

int bq769x0_init(BQConfig *bq_conf) {
  if (bq769x0_determine_address()) {
    // initial settings for bq769x0

    // get ADC offset and gain
    adc_offset = bq769x0_read_factory_offset();
    adc_gain = bq769x0_read_factory_adcgain();

    // switch external thermistor and ADC on
    bq769x0_write_byte(BQ769X0_SYS_CTRL1, 0b00011000);
    // switch CC_EN on
    bq769x0_write_byte(BQ769X0_SYS_CTRL2, 0b01000000);

    // check
    if (bq769x0_read_byte(BQ769X0_SYS_CTRL1) != 0b00011000 ||
        bq769x0_read_byte(BQ769X0_CC_CFG) != 0x19 ||
        bq769x0_read_byte(BQ769X0_SYS_CTRL2) != 0b01000000) {
      LogError("primary settings failed");
      return -1;
    }
    if (!bq769x0_change_temperature_source(bq_conf->temp_source)) {
      LogError("bq769x0_change_temperature_source failed");
      return -1;
    }
    // protection init
    if (!bq_set_dis_scp(bq_conf->scp_threshold, bq_conf->scp_delay)) {
      LogError("bq_set_dis_scp failed");
      return -1;
    }
    if (!bq_set_dis_ocp(bq_conf->ocp_threshold, bq_conf->ocp_delay)) {
      LogError("bq_set_dis_ocp failed");
      return -1;
    }
    if (!bq_set_uvp(bq_conf->uv_threshold, bq_conf->uv_delay)) {
      LogError("bq_set_uvp failed");
      return -1;
    }
    if (!bq_set_ovp(bq_conf->ov_threshold, bq_conf->ov_delay)) {
      LogError("bq_set_ovp failed");
      return -1;
    }
    LogDebug("Success");
    return 0;
  } else {
    LogDebug("BMS communication error couldn't determine address");
    return -1;
  }
}

void bq_shutdown() {
  // puts BMS IC into SHIP mode (i.e. switched off)
  bq769x0_write_byte(BQ769X0_SYS_CTRL1, 0x0);
  bq769x0_write_byte(BQ769X0_SYS_CTRL1, 0x1);
  bq769x0_write_byte(BQ769X0_SYS_CTRL1, 0x2);
}