#include <stdio.h>
#include <string.h>
#include "driver_nrf24l01.h"

#define CE_PIN  22
#define CSN_PIN 8
#define SPI_CHANNEL 0
#define SPI_SPEED  1000000

int main() {
    nrf24l01_init(CE_PIN, CSN_PIN, SPI_CHANNEL, SPI_SPEED);
    nrf24l01_set_rx_mode();

    uint8_t buffer[32];
    uint8_t len = sizeof(buffer);

    while (1) {
        int i;
        if (nrf24l01_receive(buffer, &len) == 0) {
            buffer[len] = '\0'; // 문자열 종료
            printf("Message received %d: %s\n", i+1, buffer);
        } else {
            printf("Receive error at iteration %d\n", i+1);
        }
    }

    return 0;
}
