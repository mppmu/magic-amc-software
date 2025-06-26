#!/bin/sh
#
# File: apptainer-magic-amc-suse.sh
# Auth: M. Fras, Electronics Division, MPI for Physics, Munich
# Mod.: M. Fras, Electronics Division, MPI for Physics, Munich
# Date: 22 May 2025
# Rev.: 26 Jun 2025
#
# Shell script for launching a shell within the MAGIC AMC openSUSE 10.2 container.
#

export PROJECT_INFO_MPI="<MAGIC AMC - openSUSE 10.2> "
apptainer exec $@ -B /opt /opt/Apptainer/Images/magic-amc-suse.sif bash

