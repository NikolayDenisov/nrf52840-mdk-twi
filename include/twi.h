#ifndef TWI_H
#define TWI_H

#include <stdint.h>

#define TWI_SCL_PIN 6
#define TWI_SDA_PIN 8
#define HDC2080_ADDRESS 0x40

void twi_wait_stop();
void twi_tx(uint8_t *data, uint8_t len);
void twi_rx(uint8_t *data, uint8_t len);
void twi_write_register(uint8_t reg, uint8_t val);
uint8_t twi_read_register(uint8_t reg);
void twi_init(void);

#endif