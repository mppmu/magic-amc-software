#!/usr/bin/env python3
#
# File: plot_temp_hum.py
# Auth: M. Fras, Electronics Division, MPI for Physics, Munich
# Mod.: M. Fras, Electronics Division, MPI for Physics, Munich
# Date: 01 Sep 2026
# Rev.: 02 Sep 2026
#
# Python script to plot the temperature and humidity data stored in a CSV file
# by the "monitor_temp_hum.py" script.
#
# Template created with Google AI using this prompt:
# "Write a Python program that plots data of a csv file."
#



import os
import sys
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import pandas as pd



def plot_csv_data(file_path):
    # Check if the CSV file exists.
    if not os.path.exists(file_path):
        print(f"Error: The file '{file_path}' does not exist.")
        return

    try:
        # Read the CSV file and check the columns.
        df = pd.read_csv(file_path, sep=';')
        # The CSV file must contain 10 columns.
        # 0: Date
        # 1: Time
        # 2: S1 Temperature (°C)
        # 3: S1 Relative Humidity (%RH)
        # 4: S1 Absolute Humidity (g/m³)
        # 5: S1 Dewpoint (°C)
        # 6: S2 Temperature (°C)
        # 7: S2 Relative Humidity (%RH)
        # 8: S2 Absolute Humidity (g/m³)
        # 9: S2 Dewpoint (°C)
        if len(df.columns) != 10:
            print("Error: The CSV file must have 10 columns.")
            # Show names of columns.
            print("Columns found:")
            for i, col in enumerate(df.columns):
                print(f"{i}: {col}")
            return

        # Read the CSV file and combine the columns "Date" and "Time" into "Timestamp".
        df = pd.read_csv(file_path, sep=';', parse_dates={"Timestamp": ["Date", "Time"]})
        s1_temp = df.columns[1]
        s1_hum  = df.columns[2]
        s2_temp = df.columns[5]
        s2_hum  = df.columns[6]

        # Create the diagram.
        fig, ax_t = plt.subplots(figsize=(12, 8))

        # Plot temperatures.
        ax_t.set_xlabel('Timestamp')
        ax_t.set_ylabel('Temperature [°C]', color='black')
        line_t_et = ax_t.plot(df['Timestamp'], df[s1_temp], color='red', linestyle='-', label='Temp. east tower')
        line_t_cb = ax_t.plot(df['Timestamp'], df[s2_temp], color='orange', linestyle='-', label='Temp. central box')
        ax_t.tick_params(axis='y', labelcolor='black')

        # Plot humidities.
        ax_h = ax_t.twinx()
        ax_h.set_ylabel('Himdity [%rH]', color='black')
        line_h_et = ax_h.plot(df['Timestamp'], df[s1_hum], color='blue', linestyle='-', label='Hum. east tower')
        line_h_cb = ax_h.plot(df['Timestamp'], df[s2_hum], color='lightblue', linestyle='-', label='Hum. central box')
        ax_h.tick_params(axis='y', labelcolor='black')

        # Create legend.
        lines = line_t_et + line_t_cb + line_h_et + line_h_cb
        labels = [l.get_label() for l in lines]
        ax_t.legend(lines, labels, bbox_to_anchor=(1.05, 1), loc='upper left')

        # Set plot and window titles.
        title = f"Temperature and humidity from '{file_path}'"
        fig.canvas.manager.set_window_title(title)
        fig.suptitle(title)

        # Configure the labels on the x-axis.
        ax_t.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m-%d %H:%M'))
        fig.autofmt_xdate(rotation=45)

        # Show the diagram.
        plt.tight_layout()
        plt.show()

    except Exception as e:
        print(f"The following error has occured: {e}")



if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: {os.path.basename(sys.argv[0])} <CSV file>")
        sys.exit(1)
    else:
        csv_filename = sys.argv[1]
        plot_csv_data(csv_filename)

