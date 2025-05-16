#!/usr/bin/env python3
#
# File: amc_test.py
# Auth: M. Fras, Electronics Division, MPI for Physics, Munich
# Mod.: M. Fras, Electronics Division, MPI for Physics, Munich
# Date: 18 Feb 2025
# Rev.: 16 May 2025
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
parser.add_argument('-c', '--command', action='store', type=lambda s: str(s).lower(),
                    choices=['center', 'move_rel_x', 'move_rel_y',
                             'move_rel_xy', 'move_abs', 'stop',
                             'reset_box', 'reset_driver', 'status', 'raw',
                             'scan'
                            ],
                    dest='amc_cmd', default='center',
                    help='Command to be execute on the AMC.')
parser.add_argument('-d', '--device', action='store', type=str,
                    dest='serial_device', default='/dev/ttyS0', metavar='SERIAL_DEVICE',
                    help='Serial device to access the AMC.')
parser.add_argument('-e', '--enable-rs485', action='store_true',
                    dest='enable_rs_485', default=True,
                    help='Enable the RS-485 settings.')
parser.add_argument('-i', '--pc-id', action='store', type=lambda x: int(x,0),
                    dest='pc_id', default=0xF0, metavar='PC_ID',
                    help='Host computer ID.')
parser.add_argument('-n', '--no-enable-rs485', action='store_false',
                    dest='enable_rs_485',
                    help='Disable the RS-485 settings.')
parser.add_argument('-v', '--verbosity', action='store', type=int,
                    dest='verbosity', default="1", choices=range(0, 4),
                    help='Set the verbosity level. The default is 1.')
parser.add_argument('remainder', nargs=argparse.REMAINDER)
args = parser.parse_args()


# AMC parameters.
# See section "Protocol constants" in "amc.h" of AMContr firmware V6 (2.1.6).
AMC_FRAME_SD        = 0XFB
AMC_FRAME_SOH       = 0X01
AMC_FRAME_ACK       = 0X85
AMC_FRAME_NAK       = 0X15
# AMC command bytes.
# See main function "amcontr.c" of AMContr firmware V6 (2.1.6).
AMC_CMD_MOVE_REL_X  = bytearray([0x01])
AMC_CMD_MOVE_REL_Y  = bytearray([0x02])
AMC_CMD_MOVE_REL_XY = bytearray([0x03])
AMC_CMD_MOVE_ABS    = bytearray([0x05])
AMC_CMD_CENTER      = bytearray([0x07])
AMC_CMD_RESET_BOX   = bytearray([0x09])
AMC_CMD_RESET_DRIVER= bytearray([0x0A])
AMC_CMD_STOP        = bytearray([0x0C])
AMC_CMD_STATUS      = bytearray([0x1F])
# Parameters of AMC answers.
AMC_ANS_HEADER_LEN  = 6         # Fixed header of 6 bytes.
AMC_ANS_STATUS_LEN  = 2         # 2 status bytes.
AMC_ANS_CRC_LEN     = 2         # 2 bytes of 16 bit CRC.
AMC_ANS_ACK_MIN_LEN = 11        # An ACK frame contains at least 11 bytes.
AMC_ANS_NAK_LEN     = 8         # A NAK frame contains 8 bytes.
AMC_ANS_XSTATUS_LEN = 12        # Payload length of the AMC extended status.
# Parameters for ADC conversions.
#AMC_BOARD_VREF = 4.096          # The reference voltage source U5 (ADR292GR) provides 4.096 V.
AMC_BOARD_VREF = 3.0            # Fix for boards where the reference voltage source U5 (ADR292GR) is missing.
AMC_ADC_LSB_MV_AREF = AMC_BOARD_VREF / 1024 # The LSB is VREF / 1024 for 10 bit FSR.
AMC_ADC_LSB_MV_INT  = 2.5       # With the internal reference voltage of 2.56 V, the ADC LSB is 2.5 mV.
AMC_ADC_LSB_MV_AVCC = 4.88      # For the temperature sensor, AVCC = 5.0 V is used as reference voltage.
                                # I.e. the ADC LSB is 5 V / 1024 = 4.88 mV.
AMC_COEFF_TEMP      = 0.1       # The LM135 sensor value is 10 mV / °K. I.e. 0.1 °K ^= 1 mV.
AMC_COEFF_HUMIDITY  = 100 / 60e3# Resistive humidity sensor: 0 .. 100 % RH ^= 30 .. 90 kOhm.
AMC_OFFSET_HUMIDITY = 30e3 * AMC_COEFF_HUMIDITY
AMC_COEFF_CURRENT   = -1.25     # The current is measured across a 100 mOhm shunt
                                # resistor with an LT1787HVIS8 current sense amplifier
                                # with an amplification of 8 in reverse polarity.
                                # I.e. 10 mA ^= -8 mV.
                                # The negative current is measured with an offset of VREF.
AMC_OFFSET_CURRENT  = AMC_BOARD_VREF * AMC_COEFF_CURRENT # The pin VBIAS of the LT1787HVIS8 is connected
                                                         # to VREF on the board.
AMC_COEFF_VOLT_1    = 103 / 3 * 1e-3    # Measured with a voltage divider of 100k over 3k.
AMC_COEFF_VOLT_2    = 103 / 3 * 1e-3    # Measured with a voltage divider of 100k over 3k.
AMC_COEFF_VOLT_LOGIC= 6.9 / 2.2 * 1e-3  # Measured with a voltage divider of 4.7k over 2.2k.

# Custom parameters.
STATUS_INCLUDES_HUM_CUR = False # Should the status information include the humidity and the current values,
                                # which may not work on most or all board?



#####################################################################
# Auxiliary functions.
#####################################################################

# Convert string or int to bytearray.
def x2bytearray(x):
    if isinstance(x, int):
        ba = x.to_bytes(4, 'little')
    elif isinstance(x, str):
        ba = bytearray(map(lambda x: int(x,0), x.split()))
    elif isinstance(x, list):
        ba = bytearray(list(map(lambda x: int(x,0), x)))
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

# Format a string into a short integer (2 bytes, signed).
def str2short(s):
    i = int(s, 0)
    if i < -0x7fff:
        return 0x8000
    if i < 0:
        return 0xffff - (abs(i) & 0x7fff) + 1
    if i <= 0x7fff:
        return i & 0x7fff
    return 0x7fff

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
# AMC frame functions.
#####################################################################

# Assemble the full AMC frame.
def amc_frame_assemble(amc_id, sm_driver_id, pc_id, amc_cmd):
    amc_frame = bytearray()
    amc_frame.append(x2bytearray(AMC_FRAME_SD)[0])
    amc_frame.append(x2bytearray(AMC_FRAME_SOH)[0])
    amc_frame.append(amc_id)
    amc_frame.append(sm_driver_id)
    amc_frame.append(pc_id)
    amc_frame.append(len(amc_cmd) & 0xff)
    amc_frame.extend(amc_cmd)
    amc_crc = crc16_xmodem(amc_frame[2:])
    amc_frame.extend(x2bytearray(amc_crc)[:2])
    return amc_frame, amc_crc

# Define the binary AMC command and its parameters from the requested command.
def parse_amc_cmd(amc_cmd, cmd_params):
    if amc_cmd == 'center':
        amc_cmd_bin = AMC_CMD_CENTER
    # Commands with one short signed integer as argument.
    elif amc_cmd in ['move_rel_x', 'move_rel_y']:
        if not cmd_params:
            print(PREFIX_ERROR, end='')
            print("The number of steps must be specified for the '{0:s}' command!".format(amc_cmd))
            sys.exit(2)
        if amc_cmd == 'move_rel_x':
            amc_cmd_bin = AMC_CMD_MOVE_REL_X
        else:
            amc_cmd_bin = AMC_CMD_MOVE_REL_Y
        amc_cmd_bin.extend(x2bytearray(str2short(cmd_params[0]))[0:2])
    # Commands with two short signed integers as argument.
    elif amc_cmd in ['move_rel_xy', 'move_abs']:
        if not cmd_params:
            print(PREFIX_ERROR, end='')
            print("The number of steps must be specified for the '{0:s}' command!".format(amc_cmd))
            sys.exit(2)
        if len(cmd_params) < 2:
            print(PREFIX_ERROR, end='')
            print("Both the number of steps in X and Y must be specified for the '{0:s}' command!".format(amc_cmd))
            sys.exit(2)
        if amc_cmd == 'move_rel_xy':
            amc_cmd_bin = AMC_CMD_MOVE_REL_XY
        else:
            amc_cmd_bin = AMC_CMD_MOVE_ABS
        print(cmd_params)
        amc_cmd_bin.extend(x2bytearray(str2short(cmd_params[0]))[0:2])
        amc_cmd_bin.extend(x2bytearray(str2short(cmd_params[1]))[0:2])
    elif amc_cmd == 'stop':
        amc_cmd_bin = AMC_CMD_STOP
    elif amc_cmd == 'reset_box':
        amc_cmd_bin = AMC_CMD_RESET_BOX
    elif amc_cmd == 'reset_driver':
        amc_cmd_bin = AMC_CMD_RESET_DRIVER
    elif amc_cmd == 'status':
        amc_cmd_bin = AMC_CMD_STATUS
    elif amc_cmd == 'raw':
        if not cmd_params:
            print(PREFIX_ERROR, end='')
            print("The raw AMC command string must be specified for the command '{0:s}'!".format(amc_cmd))
            sys.exit(2)
        amc_cmd_bin = x2bytearray(cmd_params)
    else:
        print(PREFIX_ERROR, end='')
        print("Unknown command '{0:s}'!".format(amc_cmd))
        sys.exit(2)
    return amc_cmd_bin

# Print messages about the AMC frame.
def amc_frame_print(serial_device, amc_frame, amc_crc, verbosity):
    if verbosity >= 1:
        print("Serial device: {0:s}".format(serial_device.name))
    if verbosity >= 2:
        print()
        print("AMC ID        : 0x{0:02X}".format(amc_frame[2]))
        print("SM driver ID  : 0x{0:02X}".format(amc_frame[3]))
        print("Host PC ID    : 0x{0:02X}".format(amc_frame[4]))
        print("Data length   : 0x{0:02X}".format(amc_frame[5]))
        print("AMC command   : {0:s}".format(bytearray2hexstr0x(amc_frame[6:-2])))
        print("XModem CRC-16 : 0x{0:04X}".format(amc_crc))
        print()
    if verbosity >= 3:
        print("Frame sent to AMC (hex): '{0:s}'".format(bytearray2hexstr(amc_frame)))
        print()

# Send frame to AMC controller.
def amc_frame_send(ser, amc_frame):
    try:
        ser.write(amc_frame)
    except Exception as error:
        print(PREFIX_ERROR, end='')
        print("Error sending data to the serial port {0:s}:".format(ser.name))
        print(error)
        sys.exit(2)

# Read answer from AMC controller.
def amc_answer_read(ser, verbosity):
    try:
        amc_answer = ser.read(ser.inWaiting())
    except Exception as error:
        print(PREFIX_ERROR, end='')
        print("Error reading data from the serial port {0:s}:".format(ser.name))
        print(error)
        sys.exit(2)
    if verbosity >= 1:
        print("Response from AMC (hex): '{0:s}'".format(bytearray2hexstr(amc_answer)))
    if verbosity >= 2:
        print()
    return amc_answer

# Evaluate the answer from the AMC controller.
def amc_answer_eval(amc_answer, amc_id, sm_driver_id, pc_id, amc_cmd_bin, amc_cmd, verbosity):
    # Parse the answer from the AMC controller.
    if len(amc_answer) < AMC_ANS_NAK_LEN:
        print(PREFIX_ERROR, end='')
        print("Answer from AMC controller too short! Expected at least {0:d} bytes, got {1:d}!".format(AMC_ANS_NAK_LEN, len(amc_answer)))
    payload_len = 0     # Need to define a default value for payload_len, as it is
                        # used for checking the CRC.
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
            if b != pc_id:
                print(PREFIX_ERROR, end='')
                print("Wrong host computer ID received! Expected 0x{0:02X}, got 0x{1:02X}.".format(pc_id, b))
            else:
                if verbosity >= 3:
                    print("Correct computer host ID (0x{0:02X}) received from the AMC controller.".format(pc_id))
        elif idx == 3:      # AMC ID.
            if b != amc_id:
                print(PREFIX_ERROR, end='')
                print("Wrong AMC ID received! Expected 0x{0:02X}, got 0x{1:02X}.".format(amc_id, b))
            else:
                if verbosity >= 3:
                    print("Correct AMC ID (0x{0:02X}) received from the AMC controller.".format(amc_id))
        elif idx == 4:      # Stepper motor driver ID.
            if b != sm_driver_id:
                print(PREFIX_ERROR, end='')
                print("Wrong stepper motor driver ID received! Expected 0x{0:02X}, got 0x{1:02X}.".format(sm_driver_id, b))
            else:
                if verbosity >= 3:
                    print("Correct stepper motor driver ID (0x{0:02X}) received from the AMC controller.".format(sm_driver_id))
        elif idx == 5:      # Number of payload bytes (without CRC).
            payload_len = b
            if verbosity >= 3:
                print("Number of payload bytes (without CRC): {0:d}".format(b))
        elif idx == 6:      # Repeated command.
            if b != amc_cmd_bin[0]:
                print(PREFIX_ERROR, end='')
                print("Wrong repeated command received! Expected 0x{0:02X}, got 0x{1:02X}.".format(amc_cmd_bin[0], b))
            else:
                if verbosity >= 3:
                    print("Correct repeated command (0x{0:02X}) received from the AMC controller.".format(amc_cmd_bin[0]))
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

    # Show status information.
    if amc_cmd == 'status':
        if len(amc_answer) != AMC_ANS_HEADER_LEN + AMC_ANS_STATUS_LEN + 1 + AMC_ANS_XSTATUS_LEN + AMC_ANS_CRC_LEN:
            print(PREFIX_ERROR, end='')
            print("Wrong length of AMC answer for status query! Expected {0:d}, got {1:d}.".format(AMC_ANS_HEADER_LEN + AMC_ANS_STATUS_LEN + 1 + AMC_ANS_XSTATUS_LEN + AMC_ANS_CRC_LEN, len(amc_answer)))
        else:
            raw_temperature= amc_answer[AMC_ANS_HEADER_LEN+ 3] + (amc_answer[AMC_ANS_HEADER_LEN+ 4] << 8)
            raw_humidity   = amc_answer[AMC_ANS_HEADER_LEN+ 5] + (amc_answer[AMC_ANS_HEADER_LEN+ 6] << 8)
            raw_current    = amc_answer[AMC_ANS_HEADER_LEN+ 7] + (amc_answer[AMC_ANS_HEADER_LEN+ 8] << 8)
            raw_volt_1     = amc_answer[AMC_ANS_HEADER_LEN+ 9] + (amc_answer[AMC_ANS_HEADER_LEN+10] << 8)
            raw_volt_2     = amc_answer[AMC_ANS_HEADER_LEN+11] + (amc_answer[AMC_ANS_HEADER_LEN+12] << 8)
            raw_volt_logic = amc_answer[AMC_ANS_HEADER_LEN+13] + (amc_answer[AMC_ANS_HEADER_LEN+14] << 8)
            # The temperature value is in Kelvin, i.e. subtract 273.15 to get Celsius.
            temperature    = raw_temperature * AMC_ADC_LSB_MV_AVCC * AMC_COEFF_TEMP - 273.15
            # The humidity is measured with a voltage divider of 68k over sensor at VREF.
            # 1. Calculate the resistance of the humidity sensor from the ADC counts.
            volt_humidity  = raw_humidity * AMC_ADC_LSB_MV_INT * 1e-3   # Voltage at humidity sensor in V instead of mV!
            res_hum_sens   = volt_humidity * 68e3 / (AMC_BOARD_VREF - volt_humidity)    # Voltage divider with 68k resistor at VREF.
            # 2. Calculate the humidity from the resistance.
            humidity       = res_hum_sens * AMC_COEFF_HUMIDITY - AMC_OFFSET_HUMIDITY
            # See comments at the definition of AMC_COEFF_CURRENT and AMC_OFFSET_CURRENT.
            current        = (raw_current * AMC_ADC_LSB_MV_INT * 1e-3) * AMC_COEFF_CURRENT - AMC_OFFSET_CURRENT
            # Simple voltage dividers are use for measuring the supply voltages.
            volt_1         = raw_volt_1 * AMC_ADC_LSB_MV_INT * AMC_COEFF_VOLT_1
            volt_2         = raw_volt_2 * AMC_ADC_LSB_MV_INT * AMC_COEFF_VOLT_2
            volt_logic     = raw_volt_logic * AMC_ADC_LSB_MV_INT * AMC_COEFF_VOLT_LOGIC
            print("AMC status:")
            print("    Temperature      : {0:02.02f} °C     (raw: 0x{1:04X} ADC counts)".format(temperature, raw_temperature))
            if STATUS_INCLUDES_HUM_CUR:
                print("    Humidity (*)     : {0:02.02f} % RH   (raw: 0x{1:04X} ADC counts)".format(humidity, raw_humidity))
                print("    Current (*)      : {0:01.03f} A      (raw: 0x{1:04X} ADC counts)".format(current, raw_current))
            print("    Supply voltage 1 : {0:02.03f} V     (raw: 0x{1:04X} ADC counts)".format(volt_1, raw_volt_1))
            print("    Supply voltage 2 : {0:02.03f} V     (raw: 0x{1:04X} ADC counts)".format(volt_2, raw_volt_2))
            print("    Logic voltage    : {0:01.03f} V      (raw: 0x{1:04X} ADC counts)".format(volt_logic, raw_volt_logic))
            if STATUS_INCLUDES_HUM_CUR:
                print()
                print("(*)")
                print("CAUTION: The values of the humidity and the current may be wrong (meaningless),")
                print("         because the required components are not mounted on the board!")

    # Check for CRC error.
    if len(amc_answer) == AMC_ANS_HEADER_LEN + payload_len + AMC_ANS_CRC_LEN:
        # CRC received with the answer.
        amc_answer_crc = int.from_bytes(amc_answer[-2:], "little")
        # Calculate the CRC of the answer.
        amc_answer_crc_calc = crc16_xmodem(amc_answer[2:-2])
        if amc_answer_crc != amc_answer_crc_calc:
            print(PREFIX_ERROR, end='')
            print("Wrong CRC received! Expected 0x{0:04X}, got 0x{1:04X}.".format(amc_answer_crc_calc, amc_answer_crc))
        else:
            if verbosity >= 3:
                print("Correct CRC 0x{0:04X} received from the AMC controller.".format(amc_answer_crc))



#####################################################################
# High-level AMC functions.
#####################################################################

# Communication with AMC controller board.
def amc_comm(ser, verbosity):
    # Define the AMC parameters.
    amc_id       = x2bytearray(args.amc_id)[0]
    sm_driver_id = x2bytearray(args.sm_driver_id)[0]
    pc_id        = x2bytearray(args.pc_id)[0]
    amc_cmd      = args.amc_cmd
    cmd_params   = args.remainder

    # Special command 'scan': Scan the RS-485 port for active controllers.
    if amc_cmd == 'scan':
        amc_scan_min = 0
        amc_scan_max = 255
        if len(cmd_params) >= 1:
            amc_scan_min = int(cmd_params[0])
        if len(cmd_params) >= 2:
            amc_scan_max = int(cmd_params[1])
        amc_scan(ser, amc_scan_min, amc_scan_max, pc_id, verbosity)
        sys.exit(0)

    # Assemble the binary AMC command sequence.
    amc_cmd_bin = parse_amc_cmd(amc_cmd, cmd_params)

    # Assemble the full AMC frame.
    amc_frame, amc_crc = amc_frame_assemble(amc_id, sm_driver_id, pc_id, amc_cmd_bin)

    # Print messages about the AMC frame for debugging.
    amc_frame_print(ser, amc_frame, amc_crc, verbosity)

    # Send frame to AMC controller.
    amc_frame_send(ser, amc_frame)

    # Wait a while for the answer from the AMC controller.
    time.sleep(0.1)

    # Read answer from AMC controller.
    amc_answer = amc_answer_read(ser, verbosity)

    # Evaluate the answer from the AMC controller.
    amc_answer_eval(amc_answer, amc_id, sm_driver_id, pc_id, amc_cmd_bin, amc_cmd, verbosity)

# Scan the RS-485 port for active controllers.
def amc_scan(ser, amc_id_start, amc_id_stop, pc_id, verbosity):
    sm_driver_id = 0
    amc_cmd_bin = AMC_CMD_STATUS
    amc_id_list = []
    for amc_id in range(amc_id_start, amc_id_stop + 1):
        if verbosity >= 1:
            print("\rTesting AMC ID {0:d}".format(amc_id), end = '', flush=True)
        amc_frame, amc_crc = amc_frame_assemble(amc_id, sm_driver_id, pc_id, amc_cmd_bin)
        amc_frame_send(ser, amc_frame)
        time.sleep(0.1)
        amc_answer = amc_answer_read(ser, 0)
        if len(amc_answer) != AMC_ANS_HEADER_LEN + AMC_ANS_STATUS_LEN + 1 + AMC_ANS_XSTATUS_LEN + AMC_ANS_CRC_LEN:
            continue
        if amc_answer[1] != AMC_FRAME_ACK:
            continue
        if amc_answer[2] != pc_id:
            continue
        if amc_answer[3] != amc_id:
            continue
        amc_id_list.append(amc_id)
    # Clear the line.
    print("\r                                                  ", end = '', flush=True)
    # Print list of active AMC IDs.
    print("\rActive AMC IDs:", end = '', flush=True)
    for amc_id in amc_id_list:
        print(" {0:d}".format(amc_id), end='')
    print()



#####################################################################
# Open and set up the serial port.
#####################################################################

def serial_open(port, baudrate, enable_rs_485):
    # Open the serial port.
    try:
        ser = serial.Serial(
            port=port,
            baudrate=baudrate,
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
        print("Error setting up the serial port {0:s}:".format(port))
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
        print("Error setting the RS-485 parameters for serial port {0:s}:".format(port))
        print(error)
        sys.exit(1)

    return ser



#####################################################################
# Main function.
#####################################################################

if __name__ == '__main__':
    # Open the serial device and set its parameters.
    ser_dev = serial_open(args.serial_device, 115200, args.enable_rs_485)
    # Perform communication with the AMC controller.
    amc_comm(ser_dev, args.verbosity)

