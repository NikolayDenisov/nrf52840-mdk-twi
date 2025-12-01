#include "hdc2080.h"
#include "nrf.h"
#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include "nrf_delay.h"
#include "twi.h"
#include "uart.h"

#define HDC2080_INT_PIN 13

void gpio_config_for_interrupt(void) {
  NRF_P1->PIN_CNF[HDC2080_INT_PIN] =
      (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
      (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
      (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos);

  NRF_GPIOTE->CONFIG[0] =
      (HDC2080_INT_PIN << GPIOTE_CONFIG_PSEL_Pos) |
      (1 << GPIOTE_CONFIG_PORT_Pos) |
      (GPIOTE_CONFIG_MODE_Event << GPIOTE_CONFIG_MODE_Pos) |
      (GPIOTE_CONFIG_POLARITY_LoToHi << GPIOTE_CONFIG_POLARITY_Pos);

  NRF_GPIOTE->EVENTS_IN[0] = 0; // Clear event
  NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Msk;
  NVIC_EnableIRQ(GPIOTE_IRQn); // Enable GPIOTE interrupt
}

void GPIOTE_IRQHandler(void) {
  if (NRF_GPIOTE->EVENTS_IN[0]) {
    NRF_GPIOTE->EVENTS_IN[0] = 0; // Clear event
    uint8_t status;
    status = hdc2080_read_interrupt_status();
    static float temperature = 0;
    static float humidity = 0;
    temperature = read_temp();
    uart_send_float(temperature);
    humidity = hdc2080_read_humidity_2();
    uart_send_float(humidity);
    uart_send_string("\r\n");
  }
}

void sensor_init(void) {
  // Инициализация датчика HDC2080
  reset();
  set_rate(FIVE_SECONDS);
  enable_drdy_interrupt();
  enable_interrupt();
  set_interrupt_polarity(ACTIVE_HIGH);
  set_measurement_mode();
}

int main(void) {
  uart_init();
  uart_send_string("HDC2080 Example!\r\n");
  twi_init();
  sensor_init();
  gpio_config_for_interrupt();
  trigger_measurement();

  while (true) {
    __WFE(); // Wait for interrupt
  }
}
