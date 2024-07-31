#ifndef DRIVER_NRF24L01_H
#define DRIVER_NRF24L01_H

#include <stdint.h>

void nrf24l01_init(int ce_pin, int csn_pin, int spi_channel, int spi_speed);
void nrf24l01_set_tx_mode(void);
void nrf24l01_set_rx_mode(void);
int nrf24l01_send(uint8_t *data, uint8_t len);
int nrf24l01_receive(uint8_t *data, uint8_t *len);

#endif // DRIVER_NRF24L01_H
