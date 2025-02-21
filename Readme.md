# Software for the MAGIC Active Mirror Control (AMC).

## AMC-Test Software

The simple python program ```amc_test.py``` can be used to send individual
commands to the AMC system. It sends hexadecimal commands to the AMC controller
board (AMContr_R3).  

__Caution: The firmware version 6 (2.1.6) is required on the AMC controller!__

### Examples

Send the standard command (0x07 = "center") over the standard serial port
```/dev/ttyS0``` to the standard AMC controller (with address 0x65) and select
the standard stepper motor driver card (0) .
```
./amc_test.py
```

Send the command 0x07 = "center" over the serial port ```/dev/ttyS1``` with
host PC ID 0xf7 to the AMC controller with address 0x65 and select stepper
motor driver 0. Produce verbose output.
```
./amc_test.py -d /dev/ttyS1 -a 0x65 -b 0 -c 0x07 -p 0xf7 -v 3
```

