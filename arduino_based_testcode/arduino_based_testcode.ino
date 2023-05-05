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
 *    Updated descriptions, Uo
 *   
 *    
 *  
 *
 */

#include <Arduino.h>
#include "TuiConsole.h"
#include <Adafruit_INA219.h>
#include <Wire.h>
#define SW_VERSION_NUMBER "2."

Adafruit_INA219 ina219;
TuiConsole *cons;

/**  spec'd from AD5144 datasheet
 * SEE: https://www.analog.com/media/en/technical-documentation/data-sheets/AD5124_5144_5144A.pdf
 **/

#define AD5144_I2C_ADDR 0b0101111 //ADDR 0 -> GND   ADDR 1 -> GND (AAD5144 DATASHEET)
#define AD5144_CMD_WRITE_RDAC 0b00010000 //offset +1 for each RDAC register
#define DEFAULT_POT_WIPER 0

#define MIN_CURR_TOLERANCE 0.001 //minimum acceptable difference between desired and realized bias current in AMPS
#define MIN_VOLT_TOLERANCE 0.010 //minimum acceptable difference between desired and realized bias current


int setWiper(uint8_t wiper, uint8_t val);
void printCV();
float getLoadV();
float getCurrent();


int connect_i2c_bus(uint8_t address){
  Wire.beginTransmission(address); 
  Wire.write(0b11100000); //Connect IIC Bus
  return Wire.endTransmission();
}
int disconnect_i2c_bus(uint8_t address){
  Wire.beginTransmission(address); 
  Wire.write(0b01100000); //Connect IIC Bus
  return Wire.endTransmission();
}
void setup(){
    cons = new TuiConsole(&Serial, 9600); //Setup Serial Console

    // Toggle () pin
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
    
    // Toggle () pin
    pinMode(3, OUTPUT);
    digitalWrite(3, HIGH);

    Wire.begin(); //start I2C bus
    ina219.begin(); //Default current/voltage sensor init
    
    setWiper(DEFAULT_POT_WIPER, 0);
}

void loop(){
    while (Serial.available() > 0) 
        Serial.read();
    
    Serial.println("(built: " + String(__DATE__) + "_" + String(__TIME__) + "_v" + SW_VERSION_NUMBER + "\r\n");
    Serial.println("Select Option:\r\n1. Read Voltage(V),Current(mA)\r\n2. Set Current\r\n3. Set Voltage\r\n4. Test LTC4302");
    int cmd = cons->getInt("\r\noption: ");
    int wiper = 0, wiperval = 0, status = -1;
    float cur = 0, vol = 0; //desired current
    int addr = 0;

    switch (cmd)
    {
    case 1: // REad out Voltage, current
        addr = cons->getInt("I2C Repeater Address?");
        connect_i2c_bus(addr);
        printCV();
        disconnect_i2c_bus(addr);
        break;
    case 2: //set and keep current. 
        /**
         * This will probably have to be it's own loop until broken out by user input
         *  current seaking algorithm....
         *  Basically need to set a wiper value, check the current and adjust accordingly
         *  S1, should set wiper to _, to represent the minumum current
         *  S2. climb up/down the wiper value until we are within some tolerance of the desired 'Current' value
         *  S3. break out
         * 
         */
        addr = cons->getInt("I2C Repeater Address?");
        connect_i2c_bus(addr);
        cur = (float) cons->getDouble("current (mA): ");
        setWiper(DEFAULT_POT_WIPER, 0); //START AT MIN VAL
        Serial.println("\r\n");
        for (int i = 0; i < 256; i++){
            float x = 0;
            status = setWiper(DEFAULT_POT_WIPER, i);
            x = getCurrent();

            if (status == 0){
                Serial.println("I=" + String(x) +"\r");
                if (abs(cur-x) <= MIN_CURR_TOLERANCE){
                    Serial.println("\r\nfound setting, diff=" + String(abs(cur-x)) + "mA measured=" + String(x) +  " mA; wiper=" + String(i) + "\r\n");
                    break;
                }
            }
            
            delay(100);
            //break out of this thing and return to menu if anything is sent over serial
            if(Serial.available() > 0){
                disconnect_i2c_bus(addr);
                return;
            }
        
            
        }
        disconnect_i2c_bus(addr);
        break;
    
    case 3: //set and keep voltage
        addr = cons->getInt("I2C Repeater Address?");
        connect_i2c_bus(addr);
        vol = (float) cons->getDouble("voltage (V): ");
        setWiper(DEFAULT_POT_WIPER, 0); //START AT MIN VAL
        Serial.println("\r\n");
        for (int i = 0; i < 256; i++){
            float x = 0;
            status = setWiper(DEFAULT_POT_WIPER, i);
            x = getLoadV();

            if (status == 0){
                Serial.println("V=" + String(x) +"\r");
                if (abs(vol-x) <= MIN_VOLT_TOLERANCE){
                    Serial.println("\r\nfound setting, diff=" + String(abs(vol-x)) + "V measured=" + String(x) +  " V; wiper=" + String(i) + "\r\n");
                    break;
                }
            }
            
            delay(100);
            //break out of this thing and return to menu if anything is sent over serial
            if(Serial.available() > 0){
                disconnect_i2c_bus(addr);
                return;
            }
        
            
        }
        disconnect_i2c_bus(addr);
        break;
    case 4:
      addr = cons->getInt("I2C Repeater Address?");

      Serial.println("Test to connect i2c bus");
      status = connect_i2c_bus(addr);
      if (status != 0){
        Serial.println("FAIL, CODE=" + String(status));
      } else {
        Serial.println("SUCCESS");
      }
      
      Serial.println("Test to d/c i2c bus");
      status = disconnect_i2c_bus(addr);
      if (status != 0){
        Serial.println("FAIL, CODE=" + String(status));
      } else {
        Serial.println("SUCCESS");
      }
      break;
    default:
        Serial.println("\r\n");
        break;
    }

}

int setWiper(uint8_t wiper, uint8_t val){
    int status = 0;
    
    Wire.beginTransmission(AD5144_I2C_ADDR); 
    Wire.write(AD5144_CMD_WRITE_RDAC + wiper); //Write to wiper/RDAC1
    Wire.write(val); //write value to chip
    
    status = Wire.endTransmission();
    if(status != 0)
        Serial.println("\r\nError, couldn't communicate with AD5144 chip over i2c status=" + String(status) + "\r\n");

    return status;
    
}

float getCurrent(){
    return ina219.getCurrent_mA();
}

float getLoadV(){
    float shuntvoltage = 0,
    busvoltage = 0,
    loadvoltage = 0;

    shuntvoltage = ina219.getShuntVoltage_mV();
    busvoltage = ina219.getBusVoltage_V();
    loadvoltage = (busvoltage + (shuntvoltage / 1000));
    return loadvoltage;
}

// Print voltage,current in volts,miliAmp\n format
void printCV(){
    Serial.println("\r\n" + String(getLoadV()) + "," + String(getCurrent()) + "\r\n");
}
