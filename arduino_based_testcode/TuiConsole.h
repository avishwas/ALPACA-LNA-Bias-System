/**
 * @file TuiConsole.h
 * @author Cody Roberson
 * @brief Main Header for the TuiConsole. The class 'serialConsole' is what shall be used for tui interactions.
 * @version 0.1
 * @date 2021-01-08
 * 
 * @copyright
 * 
 *  Copyright (C) 2021  Cody Roberson
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */
#ifndef _ARDUINOCONSOLE_INCLUDE_TUICONSOLE_H
#define _ARDUINOCONSOLE_INCLUDE_TUICONSOLE_H


#define TUICONSOLE_CHARSET_ALPHA 1 // 'a-z', 'A-Z', ' '
#define TUICONSOLE_CHARSET_NUMERIC_INTEGER 2 // '1-9', '-'
#define TUICONSOLE_CHARSET_NUMERIC_DECIMAL 3 // '1-9', '-', '.'
#define TUICONSOLE_CHARSET_ALPHA_NUMERIC 4 // 'a-z', 'A-Z', ' ', '1-9', '-', '.'


#include <Arduino.h>


class TuiConsole
{
private:
    
    Serial_ *serialPort;
    unsigned int baudrate;
    String getRawInput(const int charset);


public:
/**
 * @brief Construct a new Tui Console object. Note that this 'begins' the serial port.
 * 
 * @param serialPort pointer to which serial port to use, usualy '&Serial'
 * @param baudrate Serial Baud rate to set, usually 9600
 */
    TuiConsole(Serial_ *serialPort, unsigned int baudrate);
    ~TuiConsole();

    /**
     * @brief Gets user input restricted to 'a-z', 'A-Z', ' '
     * 
     * @param prompt (optional) message to print for the user
     * @return String 
     */
    String getAlphaString(String prompt);
    

    /**
     * @brief Gets user input restricted to 'a-z', 'A-Z', ' ', '1-9', '-', '.'
     * 
     * @param prompt (optional) message to print for the user
     * @return String 
     */
    String getString(String prompt);

    /**
     * @brief Gets user input restricted to '1-9', '-'
     * 
     * @param prompt (optional) message to print for the user
     * @return int 
     */
    int getInt(String prompt);
    
    /**
     * @brief Gets user input restricted to '1-9', '-', '.'
     * 
     * @param prompt (optional) message to print for the user
     * @return double 
     */
    double getDouble(String prompt);

  
    //Convenience overloads
    /**
    * @brief Gets user input restricted to 'a-z', 'A-Z', ' '
    * 
    * @param prompt (optional) message to print for the user
    * @return String 
    */
    String getAlphaString();
    /**
     * @brief Gets user input restricted to 'a-z', 'A-Z', ' ', '1-9', '-', '.'
     * 
     * @param prompt (optional) message to print for the user
     * @return String 
     */
    String getString();
    /**
     * @brief Gets user input restricted to '1-9', '-'
     * 
     * @param prompt (optional) message to print for the user
     * @return int 
     */
    int getInt();
    /**
     * @brief Gets user input restricted to '1-9', '-', '.'
     * 
     * @param prompt (optional) message to print for the user
     * @return double 
     */
    double getDouble();

    /**
     * @brief When a character is pressed in a serial terminal, it needs to be echoed back
     * otherwise, the user will see no feedback. This needs to be enabled for standard tui operations.
     * For applications involving software control, this needs to be DISABLED to remove this feedback.
     * 
     * ************************************************
     * Note that this bool currently makes this THREAD-UNSAFE. 
     * ************************************************
     */
    bool echoEnabled = true;

};

#endif