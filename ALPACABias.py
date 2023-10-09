"""
:Authors: - Cody Roberson (carobers@asu.edu)
    - Eric Weeks
:Date: 10/8/2023
:Copyright: 2023 Arizona State University
:Version: 1.1

Provides the end user with control over the LNA biases in the VME crate.
The channel parameter of these functions should be 1 through 144 inclusively.

.. Important ::
    This software requires the user to be root.

"""
import BiasBoardControl as bc
import math
import numpy as np

ITOLERANCE = 0.05  # %
VTOLERANCE = 0.05  # %

MAXITER = 256


def iLNA(chan: int, set_current: float):
    """Modifies V(out) until a desired set current is reached.

    :param chan: LNA channel (1 through 144)
    :type chan: int
    :param set_current: Desired current in mA
    :type set_current: float
    """
    bd, ch = __getboard(chan)
    potnum = 1 if ch <= 4 else 2
    wipernum = ch if ch <= 4 else ch - 4
    wiperpos = bd.get_pot(potnum, wipernum)
    while 1:
        if i > MAXITER:
            print(
                f"|ERROR| iLNA exceeded {MAXITER}, please consider changing the tolerance"
            )
            break
        val = bd.get_current(ch)
        diff = abs(val - set_current) / set_current
        if diff < ITOLERANCE:
            break
        elif val < set_current:
            if wiperpos >= 255:
                break
            wiperpos += 1
            bd.set_pot(potnum, wipernum, wiperpos)
        else:
            if wiperpos <= 0:
                break
            wiperpos -= 1
            bd.set_pot(potnum, wipernum, wiperpos)


def vLNA(chan: int, set_voltage: float):
    """Modifies V(out) until a desired voltage is reached.

    :param chan: LNA channel (1 through 144)
    :type chan: int
    :param set_voltage: Desired voltage (Volts)
    :type set_voltage: float
    """
    bd, ch = __getboard(chan)
    potnum = 1 if ch <= 4 else 2
    wipernum = ch if ch <= 4 else ch - 4
    wiperpos = bd.get_pot(potnum, wipernum)
    while 1:
        if i > MAXITER:
            print(
                f"|ERROR| vLNA exceeded {MAXITER}, please consider changing the tolerance"
            )
            break
        val = bd.get_bus(ch)
        diff = abs(val - set_voltage) / set_voltage
        if diff < VTOLERANCE:
            break
        elif val < set_voltage:
            if wiperpos >= 255:
                break
            wiperpos += 1
            bd.set_pot(potnum, wipernum, wiperpos)
        else:
            if wiperpos <= 0:
                break
            wiperpos -= 1
            bd.set_pot(potnum, wipernum, wiperpos)


def get_iLNA(chan: int):
    """Reads the current in mA for a given channel.

    :param chan: LNA channel (1 through 144)
    :type chan: int
    :return: Current (mA)
    :rtype: float
    """
    bd, ch = __getboard(chan)
    return bd.get_current(ch)


def get_vLNA(chan: int):
    """Reads the voltage in volts for a given channel.

    :param chan: LNA channel (1 through 144)
    :type chan: int
    :return: Voltage (Volts)
    :rtype: float
    """
    bd, ch = __getboard(chan)
    return bd.get_bus(ch)


def __getboard(channel: int) -> tuple[bc.BiasBoard, int]:
    """Converts a provided channel number into a bias board
    and channel relative to the board

    :param channel: User channel
    :type channel: int
    :return: Matching board and board channel
    :rtype: tuple[bc.BiasBoard, int]
    """
    assert (
        channel >= 1 and channel <= 144
    ), "Channel must be in range 1 to 144 inclusive"
    boardnum = math.ceil(channel / 8)
    boardchan = ((channel - 1) % 8) + 1
    return (__boards[boardnum - 1], boardchan)


__boards = []
for i in range(1, 18 + 1):
    b = bc.BiasBoard(i)
    __boards.append(b)
