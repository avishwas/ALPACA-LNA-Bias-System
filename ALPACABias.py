"""
:Authors: - Cody Roberson (carobers@asu.edu)
    - Eric Weeks
:Date: 4/3/2024
:Copyright: 2023 Arizona State University
:Version: 2.0.1
:Revision: 
    2.0.1 - Cody Roberson - Added ability to set current divider value for INA219
    2.1.0 - Amit Vishwas - Adding stateless comms functionality using local json files

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

CURRENT_DIVIDER = 2.7 # Default current divider value for INA219

def initialize_boards():
    """Initializes all bias boards and loads saved state if available."""
    try:
        boards = bc.BiasBoard.load_all_states()
        if not boards:
            # Initialize new boards if no saved state is found
            boards = [bc.BiasBoard(addr, CURRENT_DIVIDER) for addr in range(1, 19)]
            print("All 18 boards have been successfully initialized.")
        else:
            print("All 18 boards have been successfully retrieved from memory.")
        return boards
    except Exception as e:
        print(f"Error initializing or retrieving boards: {e}")
        return []


def save_boards_state(boards):
    """Saves the state of all bias boards."""
    bc.BiasBoard.save_all_states(boards)


def set_iLNA(chan: int, set_current: float):
    """Modifies V(out) until a desired set current is reached.

    :param chan: LNA channel (1 through 144)
    :type chan: int
    :param set_current: Desired current in mA
    :type set_current: float
    """
    boards = initialize_boards()
    bd, ch = __getboard(chan, boards)
#    bd, ch = __getboard(chan)

    i = 0
    potnum = 1 if ch <= 4 else 2
    wipernum = ch if ch <= 4 else ch - 4
    wiperpos = bd.get_pot(potnum, wipernum)
    if set_current == 0:
        bd.set_ioexpander(ch, 0)
        bd.set_pot(potnum, wipernum, 0)
        save_boards_state(boards)
        return
    else:
        bd.set_ioexpander(ch, 1)

    while 1:
        if i > MAXITER:
            print(
                f"|ERROR| iLNA exceeded {MAXITER}, please consider changing the tolerance"
            )
            break
        val = bd.get_current(ch)
        if val >= 1000.0:
            print("Error: Invalid value from INA, is the LNA connected?")
            return
        diff = abs(val - set_current) / set_current

        print(f"iLNA={val} diff={diff} wiperpos={wiperpos}")
        if diff < ITOLERANCE:
            break
        elif val < set_current:
            if wiperpos >= 255:
                print(f"Unable to reach {set_current} mA")
                break
            wiperpos += 1
            bd.set_pot(potnum, wipernum, wiperpos)
        else:
            if wiperpos <= 0:
                print(f"Unable to reach {set_current} mA")
                break
            wiperpos -= 1
            bd.set_pot(potnum, wipernum, wiperpos)
        i += 1
      
    save_boards_state(boards)


def set_vLNA(chan: int, set_voltage: float):
    """Modifies V(out) until a desired voltage is reached.

    :param chan: LNA channel (1 through 144)
    :type chan: int
    :param set_voltage: Desired voltage (Volts)
    :type set_voltage: float
    """
    boards = initialize_boards()
    bd, ch = __getboard(chan, boards)
    
    i = 0
#    bd, ch = __getboard(chan)
    potnum = 1 if ch <= 4 else 2
    wipernum = ch if ch <= 4 else ch - 4
    wiperpos = bd.get_pot(potnum, wipernum)
    
    if set_voltage == 0:
        bd.set_ioexpander(ch, 0)
        bd.set_pot(potnum, wipernum, 0)
        save_boards_state(boards)
        return
    else:
        bd.set_ioexpander(ch, 1)
    while 1:
        if i > MAXITER:
            print(
                f"|ERROR| vLNA exceeded {MAXITER}, please consider changing the tolerance"
            )
            break
        val = bd.get_bus(ch)
        if val >= 3.0:
            print("Error: Invalid value from INA, is the LNA connected or the Bias Board Damaged?")
            return
        diff = abs(val - set_voltage) / set_voltage

        print(f"vLNA={val} diff={diff} wiperpos={wiperpos}")
        if diff < VTOLERANCE:
            break
        elif val < set_voltage:
            if wiperpos >= 255:
                print(f"Unable to reach {set_voltage} V")
                break
            wiperpos += 1
            bd.set_pot(potnum, wipernum, wiperpos)
        else:
            if wiperpos <= 0:
                print(f"Unable to reach {set_voltage} V")
                break
            wiperpos -= 1
            bd.set_pot(potnum, wipernum, wiperpos)
        i += 1
      
    save_boards_state(boards)


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
    """Reads the (bus) voltage in volts for a given channel.

    :param chan: LNA channel (1 through 144)
    :type chan: int
    :return: Voltage (Volts)
    :rtype: float
    """
    bd, ch = __getboard(chan)
    return bd.get_bus(ch)


#def __getboard(channel: int) -> tuple[bc.BiasBoard, int]:
def __getboard(channel: int, boards):
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
    return boards[boardnum - 1], boardchan)


boards = initialize_boards()
#__boards = []
#for i in range(1, 18 + 1):
#    print(f"Initializing Board {i}")
#    b = bc.BiasBoard(i, CURRENT_DIVIDER)
#    __boards.append(b)
