/**
 * @file arduino_based_testcode
 * @author carobers@asu.edu
 * @brief ALPACA Bias Board Control and Test Code
 * @date 2023-05-05
 * 
 * @copyright Copyright (c) 2022 Arizona State University
 *
 *
 * Used to talk to an INA219 current/voltage sense chip and control a dc bias via an ad5144 digital potentiometer
 * 
 * REVISIONS:
 *  2022-03-07
 *      FILE CREATED
 *  2023-03-15
 *    Updated to utilize the LTC4302-A ii2 bus repeater. It is assumed that all code needs to communicate through the LTC.
 *    User needs to supply the LTC Address for each command. 
 *  2023-05-05
 *    Updated header and modified code to reflect "ALPACA v2.0 rev2 Test Plan" document revison 2.1
 *   
 *    
 *  
 *
 */

#include <Arduino.h>
#include "TuiConsole.h"
#include <Adafruit_INA219.h>
#include <Wire.h>
#define SW_VERSION_NUMBER "0.2"


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

// send the 'connect bus' command to the repeater
// expects address to be between 0 and 31
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
  IO_EXPANDER_PINSTATE ^= (-state ^ IO_EXPANDER_PINSTATE) & (1 << pin);
  Wire.beginTransmission(__IO_EXPANDER_I2C_ADDR);
  Wire.write(IO_EXPANDER_PINSTATE);
  return Wire.endTransmission();
}

void loop(){
    while (Serial.available() > 0) 
        Serial.read();
    
    Serial.println("(built: " + String(__DATE__) + "_" + String(__TIME__) + "_v" + SW_VERSION_NUMBER + "\r\n");
    Serial.println("Select Option:\r\n      1. begin_comms\r\n      2. en_dpots\r\n      3. en_v1\r\n      4. sweep_wiper\r\n      5. set_wiper\r\n      6. get_iv\r\n      7. end_comms\r\n    ");
    int cmd = cons->getInt("\r\noption: ");
    int wiper = 0, wiperval = 0, status = -1;
    float cur = 0, vol = 0;
    int addr = 0;
    double shuntvoltage = 0;
    double busvoltage = 0;
    double current_mA = 0;
    switch(cmd)
    {
      case 1: //begin_comms
        addr = cons->getInt("iic_address?");
        if (connect_i2c_bus(addr) == 0){
          if (setExpander(0, HIGH) != 0){
            Serial.println("I2C Error: Couldn't command IO expander");
            return;
          } else {
            if (! currsense_1.begin()) {
              Serial.println("Failed to find INA219 chip");
            }
          }

        } else {
          Serial.println("I2c Error: LTC4302, Couldn't tell it to connect the bus");
        }
        break;
      case 2: //en_dpots
        if (setExpander(0, HIGH) != 0){
          Serial.println("I2C Error: Couldn't command IO expander");
        }
        break;
      case 3: //enable v1
        if (setExpander(1, HIGH) != 0){
          Serial.println("I2C Error: Couldn't command IO expander");
        }
        break;
      case 4: //sweep wiper ( sweep digital pot 1 wiper 1)
        cons->getInt("To stop, press any key / press enter\r\npress enter to begin");
        while(1){
          setWiper(__DIGITALPOT_1_I2C_ADDR, 0, wiperval);
          delay(10);
          wiperval++;

          if (wiperval == 255)
            wiperval=0;
          
          if(Serial.available() > 0)
            return;
            
        }
        break;
      case 5: //set_wiper (digital pot 1 wiper 1)
        wiperval = cons->getInt("0 to 255") & 0xFF; //get number and force it to be 255 or less
        setWiper(__DIGITALPOT_1_I2C_ADDR, 0, wiperval);
        break;
      
      case 6: // get v/i from INA219
        shuntvoltage = currsense_1.getShuntVoltage_mV();
        busvoltage = currsense_1.getBusVoltage_V();
        current_mA = currsense_1.getCurrent_mA();

        Serial.println("current (mA): " + String(current_mA));
        Serial.print("Bus Voltage (V):   "); Serial.println(busvoltage);
        Serial.print("Shunt Voltage (mV): "); Serial.println(shuntvoltage); 
        Serial.print("v_drop (mV): "); Serial.println((busvoltage-(shuntvoltage/1000))); 
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


