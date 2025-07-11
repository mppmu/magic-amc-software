#!/bin/sh
#
# File: apptainer-magic-amc-suse-sandbox-writable.sh
# Auth: M. Fras, Electronics Division, MPI for Physics, Munich
# Mod.: M. Fras, Electronics Division, MPI for Physics, Munich
# Date: 22 May 2025
# Rev.: 11 Jul 2025
#
# Shell script for launching a shell within the MAGIC AMC openSUSE 10.2 container.
#

export PROJECT_INFO_MPI="<MAGIC AMC - openSUSE 10.2 (sandbox, writable)> "
apptainer exec $@ --writable -B /opt,/mnt /opt/Apptainer/Images/magic-amc-suse bash

