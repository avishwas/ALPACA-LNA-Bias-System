"""
:author: Cody Roberson (carobers@asu.edu)
:date: 04/30/2023
:descrption: Controls / Tests the ALPACA bias boards designed by Eric W.
:revisions:
    0.1 Initial implementation of skeleton and operations
    0.2 Implement approved test fucntions

"""
import odroid_wiringpi as wp
import logging

__VERSION__ = 0.1

__LOGGINGFORMAT = "%(asctime)s | %(levelname)s | %(filename)s:%(lineno)d | %(message)s"
logging.basicConfig(format=__LOGGINGFORMAT, level=logging.INFO)
log = logging.getLogger(__name__)

__INSTR_LTCCONNECT = 0b1_1100_000
__INSTR_LTCDISCONNECT = 0b0_1100_000
__PCF8575ADDR = 0b0100_000
__LTC_FD = None  # LTC file descriptor
__PCF_FD = None


def begin_comms(iic_address: int = 0):
    """
    instructs the onboard LTC4302 to connect the i2c bus and do some intial setup consisting of the following:
        1. Set the I2C to connect
        2. set Expander Pins P11 to P0 to 0
    :param iic_address: i2c address
    """
    assert iic_address >= 0, "Address must be > 0"
    assert iic_address <= 31, "Max address is 31"
    ltcaddr = 0b11_00000 | iic_address
    __LTC_FD = wp.wiringPiI2CSetup(ltcaddr)
    # may want to check for None or 0 FD
    wp.wiringPiI2CWrite(__LTC_FD, __INSTR_LTCCONNECT)

    __PCF_FD = wp.wiringPiI2CSetup(__PCF8575ADDR)
    # set all IO pins low
    wp.wiringPiI2CWrite(__PCF_FD, 0)
    wp.wiringPiI2CWrite(__PCF_FD, 0)


def en_dpots():
    """Enable power for Digital pots
    p00 -> HIGH
    """
    wp.wiringPiI2CWrite(__PCF_FD, 0b0000_0001)
    wp.wiringPiI2CWrite(__PCF_FD, 0)


def en_v1():
    """Enable power for Voltage Regulator
    p01 -> HIGH
    """
    wp.wiringPiI2CWrite(__PCF_FD, 0b0000_0011)
    wp.wiringPiI2CWrite(__PCF_FD, 0)


def sweep_wiper():
    pass


def set_wiper():
    pass


def get_iv():
    pass


def end_comms(iic_address: int = 0):
    assert iic_address >= 0, "Address must be > 0"
    assert iic_address <= 31, "Max address is 31"


if __name__ == "__main__":
    log.setLevel(logging.DEBUG)  # Remove me for release
    log.info("version %.1f", __VERSION__)
    pass
