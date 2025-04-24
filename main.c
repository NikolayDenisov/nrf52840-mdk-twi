#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include <stdint.h>

#define HDC2080_ADDR (0x40)

#define TWI_SCL_PIN 27
#define TWI_SDA_PIN 26

void twi_init(void) {
  NRF_TWI0->PSEL.SCL = TWI_SCL_PIN;
  NRF_TWI0->PSEL.SDA = TWI_SDA_PIN;

  NRF_TWI0->FREQUENCY = TWI_FREQUENCY_FREQUENCY_K100
                        << TWI_FREQUENCY_FREQUENCY_Pos;

  NRF_TWI0->ENABLE = TWI_ENABLE_ENABLE_Enabled << TWI_ENABLE_ENABLE_Pos;
}

void twi_write(uint8_t address, uint8_t reg) {
  NRF_TWI0->ADDRESS = address;
  NRF_TWI0->TXD = reg;
  NRF_TWI0->TASKS_STARTTX = 1;
  while (!NRF_TWI0->EVENTS_TXDSENT) {
  }
  NRF_TWI0->EVENTS_TXDSENT = 0;

  NRF_TWI0->TASKS_STOP = 1;
  while (NRF_TWI0->EVENTS_STOPPED == 0) {
  }
  NRF_TWI0->EVENTS_STOPPED = 0;
}

uint8_t twi_read(uint8_t address, uint8_t reg) {
  uint8_t data;

  NRF_TWI0->ADDRESS = address;
  NRF_TWI0->TXD = reg;
  NRF_TWI0->TASKS_STARTTX = 1;
  while (!NRF_TWI0->EVENTS_TXDSENT) {
  }
  NRF_TWI0->EVENTS_TXDSENT = 0;

  NRF_TWI0->TASKS_STARTRX = 1;
  while (!NRF_TWI0->EVENTS_RXDREADY) {
  }
  data = NRF_TWI0->RXD;
  NRF_TWI0->EVENTS_RXDREADY = 0;

  NRF_TWI0->TASKS_STOP = 1;
  while (NRF_TWI0->EVENTS_STOPPED == 0) {
  }
  NRF_TWI0->EVENTS_STOPPED = 0;

  return data;
}

void clock_initialization() {
  /* Start 16 MHz crystal oscillator */
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_HFCLKSTART = 1;

  /* Wait for the external oscillator to start up */
  while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) {
    // Do nothing.
  }

  /* Start low frequency crystal oscillator for app_timer(used by bsp)*/
  NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_LFCLKSTART = 1;

  while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0) {
    // Do nothing.
  }
}

int main(void) {
  clock_initialization();
  twi_init();

  while (1) {
    uint8_t temp_low = twi_read(HDC2080_ADDR, 0x00);  // TEMP_LOW
    uint8_t temp_high = twi_read(HDC2080_ADDR, 0x01); // TEMP_HIGH
    for (int i = 0; i < 1000000; i++) {
      __asm("nop");
    }
  }
}
