#include "uart.h"
#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include <stdbool.h>
#include <stdint.h>

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
