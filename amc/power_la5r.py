#!/usr/bin/env python3
#
# File: power_la5r.py
# Auth: M. Fras, Electronics Division, MPI for Physics, Munich
# Mod.: M. Fras, Electronics Division, MPI for Physics, Munich
# Date: 03 Aug 2026
# Rev.: 02 Sep 2026
#
# Python script to switch on/off parts of the MAGIC I AMC using a Lineeye LA-5R
# device.
#
# Note:
# - As the outputs of the Lineeye LA-5R are all open after power on and PC7
#   needs to start up after power on, a relay is used to make the inversion.
#   The output of the LA-5R drives the coil of the relay and the power of PC7
#   is connected to the NC (normally closed) port of the relay. Thus, the
#   function of the channel driving the PC7 power is inverted.
#
# The function "control_la5r_channel" was partially generated with Google AI
# using this prompt:
# "Write a Python software to switch on and off individual channels of the
#  Lineeye LA-5R device over the network."
#



import socket
import sys
import time



# Lineeye LA-5R device network settings.
LINEEYE_IP = "161.72.130.115"
LINEEYE_PORT = 10003

# Set verbosity level.
verbosity = 1



# Get the status of the relays of the Lineeye LA-5R device.
def status_la5r_channels(ip_address, port=10003):
    """
    Get the status of all 5 relay channels of the Lineye LA-5R device.
    """

    # Command to query the status of the relays.
    cmd_header = 0xE0

    # Assemble the packet as a bytearray.
    payload = bytes([cmd_header])

    try:
        # Establish TCP connection.
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(3.0)   # Timeout protection for network delays.
            s.connect((ip_address, port))

            # Send the command to the Lineeye LA-5R device.
            if verbosity >= 3:
                print("Sending command '0x{0:s}' to Lineeye LA-5R.".format(payload.hex().upper()))
            s.sendall(payload)

            # The Lineeye LA-5R device usually responds with a confirmation (echo of the status).
            response = s.recv(1024)
            if response:
                if verbosity >= 3:
                    print("Response from Lineeye LA-5R (hex): 0x{0:s}".format(response.hex().upper()))
            # Check for errors in response from Lineeye LA-5R device.
            if not response:
                print("Error: No response from the Lineeye LA-5R device!")
                sys.exit(12)
            else:
                if len(response) != 2:
                    print("Error: The response from the Lineeye LA-5R device should have 1 bytes, but it has {0:d}!".format(len(response)))
                    sys.exit(13)
                elif response[0] != cmd_header:
                    print("Error: The response from the Lineeye LA-5R device is wrong! The first byte should be 0x{0:02X}, but it is 0x{1:02X}!".format(cmd_header, response[0]))
                    sys.exit(14)
                else:
                    print("LA-5R output status:")
                    print("- PC7   : %s" % ('OFF' if response[1] & 0x01 else 'ON'))     # PC7 power is inverted!
                    print("- AMC-L : %s" % ('ON' if response[1] & 0x02 else 'OFF'))
                    print("- AMC-U : %s" % ('ON' if response[1] & 0x04 else 'OFF'))
                    print("- SBIG  : %s" % ('ON' if response[1] & 0x08 else 'OFF'))
                    print("- spare : %s" % ('ON' if response[1] & 0x10 else 'OFF'))

            # Clean up the connection.
            s.close()

    except socket.timeout:
        print("Error: Connection to {0:s} timed out.".format(ip_address))
        sys.exit(11)
    except Exception as e:
        print("A network error occurred: {0}".format(e))
        sys.exit(12)



# Switch on/off a single channel of the Lineeye LA-5R device.
def control_la5r_channel(ip_address, channel, turn_on, port=10003, show_status=True):
    """
    Controls a single relay channel of the Lineeye LA-5R.

    :param ip_address: IP address of the LA-5R device (e.g., '192.168.1.100')
    :param channel: Channel number (1 to 5)
    :param turn_on: True to switch ON, False to switch OFF
    :param port: TCP port (Factory default is 10003)
    """
    if not 1 <= channel <= 5:
        print("Error: Channel must be between 1 and 5.")
        sys.exit(10)

    # Show debug information.
    if verbosity >= 2:
        print("Turning {0:s} channel {1:d} of the Lineeye LA-5R device with IP address {2:s}.".format('ON' if turn_on else 'OFF', channel, ip_address))

    # The LA-5R uses 5 bits for masking (bit 0 for channel 1 up to bit 4 for channel 5).
    bit_position = channel - 1

    # Byte 1: The command for targeted single-channel control (specific DO command).
    cmd_header = 0xFC

    # Byte 2: State byte (Which state should be set? 1 = ON, 0 = OFF).
    if turn_on:
        state_mask = 1 << bit_position
    else:
        state_mask = 0x00

    # Byte 3: Mask byte (Which channel should be modified? 1 = change, 0 = ignore).
    channel_mask = 1 << bit_position

    # Assemble the packet as a bytearray.
    payload = bytes([cmd_header, state_mask, channel_mask])

    try:
        # Establish TCP connection.
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(3.0)   # Timeout protection for network delays.
            s.connect((ip_address, port))

            # Send the command to the Lineeye LA-5R device.
            if verbosity >= 3:
                print("Sending command '0x{0:s}' to Lineeye LA-5R: Channel {1:d} -> {2:s}.".format(payload.hex().upper(), channel, 'ON' if turn_on else 'OFF'))
            s.sendall(payload)

            # The Lineeye LA-5R device usually responds with a confirmation (echo of the status).
            response = s.recv(1024)
            if response:
                if verbosity >= 3:
                    print("Response from Lineeye LA-5R (hex): 0x{0:s}".format(response.hex().upper()))
            # Check for errors in response from Lineeye LA-5R device.
            if not response:
                print("Error: No response from the Lineeye LA-5R device!")
                sys.exit(12)
            else:
                if len(response) != 2:
                    print("Error: The response from the Lineeye LA-5R device should have 2 bytes, but it has {0:d}!".format(len(response)))
                    sys.exit(13)
                elif response[0] != cmd_header:
                    print("Error: The response from the Lineeye LA-5R device is wrong! The first byte should be 0x{0:02X}, but it is 0x{1:02X}!".format(cmd_header, response[0]))
                    sys.exit(14)
                else:
                    if show_status:
                        print("LA-5R output status:")
                        print("- PC7   : %s" % ('OFF' if response[1] & 0x01 else 'ON'))     # PC7 power is inverted!
                        print("- AMC-L : %s" % ('ON' if response[1] & 0x02 else 'OFF'))
                        print("- AMC-U : %s" % ('ON' if response[1] & 0x04 else 'OFF'))
                        print("- SBIG  : %s" % ('ON' if response[1] & 0x08 else 'OFF'))
                        print("- spare : %s" % ('ON' if response[1] & 0x10 else 'OFF'))

            # Clean up the connection.
            s.close()

    # Handle possible exceptions.
    except socket.timeout:
        print("Error: Connection to {0:s} timed out.".format(ip_address))
        sys.exit(11)
    except Exception as e:
        print("A network error occurred: {0}".format(e))
        sys.exit(12)



# Show help.
def print_help():
    print("Usage: power_la5r.py <component> <ON|OFF>")
    print("       power_la5r.py status")
    print()
    print("List of components:")
    print("- PC7   : PC7 (DO1) - Caution: This will interrupt the power of PC7!")
    print("- AMC   : All AMC boxes (DO2, DO3).")
    print("- AMC-L : Lower half of AMC boxes (DO2).")
    print("- AMC-U : Upper half of AMC boxes (DO3).")
    print("- SBIG  : SBIG camera (DO4).")
    print("- spare : Spare channel (DO5).")



# Main script with command line evaluation.
if __name__ == "__main__":

    # Evaluate command line arguments.
    if len(sys.argv) == 2:
        command = sys.argv[1].lower()
        if command == "status":
            status_la5r_channels(ip_address=LINEEYE_IP, port=LINEEYE_PORT)
            sys.exit(0)
        else:
            print_help()
            sys.exit(1)
    if len(sys.argv) != 3:
        print_help()
        sys.exit(1)
    component = sys.argv[1]
    on_off = sys.argv[2]

    # Evaluate on/off request.
    if on_off.lower() == "off":
        turn_on = False
    elif on_off.lower() == "on":
        turn_on = True
    else:
        print("Error: Unknown power state: '{0:s}'. Please specify either 'ON' or 'OFF'!".format(on_off))
        sys.exit(2)

    # Evaluate component to be switched on/off.
    if component.upper() == "PC7":
        # For safety: Let user confirm power cut of PC7!
        if not turn_on:
            reply = input("Do you really want to cut the power of PC7 (yes/no)? ")
            if reply.lower() != "yes":
                print("Operation aborted! Power of PC7 *not* turned off!")
                sys.exit(3)
        turn_on_pc7 = not turn_on    # PC7 power is inverted!
        control_la5r_channel(ip_address=LINEEYE_IP, port=LINEEYE_PORT, channel=1, turn_on=turn_on_pc7, show_status=True)
    elif component.upper() == "AMC":
        if verbosity >= 2:
            control_la5r_channel(ip_address=LINEEYE_IP, port=LINEEYE_PORT, channel=2, turn_on=turn_on, show_status=True)
        else:
            control_la5r_channel(ip_address=LINEEYE_IP, port=LINEEYE_PORT, channel=2, turn_on=turn_on, show_status=False)
        time.sleep(0.1)
        control_la5r_channel(ip_address=LINEEYE_IP, port=LINEEYE_PORT, channel=3, turn_on=turn_on, show_status=True)
    elif component.upper() == "AMC-L":
        control_la5r_channel(ip_address=LINEEYE_IP, port=LINEEYE_PORT, channel=2, turn_on=turn_on, show_status=True)
    elif component.upper() == "AMC-U":
        control_la5r_channel(ip_address=LINEEYE_IP, port=LINEEYE_PORT, channel=3, turn_on=turn_on, show_status=True)
    elif component.upper() == "SBIG":
        control_la5r_channel(ip_address=LINEEYE_IP, port=LINEEYE_PORT, channel=4, turn_on=turn_on, show_status=True)
    elif component.lower() == "spare":
        control_la5r_channel(ip_address=LINEEYE_IP, port=LINEEYE_PORT, channel=5, turn_on=turn_on, show_status=True)
    else:
        print("Error: Component '{0:s}' not supported!".format(component))
        print()
        print_help()
        sys.exit(4)

