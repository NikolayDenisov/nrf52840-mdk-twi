#include "hdc2080.h"
#include "nrf_delay.h"
#include "twi.h"
#include "uart.h"

void trigger_measurement(void) {
  uint8_t config_content = twi_read_register(MEASUREMENT_CONFIG);
  config_content |= 0x01;
  twi_write_register(MEASUREMENT_CONFIG, config_content);
}

void reset() {
  uart_send_string("Reset start!\r\n");
  twi_write_register(INT_CONFIG, 0x80);
  nrf_delay_ms(50);
  uart_send_string("Reset finish!\r\n");
}

void set_measurement_mode() {
  uart_send_string("Configuration start!\r\n");
  twi_write_register(MEASUREMENT_CONFIG, 0x02);
  nrf_delay_ms(130);
  uart_send_string("Configuration finish!\r\n");
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