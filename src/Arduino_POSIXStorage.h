/*
*********************************************************************************************************
*                                     Arduino_POSIXStorage Library
*
*                            Copyright 2023 Arduino SA. http://arduino.cc
*
*                           Original Author: A. Vidstrom (info@arduino.cc)
*
*                    Complements the POSIX storage functions already included in the Renesas
*                    core and the Arduino_USBHostMbed5 library.
*
*                    The library supports and is tested on:
*
*                    - Portenta C33 with Portenta Breakout Board (SD Card and USB Thumb Drive)
*                    - Portenta C33 with Portenta Vision Shield  (SD Card)
*                    - Portenta H7 with Portenta Breakout Board  (SD Card and USB Thumb Drive)
*                    - Portenta H7 with Portenta Vision Shield   (SD Card)
*                    - Portenta Machine Control                  (USB Thumb Drive)
*
*                    After making changes:
*                    
*                    - Make sure all test combinations finish without errors (tests directory)
*                    - Make sure the library itself compiles cleanly with all compiler warnings
*                      enabled (several lower-level libraries don't at the time of writing)
*                    - Review the whole library carefully, because there are many combinations
*                      to handle correctly
*
*
*                             SPDX-License-Identifier: LGPL-2.1-or-later
*
*                    This library is free software; you can redistribute it and/or
*                    modify it under the terms of the GNU Lesser General Public
*                    License as published by the Free Software Foundation; either
*                    version 2.1 of the License, or (at your option) any later version.
*
*                    This library is distributed in the hope that it will be useful,
*                    but WITHOUT ANY WARRANTY; without even the implied warranty of
*                    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*                    Lesser General Public License for more details.
*
*                    You should have received a copy of the GNU Lesser General
*                    Public License along with this library; if not, write to the
*                    Free Software Foundation, Inc., 59 Temple Place, Suite 330,
*                    Boston, MA  02111-1307  USA
*
*********************************************************************************************************
*/

#ifndef POSIXStorage_H
#define POSIXStorage_H

/*
*********************************************************************************************************
*                            Included header files to be exposed to the sketch
*********************************************************************************************************
*/

// These are necesssary to expose to the sketch to get the retargeting from mbed -->

#if defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_OPTA)
  #include <mbed.h>
#endif

#include <FATFileSystem.h>
#include <LittleFileSystem.h>

// <--

/*
*********************************************************************************************************
*                            Using declarations to be exposed to the sketch
*********************************************************************************************************
*/

// These are necesssary to expose to the sketch to get the retargeting from mbed -->

#if defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_OPTA)
  using mbed::FATFileSystem;
  using mbed::LittleFileSystem;
#endif

// <--

/*
*********************************************************************************************************
*                               Enumerations to be exposed to the sketch
*********************************************************************************************************
*/

enum DeviceNames : uint8_t
{
  DEV_SDCARD,
  DEV_USB
};

enum FileSystems : uint8_t
{
  FS_FAT,
  FS_LITTLEFS
};

enum MountFlags : uint8_t
{
  MNT_DEFAULT,
  MNT_RDONLY    // Read only
};

/*
*********************************************************************************************************
*                     Non-retargeted storage functions to be exposed to the sketch
*********************************************************************************************************
*/

/**
* @brief Attach a file system to a device.
* @param deviceName The device to attach to: DEV_SDCARD or DEV_USB.
* @param fileSystem The file system type to attach: FS_FAT or FS_LITTLEFS.
* @param mountFlags The only valid flag at this time: MNT_DEFAULT.
* @return On success: 0. On failure: -1 with an error code in the errno variable.
*/
int mount(const enum DeviceNames deviceName,
          const enum FileSystems fileSystem,
          const enum MountFlags mountFlags);

/**
* @brief Remove the attached file system from a device.
* @param deviceName The device to remove from: DEV_SDCARD or DEV_USB.
* @return On success: 0. On failure: -1 with an error code in the errno variable.
*/
int umount(const enum DeviceNames deviceName);

/**
* @brief Register a hotplug callback function. Currently only supported for DEV_USB on Portenta C33.
* @param deviceName The device to register for: DEV_SDCARD or DEV_USB.
* @param callbackFunction A function pointer to the callback.
* @return On success: 0. On failure: -1 with an error code in the errno variable.
*/
int register_hotplug_callback(const enum DeviceNames deviceName, void (* const callbackFunction)());

/**
* @brief Deregister a previously registered hotplug callback function. Not currently supported on any platform.
* @param deviceName The device to deregister for: DEV_SDCARD or DEV_USB.
* @return On success: 0. On failure: -1 with an error code in the errno variable.
*/
int deregister_hotplug_callback(const enum DeviceNames deviceName);

/**
* @brief Format a device (make file system).
* @param deviceName The device to format: DEV_SDCARD or DEV_USB.
* @param fileSystem The file system type to format: FS_FAT or FS_LITTLEFS. FS_FAT is probably the better choice for both SD Cards and USB thumb drives in most cases.
* @return On success: 0. On failure: -1 with an error code in the errno variable.
*/
int mkfs(const enum DeviceNames deviceName, const enum FileSystems fileSystem);

#endif  // POSIXStorage_H
