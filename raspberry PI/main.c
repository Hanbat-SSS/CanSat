#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "driver_nrf24l01.h"

#define CE_PIN 25
#define CSN_PIN 8
#define SPI_CHANNEL 0
#define SPI_SPEED 100000

int main() {
    nrf24l01_init(CE_PIN, CSN_PIN, SPI_CHANNEL, SPI_SPEED);
    nrf24l01_set_tx_mode();

    char message[] = "Hello World!";
    for(int i = 0; i < 10; i++) {
        if(nrf24l01_send((uint8_t *)message, strlen(message)) != 0) {
	    fprintf(stderr, "NRF24L01 send error\n");
	    return 1;
        }
	printf("Message sent: %s\n", message);
	sleep(2);
    }
    return 0;
}
