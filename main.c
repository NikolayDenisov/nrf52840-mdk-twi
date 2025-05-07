#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "nrf_delay.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h> // Для sprintf

#define UART_TX_PIN 20
#define HDC2080_ADDRESS 0x40
#define TWI_SCL_PIN 26
#define TWI_SDA_PIN 27
#define TEMP_LOW 0x00
#define TEMP_HIGH 0x01
#define MEASUREMENT_CONFIG 0x0F
#define INT_CONFIG 0x0E

void uart_init(void) {
  NRF_P0->PIN_CNF[UART_TX_PIN] = (1 << 0) | (1 << 1) | (0 << 2);

  NRF_UART0->PSEL.TXD = UART_TX_PIN;
  NRF_UART0->PSEL.RTS = 0xFFFFFFFF;
  NRF_UART0->PSEL.CTS = 0xFFFFFFFF;
  NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud115200;
  NRF_UART0->ENABLE = UART_ENABLE_ENABLE_Enabled;
  NRF_UART0->TASKS_STARTTX = 1;
}

void uart_send_char(char c) {
  NRF_UART0->TXD = c;
  while (NRF_UART0->EVENTS_TXDRDY == 0) {
  }
  NRF_UART0->EVENTS_TXDRDY = 0;
}

void uart_send_string(const char *str) {
  while (*str != '\0') {
    uart_send_char(*str);
    str++;
  }
}

static void twi_wait_stop() {
  while (!NRF_TWIM0->EVENTS_STOPPED) {
  }
  NRF_TWIM0->EVENTS_STOPPED = 0;
}

static void twi_tx(uint8_t *data, uint8_t len) {
  NRF_TWIM0->SHORTS = TWIM_SHORTS_LASTTX_STOP_Msk;
  NRF_TWIM0->TXD.PTR = (uint32_t)data;
  NRF_TWIM0->TXD.MAXCNT = len;
  NRF_TWIM0->TASKS_STARTTX = 1;
  twi_wait_stop();
}

static void twi_rx(uint8_t *data, uint8_t len) {
  NRF_TWIM0->RXD.PTR = (uint32_t)data;
  NRF_TWIM0->RXD.MAXCNT = len;
  NRF_TWIM0->SHORTS = TWIM_SHORTS_LASTRX_STOP_Msk;
  NRF_TWIM0->TASKS_STARTRX = 1;
  twi_wait_stop();
}

static void twi_write_register(uint8_t reg, uint8_t val) {
  uint8_t buf[] = {reg, val};
  NRF_TWIM0->ADDRESS = HDC2080_ADDRESS;
  twi_tx(buf, sizeof(buf));
}

static uint8_t twi_read_register(uint8_t reg) {
  uint8_t val;
  NRF_TWIM0->ADDRESS = HDC2080_ADDRESS;
  twi_tx(&reg, 1);
  twi_rx(&val, 1);
  return val;
}

void twi_init(void) {
  NRF_TWIM0->PSEL.SCL = TWI_SCL_PIN;
  NRF_TWIM0->PSEL.SDA = TWI_SDA_PIN;
  NRF_TWIM0->FREQUENCY = TWIM_FREQUENCY_FREQUENCY_K100;
  NRF_TWIM0->SHORTS = 0;
  NRF_TWIM0->ENABLE = TWIM_ENABLE_ENABLE_Enabled << TWIM_ENABLE_ENABLE_Pos;
}

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

void uart_send_int(int value) {
  char buf[12]; // Достаточно для int32
  int i = 0;
  if (value < 0) {
    uart_send_char('-');
    value = -value;
  }
  if (value == 0) {
    uart_send_char('0');
    return;
  }
  while (value > 0) {
    buf[i++] = (value % 10) + '0';
    value /= 10;
  }
  while (i > 0) {
    uart_send_char(buf[--i]);
  }
}

void uart_send_float(float val) {
  if (val < 0 && val > -1.0f) {
    uart_send_char('-'); // Отдельная проверка на -0.XX
  }
  int32_t int_part = (int32_t)val;
  int32_t frac_part =
      (int32_t)((val > 0 ? val - int_part : int_part - val) * 100 +
                0.5f); // округление
  uart_send_string("temperature: ");
  uart_send_int(int_part);
  uart_send_char('.');
  if (frac_part < 10) {
    uart_send_char('0'); // ведущий ноль, если например .04
  }
  uart_send_int(frac_part);
  uart_send_string(" C\r\n");
}

int main(void) {
  uart_init();
  uart_send_string("HDC2080 Example!\r\n");
  twi_init();
  reset();
  set_measurement_mode();
  uart_send_string("Success!\r\n");
  while (true) {
    trigger_measurement();
    nrf_delay_ms(1000);

    static float temperature = 0;
    temperature = read_temp();
    uart_send_float(temperature);
  }
}
