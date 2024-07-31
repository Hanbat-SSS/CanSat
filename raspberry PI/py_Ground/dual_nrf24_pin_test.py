import time
from pyrf24 import RF24, RF24_PA_LOW, RF24_DRIVER

# Set radio_1 CE, CSN PIN
CSN_PIN_1 = 0 #SPI0 CE0 -> spidev 0.0
if RF24_DRIVER == 'MRAA':
    CE_PIN_1 = 15
elif RF24_DRIVER == 'wiringPi':
    CE_PIN_1 = 3
else:
    CE_PIN_1 = 22

radio_1 = RF24(CE_PIN_1, CSN_PIN_1)

# Set radio_2 CE, CSN PIN
CSN_PIN_2 = 10 #SPI1 CE0 -> spidev1.0
if RF24_DRIVER == 'MRAA':
    CE_PIN_2 = 32
elif RF24_DRIVER == 'wiringPi':
    CE_PIN_2 = 26
else:
    CE_PIN_2 = 12

radio_2 = RF24(CE_PIN_2, CSN_PIN_2)

def test_radio_initialization(radio, radio_number):
    if not radio.begin():
        raise RuntimeError(f"radio_{radio_number} hardware is not responding")
    print(f"radio_{radio_number} initialized successfully")

try:
    test_radio_initialization(radio_1, 1)
    test_radio_initialization(radio_2, 2)
except RuntimeError as e:
    print(e)
finally:
    radio_1.powerDown()
    radio_2.powerDown()