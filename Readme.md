Software for the MAGIC Active Mirror Control (AMC)
==================================================

## Dependencies

In order to compile and run the AMC software and the SBIG software, some
dependencies are required.

Please see the [dependencies document](Dependencies.md) for details.



## AMC Software

* To build the AMC software, run these commands:
  ```
  cd amc
  make mrproper && make all
  ```
* Before running the AMC software:
  - Copy the LUT files into the `amc` directory.
  - Create a link inside the `amc` directory with the name
    `test_Panels1.txt` to a valid AMC panel list file, e.g.
    `AMC1_panel_list.txt`.
  - Create the links `/home/operator/log1` and `/home/operator/sbig1`
    pointing to valid directories for storing the log files and SBIG images.
* To run the AMC software, cd into the `amc` directory and run:
  ```
  ./amc
  ```
* The software tends to crash after displaying the graphical user interface on
  computers with multiple cores. The reason is probably that it starts some
  threads that are not thread-safe and can cause race conditions if they are
  executed in parallel on different CPU cores. To overcome this problem, or at
  least minimize its effects, the AMC software can be restricted to a single
  core using the `taskset` command.
  ```
  taskset -c 0 ./amc
  ```
* The shell script `amc/amc.sh` automatically launches an Apptainer container,
  changes to the directory `/home/operator/V4.50`, and starts the AMC software
  within that container restricted to one CPU core. By copying this script to a
  location within the `PATH` environment variable, the AMC software can simply be
  lauched with this command:
  ```
  ./amc.sh
  ```
  Please note that this script should be run from the `operator` account.



## SBIG Software

* To build the SBIG software, run these commands:
  ```
  cd sbig
  make mrproper && make all
  ```
* To run the AMC software, cd into the `sbig` directory and run:
  ```
  ./sbigab
  ```



## AMC-Test Software

The simple Python program `amc_test.py` can be used to send individual
commands to the AMC system. It sends hexadecimal commands to the AMC controller
board (AMContr_R3) and it receives and evaluates the response.  

__Caution: The firmware version 6 (2.1.6) is required on the AMC controller!__


### Examples

Send the standard command ("center" = 0x07) over the standard serial port
`/dev/ttyS0` to the standard AMC controller (with address 0x65) and select
the standard stepper motor driver card (0).
```
./amc_test.py
```

Send the standard command ("center" = 0x07) over the serial port
`/dev/ttyS1` to the AMC controller with address 40 and select the stepper
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

Send the raw command 0x07 = "center" over the serial port `/dev/ttyS1` with
host computer ID 0xf7 to the AMC controller with address 0x65 (101) and select
stepper motor driver 0. Produce verbose output.
```
./amc_test.py -d /dev/ttyS1 -a 0x65 -b 0 -i 0xf7 -v 3 -c raw 0x7
```

Scan the serial port `/dev/ttyS1` for active AMC boxes.
```
./amc_test.py -d /dev/ttyS1 -c scan
```

Scan the serial port `/dev/ttyS1` for active AMC boxes with addresses
between 30 (0x1e) and 90 (0x5a).  
Decimal addresses:
```
./amc_test.py -d /dev/ttyS1 -c scan 30 90
```
Hexadecimal addresses:
```
./amc_test.py -d /dev/ttyS1 -c scan 0x1e 0x5a
```


### Scan the System for Active AMC Boxes

A simple shell script is available to scan all 8 chains for active AMC boxes:
```
./amc_scan.sh
```
It calls the `amc_test.py` script to scan one chain at a time.

