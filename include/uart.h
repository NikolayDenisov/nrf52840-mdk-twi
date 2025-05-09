#ifndef UART_H
#define UART_H

#define UART_TX_PIN 20

void uart_init(void);
void uart_send_char(char c);
void uart_send_string(const char *str);
void uart_send_int(int value);
void uart_send_float(float val);

#endif