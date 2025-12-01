#include "hdc2080.h"
#include "nrf_delay.h"
#include "twi.h"

void trigger_measurement(void) {
  uint8_t config_content = twi_read_register(MEASUREMENT_CONFIG);
  config_content |= 0x01;
  twi_write_register(MEASUREMENT_CONFIG, config_content);
}

void reset() {
  twi_write_register(INT_CONFIG, 0x80);
  nrf_delay_ms(50);
}

void set_measurement_mode() {
  twi_write_register(MEASUREMENT_CONFIG, 0x02);
  nrf_delay_ms(130);
}

float read_temp(void) {
  uint8_t byte[2];
  uint16_t temp;

  byte[0] = twi_read_register(TEMP_LOW);
  byte[1] = twi_read_register(TEMP_HIGH);

  temp = ((uint16_t)byte[1] << 8) | byte[0];

  return ((float)temp) * 165.0f / 65536.0f - 40.5f;
}

uint8_t read_interrupt_status(void) {
  uint8_t reg_contents;
  reg_contents = twi_read_register(INTERRUPT_DRDY);
  return reg_contents;
}

/*Bit 2 of the INT_CONFIG register can be used to enable/disable
        the interrupt pin  */
void enable_interrupt(void) {
  uint8_t config_contents;
  config_contents = twi_read_register(INT_CONFIG);
  config_contents = (config_contents | 0x04);
  twi_write_register(INT_CONFIG, config_contents);
}

void hdc2080_set_measurement_mode() {
  twi_write_register(MEASUREMENT_CONFIG, 0xF9);
  nrf_delay_ms(130);
}

float hd2080_read_temp(void) {
  uint8_t byte[2];
  uint16_t temp;

  byte[0] = twi_read_register(TEMP_LOW);
  byte[1] = twi_read_register(TEMP_HIGH);

  temp = ((uint16_t)byte[1] << 8) | byte[0];

  return ((float)temp) * 165.0f / 65536.0f - 40.5f;
}

float hdc2080_read_humidity(void) {
  uint8_t byte[2];
  uint16_t humidity;
  byte[0] = twi_read_register(HUMID_LOW);
  byte[1] = twi_read_register(HUMID_HIGH);
  humidity = (unsigned int)byte[1] << 8 | byte[0];
  return (float)(humidity) / 65536 * 100;
}

uint8_t hdc2080_read_interrupt_status(void) {
  uint8_t reg_contents;
  reg_contents = twi_read_register(INTERRUPT_DRDY);
  return reg_contents;
}

void enable_drdy_interrupt(void) {
  uint8_t reg_contents;
  reg_contents = twi_read_register(INTERRUPT_CONFIG);
  reg_contents = (reg_contents | 0x80);
  twi_write_register(INTERRUPT_CONFIG, reg_contents);
}

/*Bit 1 of the INT_CONFIG register can be used to control the
        the interrupt pins polarity */
void set_interrupt_polarity(int polarity) {
  uint8_t config_contents;
  config_contents = twi_read_register(INT_CONFIG);
  switch (polarity) {
  case ACTIVE_LOW:
    config_contents = (config_contents & 0xFD);
    break;
  case ACTIVE_HIGH:
    config_contents = (config_contents | 0x02);
    break;
  default:
    config_contents = (config_contents & 0xFD);
  }
  twi_write_register(INT_CONFIG, config_contents);
}
/* Bit 0 of the INT_CONFIG register can be used to control the
   interrupt pin's mode */
void set_interrupt_mode(int mode) {
  uint8_t config_contents;
  config_contents = twi_read_register(INT_CONFIG);
  switch (mode) {
  case LEVEL_MODE:
    config_contents = (config_contents & 0xFE);
    break;
  case COMPARATOR_MODE:
    config_contents = (config_contents | 0x01);
    break;
  default:
    config_contents = (config_contents & 0xFE);
  }
  twi_write_register(INT_CONFIG, config_contents);
}

/*Bits 6-4  of the INT_CONFIG register controls the measurement
        rate  */
void set_rate(int rate) {
  uint8_t config_contents;
  config_contents = twi_read_register(INT_CONFIG);
  switch (rate) {
  case MANUAL:
    config_contents = (config_contents & 0x8F);
    break;
  case TWO_MINS:
    config_contents = (config_contents & 0x9F);
    config_contents = (config_contents | 0x10);
    break;
  case ONE_MINS:
    config_contents = (config_contents & 0xAF);
    config_contents = (config_contents | 0x20);
    break;
  case TEN_SECONDS:
    config_contents = (config_contents & 0xBF);
    config_contents = (config_contents | 0x30);
    break;
  case FIVE_SECONDS:
    config_contents = (config_contents & 0xCF);
    config_contents = (config_contents | 0x40);
    break;
  case ONE_HZ:
    config_contents = (config_contents & 0xDF);
    config_contents = (config_contents | 0x50);
    break;
  case TWO_HZ:
    config_contents = (config_contents & 0xEF);
    config_contents = (config_contents | 0x60);
    break;
  case FIVE_HZ:
    config_contents = (config_contents | 0x70);
    break;
  default:
    config_contents = (config_contents & 0x8F);
  }
  twi_write_register(INT_CONFIG, config_contents);
}

float hdc2080_read_humidity_2(void) {
  uint8_t buf[2];
  uint16_t raw;

  twi_read_registers(HUMID_LOW, buf, 2);

  raw = ((uint16_t)buf[1] << 8) | buf[0];
  return (float)raw / 65536.0f * 100.0f;
}