Dependencies for the MAGIC Active Mirror Control (AMC) Software
===============================================================

## General Remarks

In order to compile and run the AMC software and the SBIG software, some
dependencies need to be installed. In case the AMC software is running inside a
container, some of these dependencies must be installed in the container while
other ones need to be installed on the host system.  

A recent 64-bit Ubuntu system (Ubuntu 24.04.2 LTS) has been successfully used
as a host.
[Apptainer](https://apptainer.org/ "Apptainer is an open source container platform")
was used as container platform.  

The AMC software can be compiled and executed on both an openSUSE 10.2 and an
Ubuntu 18.04 container. However, some issues have been discovered:
* openSUSE 10.2 container:
  - The C compiler `gcc` only works inside a *writable sandbox* container. It
    fails inside a non-writable sandbox container and inside a container
    started from the `*.sif` file. The reason is unclear. However, the compiled
    software runs well inside the non-writable containers.
  - The software builds and runs well. However, it tends to crash after the GUI
    appears. This may be related to several threads of the software, which may
    not be really thread safe. The stability can be improved by limiting the
    AMC software to one single CPU core using the `taskset` command:
    ```
    taskset -c 0 ./amc
    ```
* Ubuntu 18.04 container:
  - The software builds well. It starts up and initializes the mirrors, but it
    gets stuck during the adjustment of the mirrors. The reason is unclear.

The AMC test software written in Python can run on the host system. It requires
Python 3.9 or higher and the pySerial module to be installed.



## Ubuntu 24.04 Host System

The SBIG camera is attached to the PC as USB device. In order to be correctly
detected and operated, udev rlues and some firmware files are required.


### Udev Rules

* The udev rules are located in the directory `depend/udev` of this repository.
* If You run the AMC software inside a container, the udev rules must be copied
  to the udev rules directory of the **host** system.
* Copy the file `51-sbig-debian.rules` into the directory containing the udev
  rules. In Ubuntu 22.04, this is the directory `/etc/udev/rules.d`.


### SBIG Firmware

* The SBIG firmware files are located in the directory
  `depend/sbig/firmware` of this repository.
* If You run the AMC software inside a container, the SBIG firmware files must
  be copied to the firmware directory of the **host** system.
* Copy all files to the firmware directory `/lib/firmware`. 



## openSUSE 10.2 Container

To build and set up the openSUSE 10.2 container, follow these instructions:
* Download the openSUSE 10.2 i386 ISO image from here:  
  [https://ftp5.gwdg.de/pub/opensuse/discontinued/distribution/10.2/iso/dvd/openSUSE-10.2-GM-DVD-i386.iso](https://ftp5.gwdg.de/pub/opensuse/discontinued/distribution/10.2/iso/dvd/openSUSE-10.2-GM-DVD-i386.iso)
* Execute the following commands as root.
  ```
  sudo -i
  ```
* Create a working directory and change into it:
  ```
  mkdir magic-amc
  cd magic-amc
  ```
* Mount the openSUSE 10.2 i386 ISO image `openSUSE-10.2-GM-DVD-i386.iso`:
  ```
  mount -o loop /path/to/openSUSE-10.2-GM-DVD-i386.iso /mnt
  ```
* Create a link called `openSUSE-10.2` to the mount directory:
  ```
  ln -s /mnt openSUSE-10.2
  ```
* Use Apptainer to build a sandbox container using the
  [magic-amc-suse.def](depend/apptainer/opensuse/10.2/magic-amc-suse.def)
  file:
  ```
  apptainer build --sandbox magic-amc-suse magic-amc-suse.def
  ```
* Launch a shell in the (writable) container:
  ```
  apptainer shell --writable magic-amc-suse
  ```
* Install the remaining dependencies inside the container:
  ```
  rpm -ihv --nodeps /path/to/sbig-1.1-9.i586.rpm
  rpm -ihv --nodeps /path/to/sbig-firmware-1.0-13.noarch.rpm
  rpm -ihv --nodeps /path/to/xforms-1.0-266.i586.rpm
  rpm -ihv --nodeps /path/to/xforms-devel-1.0-266.i586.rpm
  rpm -ihv --nodeps /path/to/xforms-glx-1.0-266.i586.rpm
  cp -a /path/to/libsbigcam.a /usr/local/lib/
  cp -a /path/to/libsbigcam.so /usr/local/lib/
  chown root:root /usr/local/lib/libsbigcam.*
  chmod 644 /usr/local/lib/libsbigcam.*
  cp -a /path/to/libcfitsio.a /usr/local/lib/
  chown root:root /usr/local/lib/libcfitsio.a
  chmod 644 /usr/local/lib/libcfitsio.a
  cp -a /path/to/fitsio*.h /usr/local/include/
  cp -a /path/to/longnam.h /usr/local/include/
  chown root:root /usr/local/include/fitsio*.h
  chown root:root /usr/local/include/longnam.h
  chmod 644 /usr/local/include/fitsio*.h
  chmod 644 /usr/local/include/longnam.h
  ldconfig
  ```
* Exit the container.
  ```
  exit
  ```
* Build a sif container image from the sandbox:
  ```
  apptainer build magic-amc-suse.sif magic-amc-suse
  ```



## Ubuntu 18.04 Container

The following instructions have been tested with a container running 64-bit
Ubuntu 18.04 with 32-bit software enabled.

### SBIG Libraries

* The SBIG libraries are located in the directory
  `depend/lib` of this repository.
* If You run the software inside a container, the SBIG libraries must be
  copied to the **container** system.
* Copy the files `libsbigcam.a` and `libsbigcam.so` to
  `/usr/local/lib`.
* If packages of `libsbigudrv` are available for your Linux distribution,
  it is highly recommended to use these. Install the library and its
  development package. Please note that the 32-bit version is required.
  In Ubuntu 18.04, run these commands:
  ```
  apt install libsbigudrv2:i386
  apt install libsbigudrv2-dev:i386
  ```
* If no packages of `libsbigudrv` are available, copy the files
  `libsbigudrv.a` and `libsbigudrv.so` to `/usr/local/lib`.


### Build tools and auxiliary packages.

* If You run the software inside a container, all build tools and auxiliary
  packages must be installed in the **container** system.
* For Ubuntu 18.04, run these commands:
  ```
  apt install autoconf
  apt install autogen
  apt install automake
  apt install build-essential
  apt install git
  apt install make
  apt install libcfitsio-dev
  apt install libforms-dev:i386
  apt install libtool
  apt install libturbo8_i386
  apt install libusb-1.0-0-dev
  apt install libxpm-dev:i386
  apt install libxpm4:i386
  apt install python3
  apt install python3-serial
  apt install saods9
  apt install xpa-tools
  ```

