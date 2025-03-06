#!/usr/bin/env python3
#
# File: amc_test.py
# Auth: M. Fras, Electronics Division, MPI for Physics, Munich
# Mod.: M. Fras, Electronics Division, MPI for Physics, Munich
# Date: 18 Feb 2025
# Rev.: 06 Mar 2025
#
# Python script to send a hex command to the AMC test setup to move the motor.
#
# CAUTION: To be used with the AMC controller firmware V6 (AMContr 2.1.6) only!
#
# AMC frame format from computer to AMC controller:
# 1. SD byte 0xFB (details unknown).
# 2. SOH byte 0x01 (details unknown).
# 3. AMC ID byte.
# 4. Stepper motor driver ID byte (0..3).
# 5. Host computer ID byte.
# 6. Data length (1 byte).
# 7. Command + data bytes (matching the number given as data length in 6.).
# 8. XModem 16-bit CRC (2 bytes, LSB first, MSB last).
#
# AMC frame format from AMC controller to computer:
# 1.  SD byte 0xFB (details unknown).
# 2.  ACK (0x85) or NAK (0x15).
# 3.  Host computer ID byte.
# 4.  AMC ID byte.
# 5.  Stepper motor driver ID byte (0..3).
# 6.  Number of payload bytes (1 byte).
# 7.  Repeat of command sent from host computer (1 byte).
# 8.  Two status bytes.
# 9.  Data bytes.
# 10. XModem 16-bit CRC (2 bytes, LSB first, MSB last).
#



import argparse
import sys
import time
import serial
import serial.rs485



PREFIX_ERROR = "ERROR: "



#####################################################################
# Parse command line arguments.
#####################################################################

parser = argparse.ArgumentParser(description='Send a hexadecimal command string to the MAGIC AMC.')
parser.add_argument('-a', '--amc-id', action='store', type=lambda x: int(x,0),
                    dest='amc_id', default=0x65, metavar='AMC_ID',
                    help='AMC ID = address of controller board.')
parser.add_argument('-b', '--sm-driver-id', action='store', type=lambda x: int(x,0),
                    dest='sm_driver_id', default=0x00, metavar='SM_DRIVER_ID',
                    help='Stepper motor driver ID (0..3).')
parser.add_argument('-c', '--command', action='store', type=str,
                    dest='amc_cmd', default='0x07', metavar='AMC_COMMAND',
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
parser.add_argument('-p', '--pc-id', action='store', type=lambda x: int(x,0),
                    dest='pc_id', default=0xF0, metavar='PC_ID',
                    help='Host computer ID.')
parser.add_argument('-v', '--verbosity', action='store', type=int,
                    dest='verbosity', default="1", choices=range(0, 4),
                    help='Set the verbosity level. The default is 1.')
args = parser.parse_args()

# Serial device parameters.
serial_device = args.serial_device
enable_rs_485 = args.enable_rs_485

# AMC parameters.
# See section "Protocol constants" in "amc.h" of AMContr firmware V6 (2.1.6).
AMC_FRAME_SD        = 0XFB
AMC_FRAME_SOH       = 0X01
AMC_FRAME_ACK       = 0X85
AMC_FRAME_NAK       = 0X15
AMC_ANS_HEADER_LEN  = 6         # Fixed header of 6 bytes.
AMC_ANS_STATUS_LEN  = 2         # 2 status bytes.
AMC_ANS_CRC_LEN     = 2         # 2 bytes of 16 bit CRC.
AMC_ANS_ACK_MIN_LEN = 11        # An ACK frame contains at least 11 bytes.
AMC_ANS_NAK_LEN     = 8         # A NAK frame contains 8 bytes.

# Verbosity.
verbosity = args.verbosity



#####################################################################
# Auxiliary functions.
#####################################################################

# Convert string or int to bytearray.
def x2bytearray(x):
    if isinstance(x, int):
        ba = x.to_bytes(4, 'little')
    elif isinstance(x, str):
        ba = bytearray(map(lambda x: int(x,0), x.split()))
    else:
        ba = bytearray()
    return ba

# Format a bytearray into a hex string.
def bytearray2hexstr(ba):
    if len(ba) <= 0:
        return ''
    return ''.join('{:02X} '.format(x) for x in ba).rstrip()

# Format a bytearray into a hex string with leading '0x'.
def bytearray2hexstr0x(ba):
    if len(ba) <= 0:
        return ''
    return ''.join('0x{:02X} '.format(x) for x in ba).rstrip()

# Algorithm found here:
# https://mdfs.net/Info/Comp/Comms/CRC16.htm
# Thanks for the excellent work!
def crc16_xmodem(data):
    crc  = 0x0000               # CRC start value = 0x0000.
    poly = 0x1021               # CRC polynomic = 0x1021 for XModem.
    for byte in data:           # Loop over data byte by byte.
        crc ^= (byte << 8)      # XOR data byte into CRC top byte.
        for i in range(8):      # Rotate left over 8 bits.
            crc = crc << 1      # Rotate left by one bit.
            if crc & 0x10000:   # Bit 15 was set (now bit 16) ...
                crc = (crc ^ poly) & 0xffff # ... XOR with XModem polynomic and ensure
                                            #     the CRC remains a 16-bit value.
    return crc



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
    print(PREFIX_ERROR, end='')
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
    print(PREFIX_ERROR, end='')
    print("Error setting the RS-485 parameters for serial port {0:s}:".format(serial_device))
    print(error)
    sys.exit(1)



# Assemble the full AMC frame.
amc_frame = bytearray()
amc_frame.append(x2bytearray(AMC_FRAME_SD)[0])
amc_frame.append(x2bytearray(AMC_FRAME_SOH)[0])
amc_frame.append(x2bytearray(args.amc_id)[0])
amc_frame.append(x2bytearray(args.sm_driver_id)[0])
amc_frame.append(x2bytearray(args.pc_id)[0])
amc_frame.append(len(x2bytearray(args.amc_cmd)) & 0xff)
amc_frame.extend(x2bytearray(args.amc_cmd))
amc_crc = crc16_xmodem(amc_frame[2:])
amc_frame.extend(x2bytearray(amc_crc)[:2])

# Messages for debugging.
if verbosity >= 1:
    print("Serial device: {0:s}".format(ser.name))
if verbosity >= 2:
    print()
    print("AMC ID         : 0x{0:02X}".format(amc_frame[2]))
    print("SM driver ID   : 0x{0:02X}".format(amc_frame[3]))
    print("Host PC ID     : 0x{0:02X}".format(amc_frame[4]))
    print("Data length    : 0x{0:02X}".format(amc_frame[5]))
    print("AMC command    : {0:s}".format(bytearray2hexstr0x(amc_frame[6:-2])))
    print("XModem CRC-16  : 0x{0:02X}".format(amc_crc))
    print()
if verbosity >= 3:
    print("Frame sent to AMC (hex): '{0:s}'".format(bytearray2hexstr(amc_frame)))
    print()

# Send frame to AMC controller.
try:
    ser.write(amc_frame)
except Exception as error:
    print(PREFIX_ERROR, end='')
    print("Error sending data to the serial port {0:s}:".format(serial_device))
    print(error)
    sys.exit(2)

# Wait a while for the answer from the AMC controller.
time.sleep(0.1)

# Read answer from AMC controller.
try:
    amc_answer = ser.read(ser.inWaiting())
except Exception as error:
    print(PREFIX_ERROR, end='')
    print("Error reading data from the serial port {0:s}:".format(serial_device))
    print(error)
    sys.exit(2)
if verbosity >= 1:
    print("Response from AMC (hex): '{0:s}'".format(bytearray2hexstr(amc_answer)))
if verbosity >= 2:
    print()

# Evaluate the answer from the AMC controller.
if len(amc_answer) < AMC_ANS_NAK_LEN:
    print(PREFIX_ERROR, end='')
    print("Answer from AMC controller too short! Expected at least {0:d} bytes, got {1:d}!".format(AMC_ANS_NAK_LEN, len(amc_answer)))
for idx, b in enumerate(amc_answer):
    if idx == 0:        # AMC frame start.
        if b != AMC_FRAME_SD:
            print(PREFIX_ERROR, end='')
            print("Answer from AMC controller does not start with 0x{0:02X}!".format(AMC_FRAME_SD))
        else:
            if verbosity >= 2:
                print("Correct frame start (0x{0:02X}) received from the AMC controller.".format(AMC_FRAME_SD))
    elif idx == 1:      # ACK or NAK.
        if b == AMC_FRAME_NAK:
            print(PREFIX_ERROR, end='')
            print("NAK received from AMC command!")
        elif b != AMC_FRAME_ACK:
            print(PREFIX_ERROR, end='')
            print("Invalid response from AMC: The second byte is neither ACK (0x{0:02X}) nor NAK (0x{1:02X})!".format(AMC_FRAME_ACK, AMC_FRAME_NAK))
        else:
            if verbosity >= 2:
                print("Correct ACK (0x{0:02X}) received from the AMC controller.".format(AMC_FRAME_ACK))
    elif idx == 2:      # Host computer ID.
        if b != x2bytearray(args.pc_id)[0]:
            print(PREFIX_ERROR, end='')
            print("Wrong host computer ID received! Expected 0x{0:02X}, got 0x{1:02X}.".format(x2bytearray(args.pc_id)[0], b))
        else:
            if verbosity >= 3:
                print("Correct computer host ID (0x{0:02X}) received from the AMC controller.".format(x2bytearray(args.pc_id)[0]))
    elif idx == 3:      # AMC ID.
        if b != x2bytearray(args.amc_id)[0]:
            print(PREFIX_ERROR, end='')
            print("Wrong AMC ID received! Expected 0x{0:02X}, got 0x{1:02X}.".format(x2bytearray(args.amc_id)[0], b))
        else:
            if verbosity >= 3:
                print("Correct AMC ID (0x{0:02X}) received from the AMC controller.".format(x2bytearray(args.amc_id)[0]))
    elif idx == 4:      # Stepper motor driver ID.
        if b != x2bytearray(args.sm_driver_id)[0]:
            print(PREFIX_ERROR, end='')
            print("Wrong stepper motor driver ID received! Expected 0x{0:02X}, got 0x{1:02X}.".format(x2bytearray(args.sm_driver_id)[0], b))
        else:
            if verbosity >= 3:
                print("Correct stepper motor driver ID (0x{0:02X}) received from the AMC controller.".format(x2bytearray(args.sm_driver_id)[0]))
    elif idx == 5:      # Number of payload bytes (without CRC).
        payload_len = b
        if verbosity >= 3:
            print("Number of payload bytes (without CRC): {0:d}".format(b))
    elif idx == 6:      # Repeat command.
        if b != x2bytearray(args.amc_cmd)[0]:
            print(PREFIX_ERROR, end='')
            print("Wrong repeated command received! Expected 0x{0:02X}, got 0x{1:02X}.".format(x2bytearray(args.amc_cmd)[0], b))
        else:
            if verbosity >= 3:
                print("Correct repeated command (0x{0:02X}) received from the AMC controller.".format(x2bytearray(args.amc_cmd)[0]))
    elif idx == 7:      # Status byte 1. See page 12 of "amc2cmd.pdf" and
                        # "amc.h" of AMC controller firmware version 2.1.6.
                        # The information in "amc.h" takes precedence.
        if verbosity >= 2:
            print("Status byte 1: 0x{0:02X}".format(b))
            if b & 0x01: print("    Verges error, execution of command done/tried.")
            if b & 0x02: print("    Unknown command (inexistent cc).")
            if b & 0x04: print("    Command rejected, nothing done (e.g. on too early move).")
            if b & 0x08: print("    Illegal/meaningless first parameter (word).")
            if b & 0x10: print("    Illegal/meaningless second/other parameter (word).")
            if b & 0x20: print("    CC and length of frame (number of params) contradicting.")
            if b & 0x40: print("    CP and length of frame contradicting.")
            if b & 0x80: print("    Bad driver address.")
    elif idx == 8:      # Status byte 2. See page 12 of "amc2cmd.pdf" and
                        # "amc.h" of AMC controller firmware version 2.1.6.
                        # The information in "amc.h" takes precedence.
        if verbosity >= 2:
            print("Status byte 2: 0x{0:02X}".format(b))
            if b & 0x01: print("    Motor X moving.")
            if b & 0x02: print("    Motor Y moving.")
            if b & 0x04: print("    Motor X direction up.")
            if b & 0x08: print("    Motor Y direction up.")
            if b & 0x10: print("    Motor X at end switch.")
            if b & 0x20: print("    Motor Y at end switch.")
            if b & 0x40: print("    Laser is on.")
            if b & 0x80: print("    Bad driver card.")
    elif idx >= 9 and idx < AMC_ANS_HEADER_LEN + payload_len:
        if verbosity >= 2:
            print("Data byte: 0x{0:02X}".format(b))

# Check for CRC error.
if len(amc_answer) == AMC_ANS_HEADER_LEN + payload_len + AMC_ANS_CRC_LEN:
    # CRC received with the answer.
    amc_answer_crc = int.from_bytes(amc_answer[-2:-1]) + (int.from_bytes(amc_answer[-1:]) << 8)
    # Calculate the CRC of the answer.
    amc_answer_crc_calc = crc16_xmodem(amc_answer[2:-2])
    if amc_answer_crc != amc_answer_crc_calc:
        print(PREFIX_ERROR, end='')
        print("Wrong CRC received! Expected 0x{0:04X}, got 0x{1:04X}.".format(amc_answer_crc_calc, amc_answer_crc))
    else:
        if verbosity >= 3:
            print("Correct CRC 0x{0:04X} received from the AMC controller.".format(amc_answer_crc))

