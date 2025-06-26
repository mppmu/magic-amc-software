Software for the MAGIC Active Mirror Control (AMC)
==================================================

## Dependencies

In order to compile and run the AMC software and the SBIG software, you need to
install the following dependencies. In case you run the software inside a
container, some of these dependencies must be installed in the container while
other ones need to be installed on the host system.  

The following instructions have been tested with Ubuntu 24.04 64-bit as host
system and Ubuntu 18.04 64-bit with 32-bit software enabled as system inside
the container.
[Apptainer](https://apptainer.org/ "Apptainer is an open source container platform")
was used as container platform.


### Udev Rules

* The udev rules are located in the directory ```depend/udev``` of
  this repository.
* If You run the software inside a container, the udev rules must be copied to
  the udev rules directory of the **host** system.
* Copy the file ```51-sbig-debian.rules``` into the directory containing the
  udev rules. In Ubuntu 22.04, this is the directory ```/etc/udev/rules.d```.


### SBIG Firmware

* The SBIG firmware files are located in the directory
  ```depend/sbig/firmware``` of this repository.
* If You run the software inside a container, the SBIG firmware files must be
  copied to the firmware directory of the **host** system.
* Copy all files to the firmware directory ```/lib/firmware```. 


### SBIG Libraries

* The SBIG libraries are located in the directory
  ```depend/lib``` of this repository.
* If You run the software inside a container, the SBIG libraries must be
  copied to the **container** system.
* Copy the files ```libsbigcam.a``` and ```libsbigcam.so``` to
  ```/usr/local/lib```.
* If packages of ```libsbigudrv``` are available for your Linux distribution,
  it is highly recommended to use these. Install the library and its
  development package. Please note that the 32-bit version is required.
  In Ubuntu 18.04, run these commands:
  ```
  apt install libsbigudrv2:i386
  apt install libsbigudrv2-dev:i386
  ```
* If no packages of ```libsbigudrv``` are available, copy the files
  ```libsbigudrv.a``` and ```libsbigudrv.so``` to ```/usr/local/lib```.


### Build tools and auxiliary packages.

* If You run the software inside a container, all build tools and auxiliary
  packages must be installed in the **container** system.
* For Ubuntu 18.04, run these commands:
  ```
  apt install autoconf
  apt install autogen
  apt install automake
  apt install build-essential
  apt install git
  apt install make
  apt install libcfitsio-dev
  apt install libforms-dev:i386
  apt install libtool
  apt install libturbo8_i386
  apt install libusb-1.0-0-dev
  apt install libxpm-dev:i386
  apt install libxpm4:i386
  apt install python3
  apt install python3-serial
  apt install saods9
  apt install xpa-tools
  ```



## AMC Software

* To build the AMC software, run these commands:
  ```
  cd amc
  make mrproper && make all
  ```
* Before runninge the AMC software:
  - Copy the LUT files into the ```amc``` directory.
  - Create a link inside the ```amc``` directory with the name
    ```test_Panels1.txt``` to a valid AMC panel list file, e.g.
    ```AMC1_panel_list.txt```.
  - Create the links ```/home/operator/log1``` and ```/home/operator/sbig1```
    pointing to valid directories for storing the log files and SBIG images.
* To run the AMC software, cd into the ```amc``` directory and run:
  ```
  ./amc
  ```



## SBIG Software

* To build the SBIG software, run these commands:
  ```
  cd sbig
  make mrproper && make all
  ```
* To run the AMC software, cd into the ```sbig``` directory and run:
  ```
  ./sbigab
  ```



## AMC-Test Software

The simple python program ```amc_test.py``` can be used to send individual
commands to the AMC system. It sends hexadecimal commands to the AMC controller
board (AMContr_R3).  

__Caution: The firmware version 6 (2.1.6) is required on the AMC controller!__


### Examples

Send the standard command ("center" = 0x07) over the standard serial port
```/dev/ttyS0``` to the standard AMC controller (with address 0x65) and select
the standard stepper motor driver card (0).
```
./amc_test.py
```

Send the standard command ("center" = 0x07) over the serial port
```/dev/ttyS1``` to the AMC controller with address 40 and select the stepper
motor driver card 1.
```
./amc_test.py -d /dev/ttyS1 -a 40 -b 1
```

Move relative in X direction:
```
./amc_test.py -c move_rel_x 100
```
```
./amc_test.py -c move_rel_x -100
```
```
./amc_test.py -d /dev/ttyS1 -a 40 -b 1 -c move_rel_x 0x1234
```

Move relative in Y direction:
```
./amc_test.py -c move_rel_y 100
```
```
./amc_test.py -c move_rel_y -100
```
```
./amc_test.py -d /dev/ttyS1 -a 40 -b 1 -c move_rel_y 0x1234
```

Move relative in X and in Y direction:
```
./amc_test.py -c move_rel_xy 100 -200
```

Move absolute in X and in Y direction:
```
./amc_test.py -c move_abs 1000 -2000
```

Stop moving:
```
./amc_test.py -c stop
```

Show detailed status:
```
./amc_test.py -v 3 -c status
```

Send the raw command 0x07 = "center" over the serial port ```/dev/ttyS1``` with
host computer ID 0xf7 to the AMC controller with address 0x65 (101) and select
stepper motor driver 0. Produce verbose output.
```
./amc_test.py -d /dev/ttyS1 -a 0x65 -b 0 -i 0xf7 -v 3 -c raw 0x7
```

