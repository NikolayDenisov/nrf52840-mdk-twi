#include "hdc2080.h"
#include "nrf_delay.h"
#include "twi.h"
#include "uart.h"

int main(void) {
  uint8_t status_int_register;
  uint8_t data_ready = 0x80;

  uart_init();
  uart_send_string("HDC2080 Example!\r\n");
  twi_init();
  reset();
  set_measurement_mode();

  while (true) {
    trigger_measurement();

    status_int_register = read_interrupt_status();
    if (status_int_register != data_ready) {
      nrf_delay_ms(500);
      continue;
    }

    static float temperature = 0;
    temperature = read_temp();
    uart_send_float(temperature);
    nrf_delay_ms(1000);
  }
}
