#ifndef HDC2080_H
#define HDC2080_H

#include <stdint.h>

#define TEMP_LOW 0x00
#define TEMP_HIGH 0x01
#define MEASUREMENT_CONFIG 0x0F
#define INT_CONFIG 0x0E
#define INTERRUPT_DRDY 0x04

void trigger_measurement(void);
void reset();
void set_measurement_mode();
float read_temp(void);
uint8_t read_interrupt_status(void);

#endif