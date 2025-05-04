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

void twi_init(void) {
  NRF_TWIM0->PSEL.SCL = TWI_SCL_PIN;
  NRF_TWIM0->PSEL.SDA = TWI_SDA_PIN;

  NRF_TWIM0->FREQUENCY = TWIM_FREQUENCY_FREQUENCY_K100;
  NRF_TWIM0->SHORTS = 0;

  NRF_TWIM0->ENABLE = TWIM_ENABLE_ENABLE_Enabled << TWIM_ENABLE_ENABLE_Pos;
}

static void write_reg(uint8_t reg, uint8_t data) {
  uint8_t send[2] = {reg, data};

  NRF_TWIM0->ADDRESS = HDC2080_ADDRESS;
  NRF_TWIM0->TXD.PTR = (uint32_t)send;
  NRF_TWIM0->TXD.MAXCNT = 2;
  NRF_TWIM0->SHORTS = TWIM_SHORTS_LASTTX_STOP_Msk;
  NRF_TWIM0->EVENTS_STOPPED = 0;
  NRF_TWIM0->TASKS_STARTTX = 1;
  while (NRF_TWIM0->EVENTS_STOPPED == 0)
    ;
}

static uint8_t read_reg(uint8_t reg) {
  uint8_t tx_buf[1];
  uint8_t rx_buf[1];

  NRF_TWIM0->SHORTS =
      TWIM_SHORTS_LASTTX_STARTRX_Msk | TWIM_SHORTS_LASTRX_STOP_Msk;
  NRF_TWIM0->ADDRESS = HDC2080_ADDRESS;

  tx_buf[0] = reg;
  NRF_TWIM0->TXD.MAXCNT = sizeof(tx_buf);
  NRF_TWIM0->TXD.PTR = (uint32_t)&tx_buf[0];

  NRF_TWIM0->RXD.PTR = (uint32_t)&rx_buf[0];
  NRF_TWIM0->RXD.MAXCNT = 1;

  NRF_TWIM0->EVENTS_STOPPED = 0;
  NRF_TWIM0->TASKS_STARTRX = 1;

  while (NRF_TWIM0->EVENTS_STOPPED == 0)
    ;

  return rx_buf[0];
}

void twi_write(uint8_t address, uint8_t reg) {
  NRF_TWI0->ADDRESS = address;

  NRF_TWI0->TXD = reg;
  NRF_TWI0->TASKS_STARTTX = 1;
  while (!NRF_TWI0->EVENTS_TXDSENT)
    ;
  NRF_TWI0->EVENTS_TXDSENT = 0;

  NRF_TWI0->TASKS_STOP = 1;
  while (!NRF_TWI0->EVENTS_STOPPED)
    ;
  NRF_TWI0->EVENTS_STOPPED = 0;
}

uint8_t twi_read_byte(uint8_t address) {
  NRF_TWI0->ADDRESS = address;

  NRF_TWI0->TASKS_STARTRX = 1;
  while (!NRF_TWI0->EVENTS_RXDREADY)
    ;
  uint8_t data = NRF_TWI0->RXD;
  NRF_TWI0->EVENTS_RXDREADY = 0;

  NRF_TWI0->TASKS_STOP = 1;
  while (!NRF_TWI0->EVENTS_STOPPED)
    ;
  NRF_TWI0->EVENTS_STOPPED = 0;

  return data;
}

float read_temp(void) {
  uint8_t byte[2];
  uint16_t temp;

  byte[0] = read_reg(TEMP_LOW);
  byte[1] = read_reg(TEMP_HIGH);

  temp = ((uint16_t)byte[1] << 8) | byte[0];

  return ((float)temp) * 165.0f / 65536.0f - 40.5f;
}

void int_to_ascii(int16_t value, char *buf) {
  int16_t int_part = value / 100;
  int16_t frac_part = value % 100;
  if (frac_part < 0)
    frac_part = -frac_part;
  if (int_part < 0) {
    *buf++ = '-';
    int_part = -int_part;
  }
  sprintf(buf, "%d.%02d C\r\n", int_part, frac_part);
}

void trigger_measurement(void) {
  static uint8_t measurement_configuration_register = 0x0F;

  NRF_TWIM0->ADDRESS = 0x40;
  NRF_TWIM0->TXD.PTR = (uint32_t)&measurement_configuration_register;
  NRF_TWIM0->TXD.MAXCNT = 1;
  NRF_TWIM0->SHORTS = TWIM_SHORTS_LASTTX_STOP_Msk;
  NRF_TWIM0->TASKS_STARTTX = 1;
  while (!NRF_TWIM0->EVENTS_STOPPED) {
  }
  NRF_TWIM0->EVENTS_STOPPED = 0;

  uint8_t reg_value = 0;
  NRF_TWIM0->ADDRESS = 0x40;
  NRF_TWIM0->RXD.PTR = (uint32_t)&reg_value;
  NRF_TWIM0->RXD.MAXCNT = 1;
  NRF_TWIM0->SHORTS = TWIM_SHORTS_LASTRX_STOP_Msk;
  NRF_TWIM0->TASKS_STARTRX = 1;

  while (!NRF_TWIM0->EVENTS_STOPPED) {
  }
  NRF_TWIM0->EVENTS_STOPPED = 0;
}

void reset() {
  uart_send_string("Reset start!\r\n");
  uint8_t reset_buffer[] = {0x0E, 0x80};

  NRF_TWIM0->SHORTS = TWIM_SHORTS_LASTTX_STOP_Msk;

  NRF_TWIM0->ADDRESS = HDC2080_ADDRESS;
  NRF_TWIM0->TXD.PTR = (uint32_t)&reset_buffer;
  NRF_TWIM0->TXD.MAXCNT = sizeof(reset_buffer);

  // Запускаем передачу
  NRF_TWIM0->TASKS_STARTTX = 1;

  // Ожидаем завершения
  while (!NRF_TWIM0->EVENTS_STOPPED) {
  }
  NRF_TWIM0->EVENTS_STOPPED = 0;
  nrf_delay_ms(50);
  uart_send_string("Reset finish!\r\n");
}

void set_measurement_mode() {
  uint8_t measurement_buffer[] = {0x0F, 0x02};

  uart_send_string("Configuration start!\r\n");
  NRF_TWIM0->SHORTS = TWIM_SHORTS_LASTTX_STOP_Msk;

  NRF_TWIM0->ADDRESS = HDC2080_ADDRESS;
  NRF_TWIM0->TXD.PTR = (uint32_t)&measurement_buffer;
  NRF_TWIM0->TXD.MAXCNT = sizeof(measurement_buffer);

  // Запускаем передачу
  NRF_TWIM0->TASKS_STARTTX = 1;

  // Ожидаем завершения
  while (!NRF_TWIM0->EVENTS_STOPPED) {
  }
  NRF_TWIM0->EVENTS_STOPPED = 0;
  nrf_delay_ms(130);
  uart_send_string("Configuration finish!\r\n");
}

int main(void) {
  uart_init();
  uart_send_string("HDC2080 Example!\r\n");
  twi_init();
  // reset
  reset();
  set_measurement_mode();
  while (true) {
    uart_send_string("Success!\r\n");
    nrf_delay_ms(1000);
    trigger_measurement();
    float temperature = 0;
    // temperature = read_temp();

    // char buf[32];
    // int_to_ascii(temperature, buf);
    // uart_send_string(buf);
  }
}
