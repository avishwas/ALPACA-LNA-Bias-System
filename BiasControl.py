"""
:Authors: - Cody Roberson (carobers@asu.edu)
:Date: 08/15/2023
:Copyright: 2023 Arizona State University

Overview
--------
Provides a control interface for users to apply to the ALPACA LNA Bias System.

..Note ::
    This software requires root


"""
import odroid_wiringpi as op
import logging
from os import system, geteuid
write = op.wiringPiI2CWrite
read = op.wiringPiI2CRead
setup = op.wiringPiI2CSetupInterface

__VERSION__ = 0.1

# Setup Logging
__LOGFMT = "%(asctime)s|%(levelname)s|%(filename)s|%(lineno)d|%(funcName)s|%(message)s"
logging.basicConfig(format=__LOGFMT, level=logging.INFO)
__logger = logging.getLogger("BiasControl")
__logh = logging.FileHandler("./BiasControl_python.log")
__logger.addHandler(__logh)
__logger.log(100, __LOGFMT)
__logh.flush()
__logh.setFormatter(logging.Formatter(__LOGFMT))

if geteuid() != 0:
    raise BaseException("\033[0;31m\n\nBiasControl.py requires ROOT privileges\n\033[0m")

# Address Constants
LTC4302_BASE_ADDR = 0b1_1100_000
INSTR_LTCDISCONNECT = 0b0_1100_000
PCF8575_BASE_ADDR = 0b0100_000
LTC4302_BASE_ADDR = 0b11_00000
DIGITALPOT_1_I2C_ADDR  = 0b0101111
__DIGITALPOT_2_I2C_ADDR = 0b0100011
__CURR_SENSE_BASE_I2C_ADDR = 0b1001000
AD5144_CMD_WRITE_RDAC = 0b00010000

class BiasChannel():
    
    def __init__(self, board_addr: int = 0) -> None:
        """ Creats a control interface for one of the bias boards for a given address in a given system.

        .. Danger::
            This library modifies the I2C Clock from the default 400 KHz to 100KHz

        :param board_addr: Board address from 00 to 31
        """
        system("cat /sys/bus/i2c/devices/i2c-0/device/speed")
        system("echo 100000 > /sys/bus/i2c/devices/i2c-0/device/speed")
        system("cat /sys/bus/i2c/devices/i2c-0/device/speed")
        assert board_addr >= 0, "Address must be 0 or higher"
        assert board_addr <= 31, "Address must be 31 or lower"

        self.__repeater_fh = setup(0, LTC4302_BASE_ADDR+board_addr)
        xpandrfh = setup(0, PCF8575_BASE_ADDR)
        write(xpandrfh, 0)
        write(xpandrfh, 0)
        op.serialClose(xpandrfh)

    def __start(self):
        """ Used to prepare controller <-> board connection by instructing a i2c repeater
        to connect our i2c bus to the board's bus.
        """
        write(self.__repeater_fh, LTC4302_BASE_ADDR)
        

    def __end(self):
        """ Used to close controller <-> board connection by instructing a i2c repeater
        to disconnect our i2c bus to the board's bus.
        """
        write(self.__repeater_fh, INSTR_LTCDISCONNECT)

    def test_chan1(self):
        """ Test Function. Used to test that the bias board Channel 1 is Working.
        """
        self.__start()
        xpandrfh = setup(0, PCF8575_BASE_ADDR)
        dpfh = setup(0, DIGITALPOT_1_I2C_ADDR)
        xpandr = 0b11
        write(xpandrfh, xpandr)
        write(xpandrfh, 0)
        write(dpfh, AD5144_CMD_WRITE_RDAC)
        write(dpfh, 255)
        op.serialClose(dpfh)
        op.serialClose(xpandr)
        self.__end()


if __name__ == "__main__":
    bc = BiasChannel(00)
    bc.test_chan1()