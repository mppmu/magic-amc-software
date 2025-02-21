#!/usr/bin/env python3
#
# File: amc_test_raw.py
# Auth: M. Fras, Electronics Division, MPI for Physics, Munich
# Mod.: M. Fras, Electronics Division, MPI for Physics, Munich
# Date: 17 Feb 2025
# Rev.: 21 Feb 2025
#
# Python script to send raw hex bytes to the AMC test setup to move the motor.
#
# CAUTION: To be used with the AMC controller firmware V6 (AMContr 2.1.6) only!
#



import argparse
import sys
import serial
import serial.rs485



#####################################################################
# Parse command line arguments.
#####################################################################

parser = argparse.ArgumentParser(description='Send a raw hexadecimal string to the MAGIC AMC.')
parser.add_argument('-c', '--command', action='store', type=str,
                    dest='amc_cmd_hex', default='FB 01 65 00 F0 01 07 0F 9A', metavar='AMC_COMMAND',
                    help='Hex string to send to the AMC.')
parser.add_argument('-d', '--device', action='store', type=str,
                    dest='serial_device', default='/dev/ttyS0', metavar='SERIAL_DEVICE',
                    help='Serial device to access the AMC.')
parser.add_argument('-e', '--enable-rs485', action='store_true',
                    dest='enable_rs_485', default=True,
                    help='Enable the RS-485 settings.')
parser.add_argument('-n', '--no-enable-rs485', action='store_false',
                    dest='enable_rs_485',
                    help='Disable the RS-485 settings.')
args = parser.parse_args()

serial_device = args.serial_device
enable_rs_485 = args.enable_rs_485
amc_cmd_hex = args.amc_cmd_hex



#####################################################################
# Auxiliary functions.
#####################################################################

def bytearray2hexstr(b):
    if len(b) <= 0:
        return ''
    return ''.join('{:02X} '.format(x) for x in b).rstrip()



#####################################################################
# Communication with AMC controller board.
#####################################################################

# Open the serial port.
try:
    ser = serial.Serial(
        port=serial_device,
        baudrate=115200,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        xonxoff=False,
        rtscts=False,
        #rtscts=True,
        dsrdtr=False,
        #dsrdtr=True,
        timeout=1
    )
except Exception as error:
    print("Error setting up the serial port {0:s}:".format(serial_device))
    print(error)
    sys.exit(1)

# Set the RS-485 parameters.
try:
    if enable_rs_485:
        ser.rs485_mode = serial.rs485.RS485Settings(
            rts_level_for_tx=True,
            rts_level_for_rx=False,
            loopback=False,
            delay_before_tx=None,
            delay_before_rx=None
        )
except Exception as error:
    print("Error setting the RS-485 parameters for serial port {0:s}:".format(serial_device))
    print(error)
    sys.exit(1)

# Send hex string to the AMC controller and receive the answer.
print("RS-485 port: {0:s}".format(ser.name))
amc_cmd_bytes = bytearray.fromhex(amc_cmd_hex)
print("Hex string sent to AMC: '{0:s}'".format(bytearray2hexstr(amc_cmd_bytes)))
ser.write(amc_cmd_bytes)
ret = ser.read(ser.inWaiting())
print("Response from AMC: '{0:s}'".format(bytearray2hexstr(ret)))

