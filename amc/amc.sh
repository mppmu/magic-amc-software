#!/bin/sh
#
# File: amc.sh
# Auth: M. Fras, Electronics Division, MPI for Physics, Garching
# Mod.: M. Fras, Electronics Division, MPI for Physics, Garching
# Date: 17 May 2025
# Rev.: 04 Jul 2025
#
# Launch the MAGIC AMC software inside an Apptainer container.
#
# Note: The AMC software tends to crash on a multi-core system. It seems not to
#       be thread safe. The work-around is to limit it to one core using the
#       taskset command.
#



export PROJECT_INFO_MPI="<MAGIC AMC - Ubuntu 18.04 - AMC> "
export TERM=xterm

# The ACM software needs to be started within the directory
# "/home/operator/V4.50" to be able to access all required files.
cd /home/operator/V4.50

#apptainer exec $@ -B /opt,/mnt,/remote /opt/Apptainer/Images/magic-amc-ubuntu.sif /bin/bash -c ./amc
#apptainer exec $@ -B /opt,/mnt,/remote /opt/Apptainer/Images/magic-amc-suse.sif /bin/bash -c ./amc

# This one works:
apptainer exec $@ -B /opt,/mnt,/remote /opt/Apptainer/Images/magic-amc-suse.sif taskset -c 0 ./amc

