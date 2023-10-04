"""
:Authors: - Cody Roberson (carobers@asu.edu)
:Date: 08/18/2023
:Copyright: 2023 Arizona State University
:Version: 0.8

Overview
--------
Provides a control interface for users to apply to the ALPACA LNA Bias System.

..Note ::
    This software requires root


"""
from odroid_wiringpi import wiringPiI2CSetupInterface as setup
from odroid_wiringpi import wiringPiI2CWrite as write
from odroid_wiringpi import wiringPiI2CRead as read
from odroid_wiringpi import wiringPiI2CWriteReg16 as write16
from odroid_wiringpi import wiringPiI2CReadReg16 as read16
from odroid_wiringpi import wiringPiI2CWriteReg8 as write8
from odroid_wiringpi import wiringPiI2CReadReg8 as read8
from odroid_wiringpi import serialClose as close
import sys
import logging
from os import system, geteuid
from time import sleep

__VERSION__ = 0.8

# Setup Logging
__LOGFMT = "%(asctime)s|%(levelname)s|%(filename)s|%(lineno)d|%(funcName)s|%(message)s"
logging.basicConfig(format=__LOGFMT, level=logging.INFO)
logger = logging.getLogger("BiasControl")
__logh = logging.FileHandler("./BiasControl_Module.log")
logger.addHandler(__logh)
logger.log(100, __LOGFMT)
__logh.flush()
__logh.setFormatter(logging.Formatter(__LOGFMT))

if geteuid() != 0:
    raise BaseException(
        "\033[0;31m\n\nBiasControl.py requires ROOT privileges\n\033[0m"
    )

# DEVICE INSTRUCTIONS
INSTR_LTCCONNECT = 0b11100000
INSTR_LTCDISCONNECT = 0b01100000
AD5144_CMD_WRITE_RDAC = 0b00010000

# Device Addresses
PCF8575_BASE_ADDR = 0b0100_000
LTC4302_BASE_ADDR = 0b11_00000
DIGITALPOT_1_I2C_ADDR = 0b0101111
DIGITALPOT_2_I2C_ADDR = 0b0100011
CURR_SENSE_BASE_I2C_ADDR = 0b1001000

# Constants for the INA219
INA219_CONFIG_BVOLTAGERANGE_32V = 0x2000
INA219_CONFIG_GAIN_8_320MV = 0x1800
INA219_CONFIG_BADCRES_12BIT = 0x0180
INA219_CONFIG_SADCRES_12BIT_1S_532US = 0x0018
INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS = 0x07
INA219_REG_CONFIG = 0x00
INA219_REG_CALIBRATION = 0x05
INA219_REG_SHUNTVOLTAGE = 0x01
INA219_REG_BUSVOLTAGE = 0x02
INA219_REG_POWER = 0x03
INA219_REG_CURRENT = 0x04

# HARD CODED CONFIG
# calibration_reg  0x5 0x1000
# config_reg -> 0x0 0x399f
# INA219CONFIG = 0x399F
# INACALVALUE = 0x1000
INA219CONFIG = 0b1001_1111_0011_1001
INACALVALUE = 0b0000_0000_0001_0000
# Linux I2C bus path
BUS = "/dev/i2c-0"


class BiasChannel:
    def __init__(self, addr: int = 0) -> None:
        """Creats a control interface for one of the bias boards for a given address in a given system.

        .. WARNING::
            This library modifies the I2C Clock from the default 400 KHz to 100KHz

        :param addr: Board address from 0 to 31. For the end user, this shall
        be boards 1 to 18.
        """
        # Write the correct clockspeed
        logger.debug(f"Bias Channel init, board_addr={addr}")
        self.addr = addr
        f = open("/sys/bus/i2c/devices/i2c-0/device/speed", "r")
        if int(f.readline()) != 100_000:
            logger.debug("i2c bus 0 is not 100KHz, setting..")
            f.close()
            f = open("/sys/bus/i2c/devices/i2c-0/device/speed", "w")
            f.write(f"100000")
            f.close()
        else:
            logger.debug("i2c bus 0 was 100KHz")
            f.close()

        assert addr >= 0, "Address must be 0 or higher"
        assert addr <= 31, "Address must be 31 or lower"
        self.pinstate = 0

    def get_fioexpander(self):
        return setup(BUS, PCF8575_BASE_ADDR)

    def get_fpot1(self):
        return setup(BUS, DIGITALPOT_1_I2C_ADDR)

    def get_fpot2(self):
        return setup(BUS, DIGITALPOT_2_I2C_ADDR)

    def get_fina(self, chan):
        return setup(BUS, CURR_SENSE_BASE_I2C_ADDR + chan)

    def set_ioexpander(self, pin, state, zero=False):
        self.__start()
        iox = self.get_fioexpander()
        if zero:
            self.pinstate = 0
            write16(iox, 0, 0)
        else:
            p = self.pinstate
            if state:
                p = p | (1 << pin)
            else:
                p = p & (~(1 << pin))
            self.pinstate = p
            write16(iox, p & 0xFF, (p & 0xFF00) >> 8)
        self.__end([iox])

    def set_pot(self, pot=1, wiper=0, value=0):
        self.__start()
        pot = None
        if pot == 2:
            pot = self.get_fpot2()
        else:
            pot = self.get_fpot1()
        write8(pot, AD5144_CMD_WRITE_RDAC + wiper, value)
        self.__end([pot])

    def set_pot_linear(self, pot=1, wiper=0, value=0):
        self.__start()
        pot = None
        if pot == 2:
            pot = self.get_fpot2()
        else:
            pot = self.get_fpot1()
        write16(pot, 0b1101000, 0b00001110)
        self.__end([pot])

    def zero_fpot(self):
        """Sets all of the Digital Pots on the Board to 0"""
        self.set_pot(1, 0, 0)
        self.set_pot(1, 1, 0)
        self.set_pot(1, 2, 0)
        self.set_pot(1, 3, 0)
        self.set_pot(2, 0, 0)
        self.set_pot(2, 1, 0)
        self.set_pot(2, 2, 0)
        self.set_pot(2, 3, 0)

    def zero_ioexpander(self):
        self.set_ioexpander(0, 0, True)

    def __start(self):
        """Used to prepare controller <-> board connection by instructing the board's i2c repeater
        to connect our respective busses.
        """
        self.frptr = setup(BUS, LTC4302_BASE_ADDR + self.addr)
        write(self.frptr, INSTR_LTCCONNECT)

    def __end(self, devices=[]):
        """Used to close controller <-> board connection by instructing a i2c repeater
        to disconnect our i2c bus to the board's bus.
        """
        for d in devices:
            close(d)
        write(self.frptr, INSTR_LTCDISCONNECT)
        close(self.frptr)

    # INA219 stuff below...
    def ina_init(self, chan):
        """
        Initializes all of the Board's INA219's
        Code ripped from https://github.com/adafruit/Adafruit_INA219/blob/master/Adafruit_INA219.cpp
        """
        self.__start()
        sleep(0.1)
        self.fina = self.get_fina(chan)

        self.ina219_currentDivider_mA = 10
        self.ina219_powerMultiplier_mW = 2

        # Set Calibration register to 'Cal' calculated above
        write16(self.fina, INA219_REG_CALIBRATION, INACALVALUE)
        #        write16(self.fina, INACALVALUE, INA219_REG_CALIBRATION)
        write16(self.fina, INA219_REG_CONFIG, INA219CONFIG)

        #       write16(self.fina, INA219CONFIG, INA219_REG_CONFIG)
        self.__end([self.fina])

    def ina_getCurrent_raw(self, chan):
        self.__start()
        ina = self.get_fina(chan)
        write16(ina, INA219_REG_CALIBRATION, INACALVALUE)
        val = read16(ina, INA219_REG_CURRENT)
        # lsb = (val & 0xFF00) >>8
        # msb = (val & 0xFF)
        # val = (msb << 8) | lsb
        self.__end([ina])
        return val

    def ina_getPower_raw(self, chan):
        self.__start()
        ina = self.get_fina(chan)
        write16(ina, INA219_REG_CALIBRATION, INACALVALUE)
        val = read16(ina, INA219_REG_POWER)
        self.__end([ina])
        return val

    def ina_getShuntVoltage_raw(self, chan):
        self.__start()
        ina = self.get_fina(chan)
        val = read16(ina, INA219_REG_SHUNTVOLTAGE)
        lsb = (val & 0xFF00) >> 8
        msb = val & 0xFF
        val = (msb << 8) | lsb

        self.__end([ina])
        return val

    def ina_getBusVoltage_raw(self, chan):
        self.__start()
        ina = self.get_fina(chan)
        sleep(0.1)
        write16(ina, INA219_REG_CALIBRATION, INACALVALUE)
        val = read16(ina, INA219_REG_BUSVOLTAGE)
        self.__end([ina])
        return (
            val >> 3
        ) * 4  # Shift to the right 3 to drop CNVR and OVF and multiply by LSB

    def ina_getShuntVoltage_mV(self, chan):
        return 0.01 * self.ina_getShuntVoltage_raw(chan)

    def ina_getBusVoltage_V(self, chan):
        value = self.ina_getBusVoltage_raw(chan)
        return value * 0.001

    def ina_getCurrent_mA(self, chan):
        valueDec = self.ina_getCurrent_raw(chan)
        valueDec /= self.ina219_currentDivider_mA
        return valueDec

    def ina_getPower_mA(self, chan):
        valueDec = self.ina_getPower_raw(chan)
        valueDec *= self.ina219_powerMultiplier_mW
        return valueDec


if __name__ == "__main__":
    logger.info("Running Test")
    if len(sys.argv) > 1:
        pass
    else:
        pass
