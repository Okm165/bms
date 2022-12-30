#include "interface.h"
#include "main.h"
#include "registers.h"
#include <string.h>

#define MAX_DELAY 100

int adc_gain;   // factory-calibrated, read out from chip (uV/LSB)
int adc_offset; // factory-calibrated, read out from chip (mV)

static int bq_address = 0x08;

void bq769x0_write_byte(uint8_t reg_addr, uint8_t data) {
  uint8_t buf[2] = {reg_addr, data};
  int temp_address = bq_address << 1;
  HAL_I2C_Master_Transmit(&hi2c1, temp_address, buf, 2, MAX_DELAY);
}

uint8_t bq769x0_read_byte(uint8_t reg_addr) {
  uint8_t buf[1];
  int temp_address = bq_address << 1;
  HAL_I2C_Master_Transmit(&hi2c1, temp_address, &reg_addr, 1, MAX_DELAY);
  HAL_I2C_Master_Receive(&hi2c1, temp_address, buf, 1, MAX_DELAY);
  return buf[0];
}

int32_t bq769x0_read_word(uint8_t reg_addr) {
  uint8_t buf[2];
  int temp_address = bq_address << 1;
  HAL_I2C_Master_Transmit(&hi2c1, temp_address, &reg_addr, 1, MAX_DELAY);
  HAL_I2C_Master_Receive(&hi2c1, temp_address, buf, 2, MAX_DELAY);
  return buf[0] << 8 | buf[1];
}

// automatically find out address
int determine_address(void) {
  bq_address = 0x08;
  bq769x0_write_byte(BQ769X0_CC_CFG, 0x19);
  printf("%d\n", bq769x0_read_byte(BQ769X0_CC_CFG));
  if (bq769x0_read_byte(BQ769X0_CC_CFG) == 0x19) {
    return 0;
  }

  bq_address = 0x18;
  bq769x0_write_byte(BQ769X0_CC_CFG, 0x19);
  printf("%d\n", bq769x0_read_byte(BQ769X0_CC_CFG));
  if (bq769x0_read_byte(BQ769X0_CC_CFG) == 0x19) {
    return 0;
  }

  return -1;
}

int bq769x0_init() {
  int err = determine_address();
  if (!err) {
    // initial settings for bq769x0
    bq769x0_write_byte(BQ769X0_SYS_CTRL1,
                       0b00011000); // switch external thermistor and ADC on
    bq769x0_write_byte(BQ769X0_SYS_CTRL2, 0b01000000); // switch CC_EN on

    // get ADC offset and gain
    adc_offset =
        (signed int)bq769x0_read_byte(BQ769X0_ADCOFFSET); // 2's complement
    adc_gain =
        365 +
        (((bq769x0_read_byte(BQ769X0_ADCGAIN1) & 0b00001100) << 1) |
         ((bq769x0_read_byte(BQ769X0_ADCGAIN2) & 0b11100000) >> 5)); // uV/LSB
  } else {
    printf("BMS communication error\n");
    return err;
  }

  return 0;
}
