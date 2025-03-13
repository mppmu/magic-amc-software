# Software for the MAGIC Active Mirror Control (AMC).

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

