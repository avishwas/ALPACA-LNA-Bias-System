"""
:author: Cody Roberson (carobers@asu.edu)
:date: 04/24/2023
:descrption: Controls / Tests the ALPACA bias boards designed by Eric W.
:revisions:
    0.1 Initial implementation of skeleton and operations
"""
import odroid_wiringpi as wp
import logging

__VERSION__ = 0.1

__LOGGINGFORMAT = "%(asctime)s | %(levelname)s | %(filename)s:%(lineno)d | %(message)s"
logging.basicConfig(format=__LOGGINGFORMAT, level=logging.INFO)
log = logging.getLogger(__name__)


class BiasBoard:
    def __init__(self) -> None:
        self.__repeaterDevId = 0
        fd = wp.wiringPiI2CSetup(self.__repeaterDevId)

    def c_board(self, address: int):
        # connect to address
        pass

    def dc_board(self, address: int):
        # disconnect board
        pass

    def set_wiper(self, address: int):
        pass

    def read_current(self):
        pass

    def read_voltage(self):
        pass


if __name__ == "__main__":
    log.setLevel(logging.DEBUG)  # Remove me for release
    log.info("version %.1f", __VERSION__)
    pass
