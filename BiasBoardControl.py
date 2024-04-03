"""
:Authors: - Cody Roberson (carobers@asu.edu)
    - Eric Weeks
:Date: 4/3/2024
:Copyright: 2023 Arizona State University
:Version: 2.0.1

:revision:
    2.0.1 - Cody Roberson - broke out the ability to control current sense divider

Low level serial interface for interacting with the cards within the VME unit.
**Users shouldn't need anything from this module.**

"""
from odroid_wiringpi import wiringPiI2CSetupInterface as setup
from odroid_wiringpi import wiringPiI2CWrite as write
from odroid_wiringpi import wiringPiI2CRead as read
from odroid_wiringpi import wiringPiI2CWriteReg16 as write16
from odroid_wiringpi import wiringPiI2CReadReg16 as read16
from odroid_wiringpi import wiringPiI2CWriteReg8 as write8
from odroid_wiringpi import wiringPiI2CReadReg8 as read8
from odroid_wiringpi import serialClose as close
from time import sleep
import numpy as np

__VERSION__ = "2.0.1"

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


def MSBF(val: int) -> int:
    """Swaps byte order of 'val'. (needed for current sense chip)

    :param val: 16-bit value
    :type val: int (unsigned)
    :return: 16-bit MSB first value
    :rtype: int (unsigned)
    """
    return ((val & 0xFF) << 8) | ((val >> 8) & 0xFF)


# Hard Coded Config based off of Adafruit INA219 32V_2A
INA219CONFIG = MSBF(
    INA219_CONFIG_BVOLTAGERANGE_32V
    | INA219_CONFIG_GAIN_8_320MV
    | INA219_CONFIG_BADCRES_12BIT
    | INA219_CONFIG_SADCRES_12BIT_1S_532US
    | INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS
)
INACALVALUE = MSBF(0x1000)

# Odroid - Linux I2C bus path
BUS = "/dev/i2c-0"
HIGH = True
LOW = False


class BiasBoard:
    """Creats a control interface for one of the bias boards for a given address in a given system.

    .. Note::
        This library modifies the I2C Clock from the default 400 KHz to 100KHz

    :param addr: Bias board address within VME crate. (1 to 18).
    :param curr_divider: Current divider for the INA219 current sense chip. (default 2.0)
    """

    def __init__(self, addr: int, curr_divider: float = 2.0) -> None:
        # Write the correct clockspeed
        assert addr >= 1, "Invalid Bias Board Address (1 to 18)"
        assert addr <= 18, "Invalid Bias Board Address (1 to 18)"
        self.addr = addr
        f = open("/sys/bus/i2c/devices/i2c-0/device/speed", "r")
        if int(f.readline()) != 100_000:
            f.close()
            f = open("/sys/bus/i2c/devices/i2c-0/device/speed", "w")
            f.write(f"100000")
            f.close()
        else:
            f.close()
        self.pinstate = 0
        self.pot1 = [0.0, 0.0, 0.0, 0.0]
        self.pot2 = [0.0, 0.0, 0.0, 0.0]

        # Do startup procedure
        self.zero_ioexpander()
        self.set_ioexpander(0, HIGH)
        for j in range(1, 8 + 1):
            self.init_currsense(j, curr_divider)
            self.set_ioexpander(j, HIGH)
        self.zero_pots()

    def __get_fioexpander(self):
        return setup(BUS, PCF8575_BASE_ADDR)

    def __get_fpot1(self):
        return setup(BUS, DIGITALPOT_1_I2C_ADDR)

    def __get_fpot2(self):
        return setup(BUS, DIGITALPOT_2_I2C_ADDR)

    def __get_fina(self, chan):
        assert chan > 0, "Channel can't be negative. (possible choices are 1-8)"
        assert chan <= 8, "Channel doesnt exist. (possible choices are 1-8)"
        chan = chan - 1
        return setup(BUS, CURR_SENSE_BASE_I2C_ADDR + chan)

    def set_ioexpander(self, pin: int, state: bool, zero=False):
        """Set gpio pin high or low within a the bias board

        :param pin: pin number (0 through 16)
        :type pin: int
        :param state: HIGH or LOW
        :type state: bool
        :param zero: When set to true, sets all pins to 0
        :type zero: bool, optional
        """
        assert pin >= 0 and pin <= 16, "Invalid pin, (0 - 16)"
        self.__start()
        iox = self.__get_fioexpander()
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

    def set_pot(self, pot: int, wiper: int, value: int = 0):
        """Sets a given digital pot, wiper to value

        :param pot: Digital Pot (1 or 2)
        :type pot: int
        :param wiper: Wiper number
        :type wiper: int
        :param value: What value to set the wiper to (0-255), defaults to 0
        :type value: int, optional
        """
        assert wiper > 0 and wiper <= 4, "Invalid wiper number (1-4)"
        assert pot == 1 or pot == 2, "Only pot 1 or 2 exists in the system"
        assert value >= 0 and value <= 255, "Acceptable values range 0 through 255"
        wiper = wiper - 1

        self.__start()
        if pot == 2:
            pot = self.__get_fpot2()
            self.pot2[wiper] = value
        else:
            pot = self.__get_fpot1()
            self.pot1[wiper] = value
        write8(pot, AD5144_CMD_WRITE_RDAC + wiper, value)
        self.__end([pot])

    def get_pot(self, pot, wiper) -> int:
        """Gets a given digital pot value

        :param pot: Digital Pot (1 or 2)
        :type pot: int, optional
        :param wiper: Wiper number
        :type wiper: int, optional
        :return: What the wiper value is (0-255)
        :rtype: int
        """
        assert wiper > 0 and wiper <= 4, "Invalid wiper number (1-4)"
        assert pot == 1 or pot == 2, "Only pot 1 or 2 exists in the system"
        wiper = wiper - 1

        if pot == 2:
            return self.pot2[wiper]
        else:
            return self.pot1[wiper]

    def __set_pot_linear(self, pot=1, wiper=0, value=0):
        self.__start()
        if pot == 2:
            pot = self.__get_fpot2()
        else:
            pot = self.__get_fpot1()
        write16(pot, 0b1101000, 0b00001110)
        self.__end([pot])

    def zero_pots(self):
        """Sets all of the Digital Pots on the Board to 0"""
        self.set_pot(1, 1, 0)
        self.set_pot(1, 2, 0)
        self.set_pot(1, 3, 0)
        self.set_pot(1, 4, 0)
        self.set_pot(2, 1, 0)
        self.set_pot(2, 2, 0)
        self.set_pot(2, 3, 0)
        self.set_pot(2, 4, 0)

    def zero_ioexpander(self):
        """Sets all of the pins on the internal io-expander to 0"""
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

    def init_currsense(self, chan: int, currentDivider: float = 2.0) -> None:
        """Initializes an INA219 current sense chip for a given channel

        Code ripped from https://github.com/adafruit/Adafruit_INA219/blob/master/Adafruit_INA219.cpp

        :param chan: select the bias channel to initialize (1 through 8)
        :type chan: int
        :param currentDivider: useful for cases where sense resistors have been modified, defaults to 2.0
        :type currentDivider: float, optional
        """
        self.__start()
        sleep(0.1)
        self.fina = self.__get_fina(chan)

        self.ina219_currentDivider_mA = currentDivider
        self.ina219_powerMultiplier_mW = 2

        # Set Calibration register to 'Cal' calculated above
        write16(self.fina, INA219_REG_CALIBRATION, INACALVALUE)
        write16(self.fina, INA219_REG_CONFIG, INA219CONFIG)
        self.__end([self.fina])

    def __ina_getCurrent_raw(self, chan):
        self.__start()
        ina = self.__get_fina(chan)
        write16(ina, INA219_REG_CALIBRATION, INACALVALUE)
        val = MSBF(read16(ina, INA219_REG_CURRENT))
        self.__end([ina])
        return val

    def __ina_getPower_raw(self, chan):
        self.__start()
        ina = self.__get_fina(chan)
        write16(ina, INA219_REG_CALIBRATION, INACALVALUE)
        val = MSBF(read16(ina, INA219_REG_POWER))
        self.__end([ina])
        return val

    def __ina_getShuntVoltage_raw(self, chan):
        self.__start()
        ina = self.__get_fina(chan)
        val = MSBF(read16(ina, INA219_REG_SHUNTVOLTAGE))
        self.__end([ina])
        return val

    def __ina_getBusVoltage_raw(self, chan):
        self.__start()
        ina = self.__get_fina(chan)
        write16(ina, INA219_REG_CALIBRATION, INACALVALUE)
        val = MSBF(read16(ina, INA219_REG_BUSVOLTAGE))
        self.__end([ina])
        return (
            val >> 3
        ) * 4  # Shift to the right 3 to drop CNVR and OVF and multiply by LSB

    def get_shunt(self, chan: int, navg: int = 6) -> float:
        """Reads a current monitor for it's shunt voltage for a given channel.

        :param chan: Select the bias channel to measure (1 through 8)
        :type chan: int
        :param navg: number of values to average, defaults to 6
        :type navg: int, optional
        :return: shunt voltage in mV
        :rtype: float
        """
        val = np.zeros(navg)
        for ind, _ in enumerate(val):
            val[ind] = 0.01 * self.__ina_getShuntVoltage_raw(chan)
        return np.average(val)

    def get_bus(self, chan: int, navg: int = 6) -> float:
        """Reads a current monitor for it's bus voltage for a given channel

        :param chan: Select the bias channel to measure (1 through 8)
        :type chan: int
        :param navg: number of values to average, defaults to 6
        :type navg: int, optional
        :return: bus voltage in Volts
        :rtype: float
        """
        val = np.zeros(navg)
        for ind, _ in enumerate(val):
            val[ind] = self.__ina_getBusVoltage_raw(chan) * 0.001
        return np.average(val)

    def get_current(self, chan: int, navg: int = 6) -> float:
        """Reads a current monitor for the bias's current draw for a given channel

        :param chan: Select the bias channel to measure (1 through 8)
        :type chan: int
        :param navg: number of values to average, defaults to 6
        :type navg: int, optional
        :return: Current in mA
        :rtype: float
        """
        val = np.zeros(navg)
        for ind, _ in enumerate(val):
            val[ind] = self.__ina_getCurrent_raw(chan) / (
                10 * self.ina219_currentDivider_mA
            )
        return np.average(val)


def test_bias_board(board: int):
    """BIAS BOARD TEST ROUTINE.

    .. DANGER ::
        This function is used to test the bias boards themselves at ASU. Inappropriate use may result in damaged amplifiers/equipment.

    Iterates through each channel and does the following:
        #. Enable corresponding regulator using io expander
        #. Init current sense chip
        #. sets pot to 0
        #. reads current, shunt, and bus
        #. sets pot to 100
        #. reads current, shunt, and bus
        #. sets pot to 200
        #. reads current, shunt, and bus
        #. repeat procedure for next channel
    """
    b = BiasBoard(board)
    print(f"Setting all expanders to 0 for board {board}")
    b.zero_ioexpander()
    b.set_ioexpander(0, HIGH)

    for chan in range(1, 8 + 1):
        input(f"press enter to enable regulator for channel {chan}")
        b.set_ioexpander(chan, HIGH)
        input(f"press enter to initialize the current sense chip for channel {chan}")

        b.init_currsense(chan, 100)

        pot = 1 if chan <= 4 else 2
        wiper = chan - 5 if chan > 4 else chan - 1
        input(f"press enter to set pot {pot};wiper {wiper}; for {chan} to 0")
        b.set_pot(pot, wiper, 0)

        input(
            f"press enter to enable take a current, bus, and shunt measurement for channel {chan} at wiper=0"
        )
        print(
            f"\tcurrent {b.get_current(chan)}\n\tshunt {b.get_shunt(chan)}\n\tbus {b.get_bus(chan)}"
        )

        input(f"press enter to set pot {pot};wiper {wiper}; for {chan} to 100")
        b.set_pot(pot, wiper, 100)

        input(
            f"press enter to enable take a current, bus, and shunt measurement for channel {chan} at wiper=100"
        )
        print(
            f"\tcurrent {b.get_current(chan)}\n\tshunt {b.get_shunt(chan)}\n\tbus {b.get_bus(chan)}"
        )

        input(f"press enter to set pot {pot};wiper {wiper}; for {chan} to 200")
        b.set_pot(pot, wiper, 200)

        input(
            f"press enter to enable take a current, bus, and shunt measurement for channel {chan} at wiper=200"
        )
        print(
            f"\tcurrent {b.get_current(chan)}\n\tshunt {b.get_shunt(chan)}\n\tbus {b.get_bus(chan)}"
        )

        print("\n\n\n")
