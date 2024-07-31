#include <pigpio.h>
#include <stdio.h>
#include "driver_nrf24l01.h"

static int CE_PIN;
static int CSN_PIN;
static int SPI_CHANNEL;
static int SPI_SPEED;

void nrf24l01_init(int ce_pin, int csn_pin, int spi_channel, int spi_speed) {
    CE_PIN = ce_pin;
    CSN_PIN = csn_pin;
    SPI_CHANNEL = spi_channel;
    SPI_SPEED = spi_speed;

    if(gpioInitialise() < 0) {
        fprintf(stderr, "pigpio init failed\n");
        return;
    }
    gpioSetMode(CE_PIN, PI_OUTPUT);
    gpioSetMode(CSN_PIN, PI_OUTPUT);
    gpioWrite(CE_PIN, PI_LOW);
    gpioWrite(CSN_PIN, PI_HIGH);
    spiOpen(SPI_CHANNEL, SPI_SPEED, 0);
}

void nrf24l01_set_tx_mode(void) {
    // set trnsmmit mode
    gpioWrite(CE_PIN, PI_LOW);
    // set register mode(example)
    uint8_t config = 0x0E; // CONFIG 레지스터 설정
    gpioWrite(CSN_PIN, PI_LOW);
    spiWrite(SPI_CHANNEL, (char*)&config, 1);
    gpioWrite(CSN_PIN, PI_HIGH);
    gpioWrite(CE_PIN, PI_HIGH);
}

void nrf24l01_set_rx_mode(void) {
    // set receive mode
    gpioWrite(CE_PIN, PI_LOW);
    //set receive register mode
    uint8_t config = 0x0F;
    gpioWrite(CSN_PIN, PI_LOW);
    spiWrite(SPI_CHANNEL, (char*)&config, 1);
    gpioWrite(CSN_PIN, PI_HIGH);
    gpioWrite(CE_PIN, PI_HIGH);
}

int nrf24l01_receive(uint8_t *data, uint8_t *len) {
    gpioWrite(CSN_PIN, PI_LOW);
    // 데이터 수신 코드 추가
    uint8_t status = 0; // 수신 상태 확인
    spiRead(SPI_CHANNEL, (char*)&status, 1);

    if (status & 0x40) { // 데이터 수신 확인
        spiRead(SPI_CHANNEL, (char*)data, *len);
        gpioWrite(CSN_PIN, PI_HIGH);
        return 0; // 성공 시 0 반환
    }

    gpioWrite(CSN_PIN, PI_HIGH);
   return -1;
}

int nrf24l01_send(uint8_t *data, uint8_t len) {
    gpioWrite(CSN_PIN, PI_LOW);
    spiWrite(SPI_CHANNEL, (char*)data, len);
    gpioWrite(CSN_PIN, PI_HIGH);
    return 0; // 성공 시 0 반환
}
