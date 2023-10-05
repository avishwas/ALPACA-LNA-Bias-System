# ALPACA-LNA-Bias-System
<p style="text-align: center" align="center">
<a href="https://github.com/psf/black"><img src="https://img.shields.io/badge/code%20style-black-000000.svg"></a>
</p>

___
## General Information
This repository contains the documentation and software for the VME cards designed for the Series 2151 VME backplane / enclosure. This repo shall be made available to our collaborators.

**[--> Read the Docs here <--](https://asu-astronomical-instrumentation.github.io/ALPACA-LNA-Bias-System/)**

<p>&nbsp</p>

### Hardware
___
Utilizes an Odroid-N2 as the computer/controller. Images and Digrams are sourced from 
<a href="https://wiki.odroid.com/odroid-n2/hardware/expansion_connectors">the Odroid Wiki</a>.

<a href="https://wiki.odroid.com/odroid-n2/hardware"><img src="https://wiki.odroid.com/_media/odroid-n2/n2plusdetail.jpg?w=980&tok=3b09a3" /></a>

<a href="https://wiki.odroid.com/odroid-n2/hardware"><img src="https://wiki.odroid.com/_media/odroid-n2/hardware/n2_pinmap.png?w=700&tok=c17874" /></a>

<p>&nbsp</p>

| GPIO | WiringPi | Name      | Mode | Initial Level | Header Pin | Header Pin | Initial Level | Mode | Name      | WiringPi | GPIO    |
| ---- | -------- | --------- | ---- | ------------- | ---------- | ---------- | ------------- | ---- | --------- | -------- | ------- |
|      |          | 3.3V      |      |               | 1          | 2          |               |      | 5v        |          |         |
| 493  | 8        | I2C.SDA0  | IN   | 0             | 3          | 4          |               |      | 5v        |          |         |
| 494  | 9        | I2C.SCL0  | IN   | 1             | 5          | 6          |               |      | GND       |          |         |
| 473  | 7        | GPIO. 07  | IN   | 0             | 7          | 8          | 1             | IN   | TxD       | 15       | 488     |
|      |          | GND       |      |               | 9          | 10         | 1             | IN   | TxR       | 16       | 489     |
| 479  | 0        | GPIO.00   | IN   | 1             | 11         | 12         | 1             | IN   | PCM/PWM   | 1        | 492     |
| 480  | 2        | GPIO. 02  | IN   | 1             | 13         | 14         |               |      | GND       |          |         |
| 483  | 3        | GPIO. 03  | IN   | 1             | 15         | 16         | 1             | IN   | GPIO. 04  | 4        | 476     |
|      |          | 3.3v      |      |               | 17         | 18         | 1             | IN   | GPIO. 05  | 5        | 477     |
| 484  | 12       | SPIO_MOSI | IN   | 1             | 19         | 20         |               |      | GND       |          |         |
| 485  | 13       | SPIO_MISO | IN   | 1             | 21         | 22         | 1             | IN   | SPI_CE0   | 6        | 478     |
| 487  | 14       | SPI_CLK   | IN   | 1             | 23         | 24         | 1             | IN   | SPI_CE1   | 10       | 486     |
|      |          | GND       |      |               | 25         | 26         | 0             | IN   | GPIO. 11  | 11       | 464     |
| 474  | 30       | I2C.SDA1  | IN   | 1             | 27         | 28         | 1             | IN   | I2C_.SCL1 | 31       | 475     |
| 490  | 21       | GPIO. 21  | IN   | 1             | 29         | 30         |               |      | GND       |          |         |
| 491  | 22       | GPIO. 22  | IN   | 1             | 31         | 32         | 0             | IN   | GPIO. 26  | 26       | 472     |
| 481  | 23       | GPIO. 23  | IN   | 1             | 33         | 34         |               |      | GND       |          |         |
| 482  | 24       | GPIO. 24  | IN   | 0             | 35         | 36         |               | IN   | GPIO. 27  | 27       | 495     |
| AIN3 | 25       | GPIO. 25  |      |               | 37         | 38         |               |      | GPIO. 28  | 28       | REF 1.8 |
|      |          | GND       |      |               | 39         | 40         |               |      | GPIO. 29  | 29       | AIN2    |

<p>&nbsp</p>

### Software
___
The software shall consist of a simple wiring-pi based software
library. The Python code shall utilize the Black styling standard. Users can program as normal and then invoke 'black' to style their code aftwerwards. 

<p>&nbsp</p>


### Commits
___
**Repository is an as is product going forward and will ignore pull requests.**
