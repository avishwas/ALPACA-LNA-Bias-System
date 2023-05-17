/**
 * @file arduino_based_testcode
 * @author carobers@asu.edu
 * @brief ALPACA Bias Board Control and Test Code
 * @date 2023-05-14
 *
 * @copyright Copyright (c) 2023 Arizona State University. All Rights Reserved.
 *
 *
 * Used to talk to an INA219 current/voltage sense chip and control a dc bias via an ad5144 digital potentiometer
 *
 * @revisions
 *  2022-03-07
 *      FILE CREATED
 *  2023-03-15
 *    Updated to utilize the LTC4302-A ii2 bus repeater. It is assumed that all code needs to communicate through the LTC.
 *    User needs to supply the LTC Address for each command.
 *  2023-05-05
 *    Updated header and modified code to reflect "ALPACA v2.0 rev2 Test Plan" document revison 2.1.
 *    Added extra functions to faciliate in-lab testing. Set to Version 0.3. We will need to fix the set-pin function.
 *    Version 0.3
 *  2023-05-14
 *    Now that a single amplifier subcircuit has been verified with this code on the bias boards, the code shall
 *    be extended to facilitate testing for the other subcircuits.
 *    Version 0.4
 *    TODO: When switching from card to card (via command 7), the current program will forget what the pin states are.
 *    Users should expect to find the pin states re-written to 0 when connecting to new or previous address
 *
 *  2023-05-15
 *    Testing in-lab. There was an error with the init ina219's where ina219_x.begin was not called. This prevented successful operation.
 *    Modified current divider for the INA's such that they report something more accurate.
 *    version 0.5
 *  2023-05-16
 *    Accounted for IO expander pin sate in commands 99 and 100. Removed extraneous 'newline/return'
 *    verison 0.6
 */

#include <Arduino.h>
#include "TuiConsole.h"
#include <Adafruit_INA219.h>
#include <Wire.h>
#define SW_VERSION_NUMBER "0.6"

// *********** I2C DEVICE ADDRESSES *****************
#define __DIGITALPOT_1_I2C_ADDR 0b0101111
#define __DIGITALPOT_2_I2C_ADDR 0b0100011
#define __I2C_REPEATER_I2C_ADDR 0b1100000
#define __IO_EXPANDER_I2C_ADDR 0b0100000
#define __CURR_SENSE_BASE_I2C_ADDR 0b1001000

// *********** I2C DEVICE COMMANDS *****************
#define __AD5144_CMD_WRITE_RDAC 0b00010000
#define __LTC4302_CMD_CONNECT_CARD 0b11100000
#define __LTC4302_CMD_DISCONNECT_CARD 0b01100000

// ************ OTHER DEFAULT PARAMETERS ******************

// Forward declarations telling the compiler that the code exists, but at the end
int setWiper(uint8_t dpot_i2c_addr, uint8_t wiper, uint8_t val);

// send the 'connect bus' command to the register.
int connect_i2c_bus(uint8_t address)
{
  Wire.beginTransmission(__I2C_REPEATER_I2C_ADDR + address);
  Wire.write(__LTC4302_CMD_CONNECT_CARD); // Connect IIC Bus
  return Wire.endTransmission();
}

// send the 'disconnect bus' command to the repeater
int disconnect_i2c_bus(uint8_t address)
{
  Wire.beginTransmission(__I2C_REPEATER_I2C_ADDR + address);
  Wire.write(__LTC4302_CMD_DISCONNECT_CARD);
  return Wire.endTransmission();
}

// global variables
uint16_t IO_EXPANDER_PINSTATE = 0;
int current_LTC_addr = 0;
Adafruit_INA219 *currsense[8];
TuiConsole *cons;

int setExpander(int pin, int state)
{
  IO_EXPANDER_PINSTATE ^= (-state ^ IO_EXPANDER_PINSTATE) & (1 << pin);
  Wire.beginTransmission(__IO_EXPANDER_I2C_ADDR);
  Wire.write((uint8_t)(IO_EXPANDER_PINSTATE & 0xFF));
  Wire.write((uint8_t)((IO_EXPANDER_PINSTATE >> 8) & 0xFF));
  return Wire.endTransmission();
}

void setup()
{
  cons = new TuiConsole(&Serial, 9600); // Setup Serial Console
  Wire.begin();                         // start/setup i2c arduino interface

  // initialize current sensor 1
  currsense[0] = new Adafruit_INA219(__CURR_SENSE_BASE_I2C_ADDR);
}

void loop()
{
  while (Serial.available() > 0)
    Serial.read();

  Serial.println("\r\nALPACA BIAS BOARD TEST CODE (built: " + String(__DATE__) + "_" + String(__TIME__) + "_v" + SW_VERSION_NUMBER + ")\r\n");
  Serial.println("Select Option:\r\n      1. begin comms\r\n      2. en dpots\r\n      3. en v1\r\n      4. sweep wiper 1\r\n      5. set wiper 1\r\n      6. get I+V 1\r\n      7. end_comms\r\n      8. set io expander pin ( numbered 0 - n )\r\n      9. set wiper\r\n      10. sweep wiper");
  Serial.println("      11. Initialize INA219s (2-8)\r\n      12. get I+V");
  Serial.println("      99. set all expander1 pins high\r\n      100. set all expander1 pins low\r\n");
  int cmd = cons->getInt("\r\noption: ");
  int wiper = 0, wiperval = 0, status = -1, pot = 0, pin = 0, state = 0, ina = 0;
  float cur = 0, vol = 0;
  int addr = 0;
  double shuntvoltage = 0;
  double busvoltage = 0;
  double current_mA = 0; // current in miliamps from INA219-1
  switch (cmd)
  {
  case 1: // begin_comms
    addr = cons->getInt("iic_address?: ");
    if (connect_i2c_bus(addr) == 0)
    {
      current_LTC_addr = addr;
      status = setExpander(0, LOW);
      Serial.println("expander status = " + String(status));
      if (!currsense[0]->begin())
      {
        Serial.println("Failed to find INA219 chip 1");
      }
    }
    else
    {
      Serial.println("I2c Error: LTC4302, Couldn't tell it to connect the bus");
    }
    break;
  case 2: // en_dpots
    status = setExpander(0, HIGH);
    Serial.println("expander status = " + String(status));

    break;
  case 3: // enable v1
    status = setExpander(1, HIGH);
    Serial.println("expander status = " + String(status));
    break;
  case 4: // sweep wiper ( sweep digital pot 1 wiper 1)
    cons->getInt("To stop, press any key / press enter\r\npress enter to begin");
    while (1)
    {
      setWiper(__DIGITALPOT_1_I2C_ADDR, 0, wiperval);
      delay(100);
      wiperval++;

      if (wiperval == 255)
        wiperval = 0;

      if (Serial.available() > 0)
        return;
    }
    break;
  case 5:                                                // set_wiper (digital pot 1 wiper 1)
    wiperval = cons->getInt("value? 0 to 255: ") & 0xFF; // get number and force it to be 255 or less
    setWiper(__DIGITALPOT_1_I2C_ADDR, 0, wiperval);
    break;

  case 6: // get v/i from INA219-1
    shuntvoltage = currsense[0]->getShuntVoltage_mV();
    busvoltage = currsense[0]->getBusVoltage_V();
    current_mA = currsense[0]->getCurrent_mA();

    Serial.println("current (mA): " + String(current_mA / 100));
    Serial.print("Bus Voltage (V):   ");
    Serial.println(busvoltage);
    Serial.print("Shunt Voltage (mV): ");
    Serial.println(shuntvoltage);
    break;

  case 7: // disconnect from bus
    if (disconnect_i2c_bus(current_LTC_addr) != 0)
    {
      Serial.println("Failed command LTC addr " + String(current_LTC_addr) + " to disconnect");
    }
    else
    {
      IO_EXPANDER_PINSTATE = 0;
    }
    break;

  case 8: // set io expander pin
    pin = cons->getInt("pin (0-n)? ");
    state = cons->getInt("state (0 for low, 1 for HIGH)? ");
    status = setExpander(pin, state);
    Serial.print("what we think the expander state is: ");
    Serial.println(IO_EXPANDER_PINSTATE, BIN);
    Serial.println("expander iic status = " + String(status));
    break;

  case 9: // set wiper
    pot = cons->getInt("Which Digital Pot (1 or 2)? ");
    wiper = cons->getInt("which wiper (1-4)? ");
    if (wiper < 1)
      wiper = 1;
    wiperval = cons->getInt("value? 0 to 255: ") & 0xFF; // get number and force it to be 255 or less
    if (pot == 2)
    {
      Serial.println("Setting Digital pot 2, wiper " + String(wiper) + " to " + String(wiperval));
      setWiper(__DIGITALPOT_2_I2C_ADDR, wiper - 1, wiperval);
    }
    else
    {
      Serial.println("Setting Digital pot 1, wiper " + String(wiper) + " to " + String(wiperval));
      setWiper(__DIGITALPOT_1_I2C_ADDR, wiper - 1, wiperval);
    }

    break;
  case 10: // sweep wiper n on digital pot n
    pot = cons->getInt("Which Digital Pot (1 or 2)? ");
    wiper = cons->getInt("which wiper (1-4)? ");
    if (wiper < 1)
      wiper = 1;

    cons->getInt("To stop, press any key / press enter\r\npress enter to begin");
    wiperval = 0;
    while (1)
    {
      if (pot == 2)
      {
        setWiper(__DIGITALPOT_2_I2C_ADDR, wiper - 1, wiperval);
      }
      else
      {
        setWiper(__DIGITALPOT_1_I2C_ADDR, wiper - 1, wiperval);
      }

      delay(200);
      wiperval++;

      if (wiperval >= 255)
        wiperval = 0;

      if (Serial.available() > 0)
        return;
    }
    break;
  case 11: // init the rest of the INA219s
    for (int i = 1; i < 8; i++)
    {
      currsense[i] = new Adafruit_INA219(__CURR_SENSE_BASE_I2C_ADDR + i);
      currsense[i]->begin();
    }
    break;

  case 12: // get I+V reading
    ina = cons->getInt("which Current Senese Chip (1-8)? ");
    if (ina < 1)
      ina = 1;
    if (ina > 8)
      ina = 8;

    if (currsense[ina - 1] == 0)
    {
      Serial.println("Please initialize the INA219's first");
      return;
    }
    shuntvoltage = currsense[ina - 1]->getShuntVoltage_mV();
    busvoltage = currsense[ina - 1]->getBusVoltage_V();
    current_mA = currsense[ina - 1]->getCurrent_mA();
    Serial.println("get IV of INA219-" + String(ina));
    Serial.println("current (mA): " + String(current_mA / 100));
    Serial.print("Bus Voltage (V):   ");
    Serial.println(busvoltage);
    Serial.print("Shunt Voltage (mV): ");
    Serial.println(shuntvoltage);
    break;
  case 99:
    IO_EXPANDER_PINSTATE = 0xFFFF;
    Wire.beginTransmission(__IO_EXPANDER_I2C_ADDR);
    Wire.write(0xFF);
    Wire.write(0xFF);
    Serial.println("expander status = " + String(Wire.endTransmission()));
    break;
  case 100:
    IO_EXPANDER_PINSTATE = 0x0000;
    Wire.beginTransmission(__IO_EXPANDER_I2C_ADDR);
    Wire.write(0x0);
    Wire.write(0x0);
    Serial.println("expander status = " + String(Wire.endTransmission()));
    break;
  }
}

int setWiper(uint8_t dpot_i2c_addr, uint8_t wiper, uint8_t val)
{
  int status = 0;

  Wire.beginTransmission(dpot_i2c_addr);
  Wire.write(__AD5144_CMD_WRITE_RDAC + wiper); // Write to wiper/RDAC1
  Wire.write(val);                             // write value to chip

  status = Wire.endTransmission();
  if (status != 0)
    Serial.println("\r\nError, couldn't communicate with AD5144 chip over i2c status=" + String(status) + "\r\n");

  return status;
}
