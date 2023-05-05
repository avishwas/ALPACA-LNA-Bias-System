/**
 * @file TuiConsole.cpp
 * @author Cody Roberson
 * @brief Implements the TUI Console
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
#include "TuiConsole.h"

TuiConsole::TuiConsole(Serial_ *serialPort, unsigned int baudrate)
{
    this->serialPort = serialPort;
    this->baudrate = baudrate;

    this->serialPort->begin(this->baudrate);
}

TuiConsole::~TuiConsole()
{
    this->serialPort->end();
}


String TuiConsole::getRawInput(const int charset)
{
    String str = "";
    char c = 0;
    size_t len = 0;

    // Wait until serial port contains input
    while (serialPort->available() == 0); 
    
    // Loop character by character until a newline or a carriage return is found
    while (c != '\n' && c != '\r')
    {   
        if (serialPort->available() > 0) 
        {
            c = serialPort->read();
            len = str.length();

            //Handle backspacing if there is currently no input
            if (len == 0 && (c == 8 || c == 127))
            {   
                c = 0;
                continue; 
            }

            //Handle backspacing if there is input
            if (c == 8 || c == 127)
            {
                
                str.remove(len-1);
                c = 8; // if char is del instead of backspace, treat it as a backspace
                if (echoEnabled)
                {
                    serialPort->print(c);
                    serialPort->print(' ');
                    serialPort->print(c);
                }
                
                c = 0;
                continue;
            }

            // filter for and accept chosen set of 'charset characters'
            switch (charset)
            {
            case TUICONSOLE_CHARSET_ALPHA:
                if (c == ' ' ||
                    (c >= 'a' && c <= 'z')||
                    (c >= 'A' && c <= 'Z')){
                    if (echoEnabled)
                        serialPort->print(c);
                    str.concat(c);
                    c = 0;
                }
                break;
            
            case TUICONSOLE_CHARSET_ALPHA_NUMERIC:
                if (c == '.' || c == ' ' || c == '-' || c == ':' ||
                    (c >= 'a' && c <= 'z')||
                    (c >= 'A' && c <= 'Z')||
                    (c >= '0' && c <= '9')){
                    if (echoEnabled)
                        serialPort->print(c);
                    str.concat(c);
                    c = 0;
                }
                break;
            
            case TUICONSOLE_CHARSET_NUMERIC_DECIMAL:
                if (c == '-' || c == '.' || (c >= '0' && c <= '9')){
                    if (echoEnabled)
                        Serial.print(c);
                    str.concat(c);
                    c = 0;
                }
                break;
            
            case TUICONSOLE_CHARSET_NUMERIC_INTEGER:
                if (c == '-' || (c >= '0' && c <= '9')){
                    if (echoEnabled)
                        Serial.print(c);
                    str.concat(c);
                    c = 0;
                }
                break;
            default:
                break;
            }
        }
    }
    return str;
}


String TuiConsole::getAlphaString(String prompt)
{
    serialPort->print(prompt);
    String str = getRawInput(TUICONSOLE_CHARSET_ALPHA);
    if (echoEnabled)
        serialPort->println("");
    return str;
}

String TuiConsole::getString(String prompt)
{
    serialPort->print(prompt);
    String str = getRawInput(TUICONSOLE_CHARSET_ALPHA_NUMERIC);
    if (echoEnabled)
        serialPort->println("");
    return str;
}

int TuiConsole::getInt(String prompt)
{
    serialPort->print(prompt);
    String str = getRawInput(TUICONSOLE_CHARSET_NUMERIC_INTEGER);
    if (echoEnabled)
        serialPort->println("");
    return str.toInt();
}

double TuiConsole::getDouble(String prompt)
{
    serialPort->print(prompt);
    String str = getRawInput(TUICONSOLE_CHARSET_NUMERIC_DECIMAL);
    if (echoEnabled)
        serialPort->println("");
    return str.toDouble();
}

String TuiConsole::getAlphaString()
{
    String str = getRawInput(TUICONSOLE_CHARSET_ALPHA);
    if (echoEnabled)
        serialPort->println("");
    return str;
}

String TuiConsole::getString()
{
    String str = getRawInput(TUICONSOLE_CHARSET_ALPHA_NUMERIC);
    if (echoEnabled)
        serialPort->println("");
    return str;
}

int TuiConsole::getInt()
{
    String str = getRawInput(TUICONSOLE_CHARSET_NUMERIC_INTEGER);
    if (echoEnabled)
        serialPort->println("");
    return str.toInt();
}

double TuiConsole::getDouble()
{
    String str = getRawInput(TUICONSOLE_CHARSET_NUMERIC_DECIMAL);
    if (echoEnabled)
        serialPort->println("");
    return str.toDouble();
}

