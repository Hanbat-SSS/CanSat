import smbus
import time
import subprocess
from datetime import datetime

class MS5611_01BXXX:
    def __init__(self, address):
        self.address = address
        self.bus = smbus.SMBus(1)

    def reset_cmd(self):
        """센서를 리셋합니다."""
        self.bus.write_byte(self.address, 0x1E)

    def read_coefficient(self):
        """보정 계수를 읽습니다."""
        self.C1 = self._read_prom(0xA2)
        self.C2 = self._read_prom(0xA4)
        self.C3 = self._read_prom(0xA6)
        self.C4 = self._read_prom(0xA8)
        self.C5 = self._read_prom(0xAA)
        self.C6 = self._read_prom(0xAC)

    def _read_prom(self, cmd):
        """PROM 데이터를 읽습니다."""
        data = self.bus.read_i2c_block_data(self.address, cmd, 2)
        return data[0] * 256 + data[1]

    def pres_conversion(self):
        """압력 변환 명령을 전송합니다."""
        self.bus.write_byte(self.address, 0x40)

    def read_pressure(self):
        """압력 데이터를 읽습니다."""
        data = self.bus.read_i2c_block_data(self.address, 0x00, 3)
        self.D1 = data[0] * 65536 + data[1] * 256 + data[2]

    def temp_conversion(self):
        """온도 변환 명령을 전송합니다."""
        self.bus.write_byte(self.address, 0x50)

    def read_temp(self):
        """온도 데이터를 읽습니다."""
        data = self.bus.read_i2c_block_data(self.address, 0x00, 3)
        self.D2 = data[0] * 65536 + data[1] * 256 + data[2]

    def result_conversion(self):
        """측정 데이터를 실제 압력과 온도로 변환합니다."""
        dT = self.D2 - (self.C5 * 256)
        TEMP = 2000 + ((dT * self.C6) / 8388608)
        OFF = self.C2 * 65536 + (self.C4 * dT) / 128
        SENS = self.C1 * 32768 + (self.C3 * dT) / 256

        T2 = 0
        OFF2 = 0
        SENS2 = 0

        if TEMP < 2000:
            T2 = (dT * dT) / 2147483648
            OFF2 = 5 * ((TEMP - 2000) ** 2) / 2
            SENS2 = 5 * ((TEMP - 2000) ** 2) / 4
            if TEMP < -1500:
                OFF2 += 7 * ((TEMP + 1500) ** 2)
                SENS2 += 11 * ((TEMP + 1500) ** 2) / 2

        TEMP -= T2
        OFF -= OFF2
        SENS -= SENS2

        pressure = ((((self.D1 * SENS) / 2097152) - OFF) / 32768.0) / 100.0
        cTemp = TEMP / 100.0
        fTemp = cTemp * 1.8 + 32

        return {'p': pressure, 'c': cTemp, 'f': fTemp}

    def cal_altitude(self, pressure, sea_pressure=1026.4):
        """압력을 이용해 고도를 계산합니다."""
        return 44330 * (1 - (pressure / sea_pressure) ** (1 / 5.255))

# 두 센서를 각각 설정
sensor_1 = MS5611_01BXXX(0x76)
sensor_2 = MS5611_01BXXX(0x77)

# Temperature_1 샘플링 데이터를 저장할 리스트
temperature_1_samples = []
file_path = "/home/test2/MS5611_data.txt"
# Target Temperature Setting
temperature_set = 25.0 
rea_excuted = False
while True:
    # 각 센서 데이터를 저장할 변수 초기화
    sensor_data = {}

    for idx, sensor in enumerate([sensor_1, sensor_2], start=1):
        sensor.reset_cmd()
        time.sleep(0.1)
        sensor.read_coefficient()
        sensor.pres_conversion()
        time.sleep(0.1)
        sensor.read_pressure()
        sensor.temp_conversion()
        time.sleep(0.1)
        sensor.read_temp()
        data = sensor.result_conversion()

        # 센서 데이터 구분 저장
        sensor_data[f"Pressure_{idx}"] = data['p']
        sensor_data[f"Temperature_{idx}"] = data['c']
        sensor_data[f"Altitude_{idx}"] = sensor.cal_altitude(data['p'])

    local = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    with open(file_path, "a") as file:
      file.write(f"Time:{local}, Pressure_1: {sensor_data['Pressure_1']:.2f} , Tempreature_1: {sensor_data['Temperature_1']:.2f} , Altitude_1: {sensor_data['Altitude_1']:.2f}  |  Pressure_2: {sensor_data['Pressure_2']:.2f} , Tempreature_2: {sensor_data['Temperature_2']:.2f} , Altitude_2: {sensor_data['Altitude_2']:.2f} \n")

    # Temperature_1 값 샘플링 진행
    temperature_1 = sensor_data["Temperature_1"]
    temperature_1_samples.append(temperature_1)

    # 샘플링 리스트 크기 유지 (최신 5개만 저장)
    if len(temperature_1_samples) > 5:
        temperature_1_samples.pop(0)

    # 온도 감소 여부 확인 및 현재 온도가 25도 이하인 경우 출력
    if len(temperature_1_samples) == 5:
        # 샘플링된 데이터가 감소하고 있는지 확인
        is_decreasing = all(
            temperature_1_samples[i] > temperature_1_samples[i + 1]
            for i in range(len(temperature_1_samples) - 1)
        )
        if is_decreasing and temperature_1 <= temperature_set:
            print("Reaction Wheel Activate!!!")
            # subprocess.Popen(["sudo", "/home/cansat/rea"])
            # rea_excuted = True

    # 동시에 업데이트된 값 출력
    print(f"Pressure_1: {sensor_data['Pressure_1']:.2f} mbar, "
          f"Temperature_1: {sensor_data['Temperature_1']:.2f} °C, "
          f"Altitude_1: {sensor_data['Altitude_1']:.2f} m")

    print(f"Pressure_2: {sensor_data['Pressure_2']:.2f} mbar, "
          f"Temperature_2: {sensor_data['Temperature_2']:.2f} °C, "
          f"Altitude_2: {sensor_data['Altitude_2']:.2f} m")
    

    print("*********************")
    time.sleep(0.4)
