# Tilt compensated Digital Compass Display


<img src="https://github.com/visakhanc/eCompass/blob/master/pic/3.jpg">


This is an easy to build eCompass Display which shows the tilt compensated Heading(Azimuth angle) along with the Pitch and Roll angles on an OLED display. It is based on AVR ATmega8 microcontroller and uses HMC5883L magnetometer module, Accelerometer sensor from MPU6050 module and a 128x64 SSD1306 OLED display. These 3 modules are connected to the single I2C bus of the AVR. So, the connections are very minimal. The AVR runs on the internal 8MHz clock. The circuit can be powered from a 3.7V Li-Ion cell or any source giving 3.3V to 5V.


Building of Source
------------------

1. Download this repository and the [common_libs](https://github.com/visakhanc/common_libs) repository to the same directory. 

2. Rename *'common_libs'* directory to *'common'*. 

For example, if you copied the repositories to *'AVR_Projects'* directory, the directory structure would be like:


	AVR_Projects/
	  |
	  |--common/
	  |
	  |--eCompass/
	

3. On the command line go into the eCompass/source directory and type 

	`make all`

   This would generate *main.hex* file which needs to be programmed to the AVR.



Construction
------------

Here are the required components:

1. AVR ATmega8 28-pin DIP IC
2. HMC5883L Digital Compass module (with 3.3V regulator)
3. MPU6050 Accelerometer+Gyroscope module (with 3.3V regulator)
4. SSD1306 128x64 OLED Display module (I2C) (with 3.3V regulator)
5. Capacitores: 0.1uF Ceramic Disk x 2
6. Resitors: 10k, 1k
7. LED (optional - only for debugging)
8. 3.3V-5V power supply (A 3.7V Li-Ion cell will do)
9. Female Berg strip for 4-pin Display connector and 6-pin ISP connector

The components can be soldered on a veroboard or perfboard, based on the [schematic](https://github.com/visakhanc/eCompass/blob/master/circuit/circuit.png). See these [pictures](https://github.com/visakhanc/eCompass/tree/master/pic) for some reference.

**NOTE:** The HMC5883L module and MPU6050 module should be aligned to each other. That is, the X-axis of both module should be parallel.

After the connections are completed, connect an ISP programmer and programm the AVR with the *main.hex* file. The fuse should be modified for 8MHz internal oscillator. If you are using USBasp programmer, simply type the following commands on the command line:

1. *make fuse*
2. *make program*

**NOTE:** Please remember to disconnect the Li-Ion battery, if programmer is supplying 5V to the AVR.

Operation
---------

HMC5883L digital compass module gives 3-axis magnetic field component values, which can be used to determine the Azimuth angle (Heading). However, this method will give accurate Heading only when the module is held horizontal to the gound.

That is, when the module is tilted, the calculated Heading value will be incorrect. To compensate for the tilt, an accelerometer sensor is used to calculate the tilt angles (Roll and Pitch). The tilt angles are applied to a correction formula to obtain the accurate Heading value. Here, accelerometer values are read from MPU6050 Accelerometer+Gyroscope module.




