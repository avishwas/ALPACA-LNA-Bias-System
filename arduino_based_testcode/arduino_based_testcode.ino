/**
 * @file arduino_based_testcode
 * @author carobers@asu.edu
 * @brief ALPACA Bias Board Control and Test Code
 * @date 2023-05-05
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
 *   
 *    
 *  
 *
 */

#include <Arduino.h>
#include "TuiConsole.h"
#include <Adafruit_INA219.h>
#include <Wire.h>
#define SW_VERSION_NUMBER "0.3"


// *********** I2C DEVICE ADDRESSES *****************
#define __DIGITALPOT_1_I2C_ADDR  0b0101111
#define __DIGITALPOT_2_I2C_ADDR  0b0100011
#define __I2C_REPEATER_I2C_ADDR  0b1100000
#define __IO_EXPANDER_I2C_ADDR   0b0100000
#define __CURR_SENSE_1_I2C_ADDR  0b1001000 

// *********** I2C DEVICE COMMANDS *****************
#define __AD5144_CMD_WRITE_RDAC          0b00010000
#define __LTC4302_CMD_CONNECT_CARD       0b11100000
#define __LTC4302_CMD_DISCONNECT_CARD    0b01100000

// ************ OTHER DEFAULT PARAMETERS ******************



// Forward declarations telling the compiler that the code exists, but at the end
int setWiper(uint8_t dpot_i2c_addr, uint8_t wiper, uint8_t val);

//send the 'connect bus' command to the register.
int connect_i2c_bus(uint8_t address){
  Wire.beginTransmission(__I2C_REPEATER_I2C_ADDR + address); 
  Wire.write(__LTC4302_CMD_CONNECT_CARD); //Connect IIC Bus
  return Wire.endTransmission();
}

// send the 'disconnect bus' command to the repeater
int disconnect_i2c_bus(uint8_t address){
  Wire.beginTransmission(__I2C_REPEATER_I2C_ADDR + address); 
  Wire.write(__LTC4302_CMD_DISCONNECT_CARD);
  return Wire.endTransmission();
}

//global variables
uint16_t IO_EXPANDER_PINSTATE = 0;
Adafruit_INA219 currsense_1(__CURR_SENSE_1_I2C_ADDR);
TuiConsole *cons;


void setup(){
    cons = new TuiConsole(&Serial, 9600); //Setup Serial Console
    Wire.begin(); //start/setup i2c arduino interface
}

int setExpander(int pin, int state){
  //FIXME:
  // IO_EXPANDER_PINSTATE ^= (-state ^ IO_EXPANDER_PINSTATE) & (1 << pin);
  // Wire.beginTransmission(__IO_EXPANDER_I2C_ADDR);
  // Wire.write(IO_EXPANDER_PINSTATE);

  // if (state){
  //   IO_EXPANDER_PINSTATE |= (1<<pin);
  //   Serial.println(IO_EXPANDER_PINSTATE, BIN);
  // } else {
  //   IO_EXPANDER_PINSTATE = IO_EXPANDER_PINSTATE & ();

  // }
  // return Wire.endTransmission();
  return 0;
}

void loop(){
    while (Serial.available() > 0) 
        Serial.read();
    
    Serial.println("\r\nALPACA BIAS BOARD TEST CODE (built: " + String(__DATE__) + "_" + String(__TIME__) + "_v" + SW_VERSION_NUMBER + ")\r\n");
    Serial.println("Select Option:\r\n      1. begin_comms\r\n      2. en_dpots\r\n      3. en_v1\r\n      4. sweep_wiper\r\n      5. set_wiper\r\n      6. get_iv\r\n      7. end_comms\r\n    ");
    Serial.println("      99. set all expander1 pins high\r\n      100. set all expander1 pins low\r\n");
    int cmd = cons->getInt("\r\noption: ");
    int wiper = 0, wiperval = 0, status = -1;
    float cur = 0, vol = 0;
    int addr = 0;
    double shuntvoltage = 0;
    double busvoltage = 0;
    double current_mA = 0; //current in miliamps from INA219-1
    switch(cmd)
    {
      case 1: //begin_comms
        addr = cons->getInt("iic_address?: ");
        if (connect_i2c_bus(addr) == 0){
          Wire.beginTransmission(__IO_EXPANDER_I2C_ADDR);
          Wire.write(0);
          Wire.write(0);
          Serial.println("expander status = " + String(Wire.endTransmission()));
          if (! currsense_1.begin()) {
            Serial.println("Failed to find INA219 chip");
          }
        } else {
          Serial.println("I2c Error: LTC4302, Couldn't tell it to connect the bus");
        }
        break;
      case 2: //en_dpots
          Wire.beginTransmission(__IO_EXPANDER_I2C_ADDR);
          Wire.write(0b00000001);
          Wire.write(0);
          Serial.println("expander status = " + String(Wire.endTransmission()));

        break;
      case 3: //enable v1
          Wire.beginTransmission(__IO_EXPANDER_I2C_ADDR);
          Wire.write(0b00000011);
          Wire.write(0);
          Serial.println("expander status = " + String(Wire.endTransmission()));
        break;
      case 4: //sweep wiper ( sweep digital pot 1 wiper 1)
        cons->getInt("To stop, press any key / press enter\r\npress enter to begin");
        while(1){
          setWiper(__DIGITALPOT_1_I2C_ADDR, 0, wiperval);
          delay(100);
          wiperval++;

          if (wiperval == 255)
            wiperval=0;
          
          if(Serial.available() > 0)
            return;
            
        }
        break;
      case 5: //set_wiper (digital pot 1 wiper 1)
        wiperval = cons->getInt("value? 0 to 255: ") & 0xFF; //get number and force it to be 255 or less
        setWiper(__DIGITALPOT_1_I2C_ADDR, 0, wiperval);
        break;
      
      case 6: // get v/i from INA219
        shuntvoltage = currsense_1.getShuntVoltage_mV();
        busvoltage = currsense_1.getBusVoltage_V();
        current_mA = currsense_1.getCurrent_mA();

        Serial.println("current (mA): " + String(current_mA/100));
        Serial.print("Bus Voltage (V):   "); Serial.println(busvoltage);
        Serial.print("Shunt Voltage (mV): "); Serial.println(shuntvoltage); 
        break;
      case 99:
          Wire.beginTransmission(__IO_EXPANDER_I2C_ADDR);
          Wire.write(0xFF);
          Wire.write(0xFF);
          Serial.println("expander status = " + String(Wire.endTransmission()));
      break;
      case 100:
          Wire.beginTransmission(__IO_EXPANDER_I2C_ADDR);
          Wire.write(0x0);
          Wire.write(0x0);
          Serial.println("expander status = " + String(Wire.endTransmission()));
      break;
    }


}

int setWiper(uint8_t dpot_i2c_addr, uint8_t wiper, uint8_t val){
    int status = 0;
    
    Wire.beginTransmission(dpot_i2c_addr); 
    Wire.write(__AD5144_CMD_WRITE_RDAC + wiper); //Write to wiper/RDAC1
    Wire.write(val); //write value to chip
    
    status = Wire.endTransmission();
    if(status != 0)
        Serial.println("\r\nError, couldn't communicate with AD5144 chip over i2c status=" + String(status) + "\r\n");

    return status;
    
}


