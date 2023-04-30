"""
:author: Cody Roberson (carobers@asu.edu)
:date: 04/30/2023
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


def begin_comms(iic_address: int = 1):
    # connect to address
    pass


if __name__ == "__main__":
    log.setLevel(logging.DEBUG)  # Remove me for release
    log.info("version %.1f", __VERSION__)
    pass
