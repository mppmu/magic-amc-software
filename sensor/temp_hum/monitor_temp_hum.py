#!/usr/bin/env python3
#
# File: monitor_temp_hum.py
# Auth: O. Leitel, Electronics Division, MPI for Physics, Munich
# Mod.: M. Fras, Electronics Division, MPI for Physics, Munich
# Date: 12 Aug 2026
# Rev.: 02 Sep 2026
#
# Python script to continuously read the temperature and humidity values of two
# OHT20-C USB sensor device and store them into CSV or text files.
#



import struct
import time
import csv
import os
import sys
import zipfile
import pathlib
from datetime import datetime
import serial
import serial.tools.list_ports



BAUD = 9600
POLL_INTERVAL_SEC = 10.0  # How often to read measurements.
CSV_FILE_PREFIX = "/home/sensor/log/monitor_temp_hum_"  # CSV output file prefix.
CSV_FILE_SUFFIX = ".csv"                                # CSV output file suffix.
CSV_FILE_ENABLE = True
TXT_FILE_PREFIX = "/home/sensor/log/monitor_temp_hum_"  # Text output file prefix.
TXT_FILE_SUFFIX = ".txt"                                # Text output file suffix.
TXT_FILE_ENABLE = True
VERBOSITY = 1



# ─────────────────────────────────────────────────────────────
# Channel definitions (confirmed from capture string analysis).
# ─────────────────────────────────────────────────────────────
CHANNELS = [
    {"id": 0, "seq_lo": 0x7B, "label": "Relative Humidity", "unit": "%RH"},
    {"id": 1, "seq_lo": 0x7A, "label": "Temperature",       "unit": "°C"},
    {"id": 2, "seq_lo": 0x79, "label": "Absolute Humidity", "unit": "g/m³"},
    {"id": 3, "seq_lo": 0x80, "label": "Dewpoint",          "unit": "°C"},
]



# ─────────────────────────────────────────────────────────────
# CSV helper.
# ─────────────────────────────────────────────────────────────

def init_csv(filepath):
    """
    Creates the CSV file with a header if it does not yet exist. If it already
    exists, data is appended to it. The header contains columns for both
    sensors.
    """
    # Create path if it does not exist.
    pathlib.Path(filepath).parent.mkdir(parents=True, exist_ok=True)
    file_exists = os.path.isfile(filepath)
    with open(filepath, mode='a', newline='', encoding='utf-8') as f:
        writer = csv.writer(f, delimiter=';')
        if not file_exists:
            writer.writerow([
                "Date",
                "Time",
                # Sensor 1.
                "S1 Temperature (°C)",
                "S1 Relative Humidity (%RH)",
                "S1 Absolute Humidity (g/m³)",
                "S1 Dewpoint (°C)",
                # Sensor 2.
                "S2 Temperature (°C)",
                "S2 Relative Humidity (%RH)",
                "S2 Absolute Humidity (g/m³)",
                "S2 Dewpoint (°C)",
            ])
            if VERBOSITY >= 1:
                print(f"✓ CSV file: {os.path.abspath(filepath)}")


def write_csv(filepath, timestamp,
              temperature1, humidity1, abs_humidity1, dewpoint1,
              temperature2, humidity2, abs_humidity2, dewpoint2):
    """
    Write a line containing the date, time, and all measured values from both
    sensors. Missing values are entered as ‘N/A’.
    """
    date_str = timestamp.strftime('%Y-%m-%d')
    time_str = timestamp.strftime('%H:%M:%S.%f')[:-3]

    def fmt_temp(v):
        return f"{v:+03.1f}" if v is not None else "N/A"

    def fmt_val(v):
        return f"{v:03.1f}" if v is not None else "N/A"

    with open(filepath, mode='a', newline='', encoding='utf-8') as f:
        writer = csv.writer(f, delimiter=';')
        writer.writerow([
            date_str, time_str,
            # Sensor 1.
            fmt_temp(temperature1),
            fmt_val(humidity1),
            fmt_val(abs_humidity1),
            fmt_temp(dewpoint1),
            # Sensor 2.
            fmt_temp(temperature2),
            fmt_val(humidity2),
            fmt_val(abs_humidity2),
            fmt_temp(dewpoint2),
        ])


def init_txt(filepath):
    """
    Creates the text file with the header if it does not yet exist. If it
    already exists, data is appended to it. The header contains columns for
    both sensors.
    """
    # Create path if it does not exist.
    pathlib.Path(filepath).parent.mkdir(parents=True, exist_ok=True)
    file_exists = os.path.isfile(filepath)
    with open(filepath, mode='a', newline='', encoding='utf-8') as f:
        if not file_exists:
            f.write(
                "Date   Time   "
                "S1_Temperature(°C)   S1_RelHumidity(%RH)   S1_AbsHumidity(g/m³)   S1_Dewpoint(°C)   "
                "S2_Temperature(°C)   S2_RelHumidity(%RH)   S2_AbsHumidity(g/m³)   S2_Dewpoint(°C)\n"
            )
            if VERBOSITY >= 1:
                print(f"✓ Text file: {os.path.abspath(filepath)}")


def write_txt(filepath, timestamp,
              temperature1, humidity1, abs_humidity1, dewpoint1,
              temperature2, humidity2, abs_humidity2, dewpoint2):
    """
    Write a line containing the date, time, and all measured values from both
    sensors. Missing values are entered as ‘N/A’.
    """
    date_str = timestamp.strftime('%Y %m %d')
    time_str = timestamp.strftime('%H %M %S %f')[:-3]

    def fmt_temp(v):
        return f"{v:+03.1f}" if v is not None else "N/A"

    def fmt_val(v):
        return f"{v:03.1f}" if v is not None else "N/A"

    with open(filepath, mode='a', newline='', encoding='utf-8') as f:
        f.write(
            f"{date_str} {time_str} "
            f"{fmt_temp(temperature1)} {fmt_val(humidity1)} "
            f"{fmt_val(abs_humidity1)} {fmt_temp(dewpoint1)} "
            f"{fmt_temp(temperature2)} {fmt_val(humidity2)} "
            f"{fmt_val(abs_humidity2)} {fmt_temp(dewpoint2)}\n"
        )



# ─────────────────────────────────────────────────────────────
# Protocol helpers.
# ─────────────────────────────────────────────────────────────

def build_tx(seq_lo, cmd_bytes):
    """
    Build 20-byte TX frame.
    Format: [seq_lo][0x05][0x00][0xFC][cmd...][0x00 padding]
    """
    frame = bytearray(20)
    frame[0] = seq_lo & 0xFF
    frame[1] = 0x05
    frame[2] = 0x00
    frame[3] = 0xFC
    for i, b in enumerate(cmd_bytes):
        if 4 + i < 20:
            frame[4 + i] = b
    return bytes(frame)


def rts_write(ser, data):
    """
    RS-485 half-duplex write.
    Assert RTS → write → wait for transmit → release RTS.
    At 9600 baud, 20 bytes = ~20ms transmit time.
    """
    ser.rts = True
    time.sleep(0.005)
    ser.write(data)
    ser.flush()
    time.sleep(0.025)
    ser.rts = False
    time.sleep(0.005)


def read_one(ser, timeout=1.0):
    """Read exactly 20 bytes with timeout. Returns bytes or b''."""
    ser.timeout = timeout
    data = ser.read(20)
    return data


def read_multi(ser, max_pkts=20, pkt_timeout=0.5):
    """
    Read multiple 20-byte response packets until timeout or
    all-zero terminator packet is received.
    Returns list of valid packets.
    """
    packets = []
    ser.timeout = pkt_timeout
    while len(packets) < max_pkts:
        rx = ser.read(20)
        if len(rx) == 0:
            break
        packets.append(rx)
        if rx[4:] == bytes(16):
            break
    return packets


def is_valid(rx):
    """Check response has correct FC marker and minimum length."""
    return len(rx) == 20 and rx[2] == 0xFC


def to_float(data, offset):
    """Safely unpack IEEE 754 float. Returns None on error or NaN."""
    if len(data) < offset + 4:
        return None
    v = struct.unpack_from('<f', data, offset)[0]
    if v != v or abs(v) > 1e10:
        return None
    return v



# ─────────────────────────────────────────────────────────────
# Device operations.
# ─────────────────────────────────────────────────────────────

def handshake(ser):
    """
    Send authentication/identification frame.
    Returns device name string or None on failure.
    """
    tx = bytes([
        0xCA, 0x02, 0x00, 0xFC,
        0xD0, 0x77, 0x26, 0x65,
        0x4D, 0xF4, 0x29, 0xA4,
        0xE1, 0x59, 0x11, 0x4B,
        0x7D, 0xDA, 0x1E, 0xA4
    ])
    rts_write(ser, tx)
    rx = read_one(ser, timeout=2.0)
    if is_valid(rx):
        return rx[4:11].decode('ascii', errors='ignore').rstrip('\x00')
    return None


def trigger_measurement(ser):
    """Send measurement trigger command. Returns True if ACK received."""
    tx = build_tx(0xFA, bytes([0x04, 0x21, 0x36, 0x08,
                                0x0A, 0x01, 0x08, 0x1A, 0x14]))
    rts_write(ser, tx)
    rx = read_one(ser, timeout=1.0)
    return is_valid(rx) and rx[0] == 0x0D


def read_channel(ser, channel_def):
    """
    Read one live measurement channel.
    Returns (primary_value, secondary_value) or (None, None).
    """
    ch   = channel_def["id"]
    s_lo = channel_def["seq_lo"]
    tx   = build_tx(s_lo, bytes([0x80, 0x16, ch, 0x03]))
    rts_write(ser, tx)
    rx = read_one(ser, timeout=1.0)
    if is_valid(rx):
        primary   = to_float(rx, 8)
        secondary = to_float(rx, 12)
        return primary, secondary
    return None, None


def read_channel_config(ser, cmd_byte, seq_lo):
    """Read channel configuration / metadata packets."""
    tx = build_tx(seq_lo, bytes([0x11, cmd_byte]))
    rts_write(ser, tx)
    return read_multi(ser, max_pkts=20, pkt_timeout=0.5)


def read_device_status(ser):
    """Read device status register. Returns raw 20-byte response or None."""
    tx = build_tx(0x80, bytes([0x80, 0x12, 0x00, 0x02]))
    rts_write(ser, tx)
    rx = read_one(ser, timeout=1.0)
    return rx if is_valid(rx) else None


def get_measurements(ser):
    """
    Read all 4 channels and return dict of results.
    Also re-triggers measurement before reading.
    """
    trigger_measurement(ser)
    time.sleep(0.05)
    ser.reset_input_buffer()

    results = {}
    for ch in CHANNELS:
        primary, secondary = read_channel(ser, ch)
        results[ch["label"]] = {
            "value":     primary,
            "secondary": secondary,
            "unit":      ch["unit"],
        }
        time.sleep(0.02)
        ser.reset_input_buffer()

    return results



# ─────────────────────────────────────────────────────────────
# Startup & channel configuration decode.
# ─────────────────────────────────────────────────────────────

def decode_config_packets(packets, label):
    """Decode and print channel config multi-packet response."""
    if VERBOSITY >= 3:
        print(f"\n  [{label}] {len(packets)} config packets:")
    for i, pkt in enumerate(packets):
        if not is_valid(pkt):
            continue
        f1 = to_float(pkt, 8)
        f2 = to_float(pkt, 12)
        f3 = to_float(pkt, 16)

        try:
            text = pkt[4:20].decode('ascii', errors='replace')
            text = ''.join(c if 32 <= ord(c) < 127 else '.' for c in text)
        except Exception:
            text = ''

        if VERBOSITY >= 3:
            print(f"    pkt[{i+1}]: f@8={f1:>12.4f}  f@12={f2:>12.4f}  "
                  f"f@16={f3:>10.4f}  ascii=[{text}]")

        if i == 0 and f1 is not None and f2 is not None:
            if f1 == 0.0 and f2 == 100.0:
                if VERBOSITY >= 3:
                    print("           → Range: 0 – 100")
            elif f1 == -50.0 and f2 == 150.0:
                if VERBOSITY >= 3:
                    print("           → Range: -50 – 150 °C")



# ─────────────────────────────────────────────────────────────
# Main.
# ─────────────────────────────────────────────────────────────

def find_oht20_ports():
    """
    Search for connected OHT20 sensor devices.
    Returns list of port device strings, sorted by port name.
    Maximum 2 devices are used.
    """
    ports = list(serial.tools.list_ports.comports())
    ports.sort()
    oht20_devices = []
    for port in ports:
        if VERBOSITY >= 3:
            print(f"{port} ; {port.device} ; {port.description} ; {port.hwid}")
        # Filter for OHT20 device(s).
        if "OHT20" in port.description:
            if VERBOSITY >= 1:
                print(f"Found OHT20 device \"{port.hwid}\" on port {port.device}.")
            oht20_devices.append(port.device)
            if len(oht20_devices) == 2:
                break   # We only support up to 2 sensors.
    return oht20_devices



def open_port(port):
    """Open a serial port for communication with an OHT20 sensor."""
    if port is None:
        return None
    ser = serial.Serial(
        port=port,
        baudrate=BAUD,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=2.0,
        rtscts=False,
        dsrdtr=False,
        xonxoff=False,
    )
    ser.dtr = True
    ser.rts = False
    time.sleep(0.2)
    ser.reset_input_buffer()
    ser.reset_output_buffer()
    return ser



def init_sensor(ser, sensor_label):
    """
    Run handshake + channel config + status read for one sensor.
    Returns device name string, or None on failure.
    """
    # ── Handshake ──────────────────────────────
    if VERBOSITY >= 1:
        print(f"\n[{sensor_label}] Connecting...")
    device_name = handshake(ser)
    if not device_name:
        if VERBOSITY >= 1:
            print(f"[{sensor_label}] ERROR: Handshake failed. Check port and wiring.")
        return None
    if VERBOSITY >= 1:
        print(f"[{sensor_label}] ✓ Connected to {device_name} on port {ser.port}.")
    time.sleep(0.1)
    ser.reset_input_buffer()

    # ── Read channel configs (once at startup) ─
    if VERBOSITY >= 2:
        print(f"\n[{sensor_label}] Reading channel configurations...")
    config_cmds = [
        (0x00, 0xFD, "Relative Humidity"),
        (0x01, 0xFE, "Temperature"),
        (0x02, 0xFB, "Absolute Humidity"),
        (0x03, 0xFC, "Raw Temp Counts"),
    ]
    for cmd_b, s_lo, lbl in config_cmds:
        pkts = read_channel_config(ser, cmd_b, s_lo)
        decode_config_packets(pkts, lbl)
        time.sleep(0.1)
        ser.reset_input_buffer()

    # ── Device status ──────────────────────────
    if VERBOSITY >= 2:
        print(f"\n[{sensor_label}] Device status:")
    status = read_device_status(ser)
    if status:
        year  = struct.unpack_from('<H', status, 16)[0]
        flags = status[4:16].hex(' ')
        if VERBOSITY >= 2:
            print(f"  Flags : {flags}")
            print(f"  Year  : {year}")
    time.sleep(0.1)
    ser.reset_input_buffer()

    return device_name



def print_measurements_dual(results1, results2):
    """Print measurements from both sensors side by side."""
    print(f"\n{'─'*70}")
    print(f"  {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"{'─'*70}")
    print(f"  {'Measurement':<22}  {'Sensor 1':>14}  {'Sensor 2':>14}")
    print(f"{'─'*70}")
    for ch in CHANNELS:
        name = ch["label"]
        unit = ch["unit"]
        # Sensor 1.
        d1  = results1.get(name, {})
        v1  = d1.get("value")
        s1  = f"{v1:>8.3f} {unit}" if v1 is not None else "--- (no data)"
        # Sensor 2.
        d2  = results2.get(name, {}) if results2 is not None else {}
        v2  = d2.get("value")
        s2  = f"{v2:>8.3f} {unit}" if v2 is not None else "--- (no data)"
        print(f"  {name:<22}  {s1:>14}  {s2:>14}")
    print(f"{'─'*70}")



def zip_old_file(filepath):
    """Compress a file into a ZIP archive and remove the original."""
    if pathlib.Path(filepath).is_file():
        zip_path = filepath + '.zip'
        with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
            zipf.write(filepath, arcname=os.path.basename(filepath))
        os.remove(filepath)
        if VERBOSITY >= 1:
            print(f"✓ Archived: {zip_path}")



if __name__ == "__main__":
    print("Monitor temperature and humidity using the OHT20-C sensor.")

    # ── Find OHT20 sensor devices ──────────────
    oht20_ports = find_oht20_ports()

    if len(oht20_ports) == 0:
        print("ERROR: No OHT20 device found. Check USB connection.")
        sys.exit(1)

    if len(oht20_ports) == 1:
        print("INFO: Only one OHT20 device found. Sensor 2 data will be logged as N/A.")

    # ── Open serial ports ──────────────────────
    ser1 = open_port(oht20_ports[0])
    ser2 = open_port(oht20_ports[1]) if len(oht20_ports) >= 2 else None

    # ── Initialise sensors ─────────────────────
    name1 = init_sensor(ser1, "Sensor 1")
    if not name1:
        ser1.close()
        if ser2:
            ser2.close()
        sys.exit(1)

    name2 = None
    if ser2 is not None:
        name2 = init_sensor(ser2, "Sensor 2")
        if not name2:
            # Sensor 2 failed – continue with Sensor 1 only.
            print("WARNING: Sensor 2 initialisation failed. Continuing with Sensor 1 only.")
            ser2.close()
            ser2 = None

    # ── Polling loop ───────────────────────────
    if VERBOSITY >= 1:
        print(f"\nPolling every {POLL_INTERVAL_SEC} s.")
        if CSV_FILE_ENABLE:
            print(f"Logging to: {os.path.abspath(CSV_FILE_PREFIX)}*{CSV_FILE_SUFFIX}")
        if TXT_FILE_ENABLE:
            print(f"Logging to: {os.path.abspath(TXT_FILE_PREFIX)}*{TXT_FILE_SUFFIX}")

    sys.stdout.flush()

    try:
        last_date    = datetime.now().date()
        csv_file_last = None
        txt_file_last = None

        while True:
            now          = datetime.now()
            current_date = now.date()

            # ── Read measurements from both sensors ────
            results1 = get_measurements(ser1)
            results2 = get_measurements(ser2) if ser2 is not None else None

            # ── Print to console ───────────────────────
            if VERBOSITY >= 2:
                print_measurements_dual(results1, results2 or {})

            # ── Extract values – Sensor 1 ──────────────
            temperature1  = results1.get("Temperature",       {}).get("value")
            humidity1     = results1.get("Relative Humidity", {}).get("value")
            abs_humidity1 = results1.get("Absolute Humidity", {}).get("value")
            dewpoint1     = results1.get("Dewpoint",          {}).get("value")

            # ── Extract values – Sensor 2 ──────────────
            if results2 is not None:
                temperature2  = results2.get("Temperature",       {}).get("value")
                humidity2     = results2.get("Relative Humidity", {}).get("value")
                abs_humidity2 = results2.get("Absolute Humidity", {}).get("value")
                dewpoint2     = results2.get("Dewpoint",          {}).get("value")
            else:
                # Sensor 2 unavailable – write N/A placeholders.
                temperature2 = humidity2 = abs_humidity2 = dewpoint2 = None

            # ── Write CSV ──────────────────────────────
            if CSV_FILE_ENABLE:
                csv_file = CSV_FILE_PREFIX + now.strftime('%Y-%m-%d') + CSV_FILE_SUFFIX
                init_csv(csv_file)
                write_csv(
                    csv_file, now,
                    temperature1, humidity1, abs_humidity1, dewpoint1,
                    temperature2, humidity2, abs_humidity2, dewpoint2,
                )
                # Zip previous day's file when date rolls over.
                if current_date != last_date and csv_file_last is not None:
                    zip_old_file(csv_file_last)
                csv_file_last = csv_file

            # ── Write TXT ──────────────────────────────
            if TXT_FILE_ENABLE:
                txt_file = TXT_FILE_PREFIX + now.strftime('%Y-%m-%d') + TXT_FILE_SUFFIX
                init_txt(txt_file)
                write_txt(
                    txt_file, now,
                    temperature1, humidity1, abs_humidity1, dewpoint1,
                    temperature2, humidity2, abs_humidity2, dewpoint2,
                )
                # Zip previous day's file when date rolls over.
                if current_date != last_date and txt_file_last is not None:
                    zip_old_file(txt_file_last)
                txt_file_last = txt_file

            # ── Update date tracker ────────────────────
            last_date = current_date

            time.sleep(POLL_INTERVAL_SEC)

    except KeyboardInterrupt:
        print("\nStopped by user.")
    except Exception as e:
        print(f"\nERROR: {e}")
    finally:
        if ser1 is not None:
            ser1.close()
        if ser2 is not None:
            ser2.close()
        if VERBOSITY >= 1:
            print("Port(s) closed.")

