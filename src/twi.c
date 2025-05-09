#include "twi.h"
#include "nrf52840.h"
#include "nrf52840_bitfields.h"
#include <stdbool.h>

void twi_wait_stop() {
  while (!NRF_TWIM0->EVENTS_STOPPED) {
  }
  NRF_TWIM0->EVENTS_STOPPED = 0;
}

void twi_tx(uint8_t *data, uint8_t len) {
  NRF_TWIM0->SHORTS = TWIM_SHORTS_LASTTX_STOP_Msk;
  NRF_TWIM0->TXD.PTR = (uint32_t)data;
  NRF_TWIM0->TXD.MAXCNT = len;
  NRF_TWIM0->TASKS_STARTTX = 1;
  twi_wait_stop();
}

void twi_rx(uint8_t *data, uint8_t len) {
  NRF_TWIM0->RXD.PTR = (uint32_t)data;
  NRF_TWIM0->RXD.MAXCNT = len;
  NRF_TWIM0->SHORTS = TWIM_SHORTS_LASTRX_STOP_Msk;
  NRF_TWIM0->TASKS_STARTRX = 1;
  twi_wait_stop();
}

void twi_write_register(uint8_t reg, uint8_t val) {
  uint8_t buf[] = {reg, val};
  NRF_TWIM0->ADDRESS = HDC2080_ADDRESS;
  twi_tx(buf, sizeof(buf));
}

uint8_t twi_read_register(uint8_t reg) {
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