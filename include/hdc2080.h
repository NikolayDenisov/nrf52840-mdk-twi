#ifndef HDC2080_H
#define HDC2080_H

#include <stdint.h>

#define TEMP_LOW 0x0
#define TEMP_HIGH 0x1
#define HUMID_LOW 0x02
#define HUMID_HIGH 0x03
#define INTERRUPT_DRDY 0x4
#define INTERRUPT_CONFIG 0x07
#define MEASUREMENT_CONFIG 0xF
#define INT_CONFIG 0xE

// Constants for setting sensor mode
#define TEMP_AND_HUMID 0
#define TEMP_ONLY 1
#define HUMID_ONLY 2
#define ACTIVE_LOW 0
#define ACTIVE_HIGH 1
#define LEVEL_MODE 0
#define COMPARATOR_MODE 1

// Constants for setting sample rate
#define MANUAL 0
#define TWO_MINS 1
#define ONE_MINS 2
#define TEN_SECONDS 3
#define FIVE_SECONDS 4
#define ONE_HZ 5
#define TWO_HZ 6
#define FIVE_HZ 7

// Constants for setting sample rate
#define MANUAL 0
#define TWO_MINS 1
#define ONE_MINS 2
#define TEN_SECONDS 3
#define FIVE_SECONDS 4
#define ONE_HZ 5
#define TWO_HZ 6
#define FIVE_HZ 7

void trigger_measurement(void);
void reset();
void set_measurement_mode();
float read_temp(void);
uint8_t read_interrupt_status(void);
void enable_interrupt(void);
void set_interrupt_polarity(int polarity);
void set_interrupt_mode(int mode);
void set_rate(int rate);
void enable_drdy_interrupt(void);

#endif