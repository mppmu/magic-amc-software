#!/bin/sh
#
# File: amc_scan.sh
# Auth: M. Fras, Electronics Division, MPI for Physics, Munich
# Mod.: M. Fras, Electronics Division, MPI for Physics, Munich
# Date: 16 May 2025
# Rev.: 16 May 2025
#
# Scan all 8 channels (ports) of the MAGIC I AMC system for active AMC boxes
# and list their AMC IDs.
#
# Notes:
# * This script is intended to be use with the new PC7 (Advantech ITA-1711).
# * Standard connections to the AMC system:
#   AMC channel         Port at ITA-1711        Serial device
#   1                   COM3                    /dev/ttyS2
#   2                   COM4                    /dev/ttyS3
#   3                   COM5                    /dev/ttyS4
#   4                   COM6                    /dev/ttyS5
#   5                   COM7                    /dev/ttyS6
#   6                   COM8                    /dev/ttyS7
#   7                   COM9                    /dev/ttyS8
#   8                   COM10                   /dev/ttyS9
#

AMC_CHANNELS="1 2 3 4 5 6 7 8"
AMC_CHANNEL_TTY_OFFSET=1
AMC_ID_LOW="0"
AMC_ID_HIGH="255"

AMC_TEST="./amc_test.py"
AMC_TEST_COMMAND="-c scan ${AMC_ID_LOW} ${AMC_ID_HIGH}"
AMC_TEST_PARAMS="-v 1"

for amc_ch in ${AMC_CHANNELS}; do
    TTYS=`echo "/dev/ttyS$((amc_ch + AMC_CHANNEL_TTY_OFFSET))"`
    echo "Scanning AMC channel $amc_ch, serial port ${TTYS}."
    ${AMC_TEST} -d ${TTYS} ${AMC_TEST_PARAMS} ${AMC_TEST_COMMAND}
done

