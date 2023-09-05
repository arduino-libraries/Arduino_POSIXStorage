/*
*********************************************************************************************************
*                                      Arduino_POSIXStorage Library
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
*                    - Opta                                      (USB Thumb Drive)
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

/*
*********************************************************************************************************
*                                         Included header files
*********************************************************************************************************
*/

#include "Arduino_POSIXStorage.h"

#include <Arduino.h>

#if defined(ARDUINO_PORTENTA_C33)
  #include <SDCardBlockDevice.h>
  #include <UsbHostMsd.h>
#elif defined(ARDUINO_PORTENTA_H7_M7) 
  #include <Arduino_USBHostMbed5.h>
  #include <BlockDevice.h>
  #include <DigitalIn.h>
  #include <SDMMCBlockDevice.h>
#elif defined(ARDUINO_OPTA) 
  #include <Arduino_USBHostMbed5.h>
  #include <BlockDevice.h>
#else
  #error "The POSIXStorage library does not support this board"
#endif



/*
*********************************************************************************************************
*                                  Library-internal using declarations
*********************************************************************************************************
*/

#if defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_OPTA)
  using mbed::BlockDevice;
  using mbed::FileSystem;
#endif

/*
*********************************************************************************************************
*                                   Library-internal data structures
*********************************************************************************************************
*/

struct DeviceFileSystemCombination {
  BlockDevice *device    = nullptr;     // Set if mounted or hotplug callback registered
  FileSystem *fileSystem = nullptr;     // Set only if mounted
};

/*
*********************************************************************************************************
*                                    Library-internal enumerations
*********************************************************************************************************
*/

enum ActionTypes : uint8_t
{
  ACTION_MOUNT,
  ACTION_FORMAT
};

enum BoardTypes : uint8_t
{
  BOARD_UNKNOWN,
  BOARD_MACHINE_CONTROL,
  BOARD_PORTENTA_H7,
  BOARD_PORTENTA_C33
};

/*
*********************************************************************************************************
*                       Unnamed namespace for library-internal global variables
*********************************************************************************************************
*/

namespace {

// Always set unused members to nullptr at all times because the rest of the code expects it -->
struct DeviceFileSystemCombination sdcard = {nullptr, nullptr};
struct DeviceFileSystemCombination usb    = {nullptr, nullptr};
// <--

bool hotplugCallbackAlreadyRegistered = false;

// Used to handle special case (powering USB A female socket separately) for Machine Control -->
bool hasMountedBefore = false;
bool runningOnMachineControl = false;
// <--

}   // End of unnamed namespace

/*
*********************************************************************************************************
*                            Unnamed namespace for library-internal functions
*********************************************************************************************************
*/

namespace {

// This detection code only works on the Portenta H7 boards -->
#if defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_OPTA)

// WARNING: This algorithm has been tested for radiated interference immunity with a
// chattering relay device, so please don't modify it without running such a test again!
enum BoardTypes detectPortentaH7TypeOnce()
{
  int inAfterUp, inAfterDown;
  mbed::DigitalIn vbusPin(PB_14);
  // Inject high level into input, wait 1 ms to settle
  vbusPin.mode(PullUp);
  delay(1);
  // Shortly float the input and read the state
  vbusPin.mode(PullNone);           
  inAfterUp = vbusPin.read();
  // Inject low level into input, wait 1 ms to settle
  vbusPin.mode(PullDown);
  delay(1);
  // Shortly float the input
  vbusPin.mode(PullNone);
  // Give the external pull-up resistor on the Portenta Machine control enough time to
  // raise the voltage to an acceptable high
  delayMicroseconds(50);
  // Read the state
  inAfterDown = vbusPin.read();
  // End floating state and leave the input in pull-up state
  vbusPin.mode(PullUp);
  // The Machine Control has an external pull-up resistor
  // and will never float, the input is always 1.
  // A plain H7 board floats, and the input follows the
  // previously injected level, first 1 and then 0.
  // H7 plus Vision Shield or Breakout Board count as plain
  // boards too.
  if ((1 == inAfterUp) && (0 == inAfterDown))
  {
    return BOARD_PORTENTA_H7;
  }
  if ((1 == inAfterUp) && (1 == inAfterDown))
  {
    return BOARD_MACHINE_CONTROL;
  }
  else
  {
    return BOARD_UNKNOWN;   // Illegal state, detection failed
  }
}   // End of detectPortentaH7TypeOnce()

// WARNING: This algorithm has been tested for radiated interference immunity with a
// chattering relay device, so please don't modify it without running such a test again!
enum BoardTypes detectPortentaH7Type()
{
  enum BoardTypes boardType = detectPortentaH7TypeOnce();
  // Repeat the test a few times to mitigate incorrect
  // results because of electrical interference
  for (int i=0; i<5; i++)
  {
    if (boardType != detectPortentaH7TypeOnce())
    {
      return BOARD_UNKNOWN;
    }
  }
  return boardType;
}   // End of detectPortentaH7Type()

#endif
// <-- This detection code only works on the Portenta H7 boards

// Automatic detection unless AUTOMATIC_OVERRIDE_PORTENTA_H7 or AUTOMATIC_OVERRIDE_PORTENTA_MACHINE_CONTROL
// is defined
enum BoardTypes detectBoardType()
{
#if defined(ARDUINO_PORTENTA_C33)
  return BOARD_PORTENTA_C33;
#elif defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_OPTA)
  #if (defined(AUTOMATIC_OVERRIDE_PORTENTA_H7) && defined(AUTOMATIC_OVERRIDE_PORTENTA_MACHINE_CONTROL))
    #error "You have defined AUTOMATIC_OVERRIDE_PORTENTA_H7 and AUTOMATIC_OVERRIDE_PORTENTA_MACHINE_CONTROL at the same time"
  #endif
  #if defined(AUTOMATIC_OVERRIDE_PORTENTA_H7)
    return BOARD_PORTENTA_H7;
  #elif defined(AUTOMATIC_OVERRIDE_PORTENTA_MACHINE_CONTROL)
    return BOARD_MACHINE_CONTROL;
  #else 
    return detectPortentaH7Type();    // Automatic detection
  #endif
#else
  #error "This board is not supported"
#endif
}   // End of detectBoardType()

void deleteDevice(const enum StorageDevices deviceName, struct DeviceFileSystemCombination * const deviceFileSystemCombination)
{
  // The USBHostMSD class for the H7 doesn't correctly support object destruction, so we only delete
  // the device object on other platforms or if the device is an SD Card -->
  bool deleteDevice = false;
#if (!(defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_OPTA)))
  (void) deviceName;    // Silence -Wunused-parameter, because this variable is only used on the H7
  deleteDevice = true;
#else
  if (DEV_SDCARD == deviceName)
  {
    deleteDevice = true;
  }
#endif
  if (true == deleteDevice)
  {
    // Ok to delete with base class pointer because the destructor of the base class is virtual
    delete deviceFileSystemCombination->device;
    deviceFileSystemCombination->device = nullptr;
  }
}   // End of deleteDevice()

// WARNING: Don't set errno and return -1 in this function - just return 0 for success or the errno code!
int mountOrFormatFileSystemOnDevice(const enum StorageDevices deviceName,
                                    struct DeviceFileSystemCombination * const deviceFileSystemCombination,
                                    const enum FileSystems fileSystem,
                                    const char * const mountPoint,
                                    const enum ActionTypes mountOrFormat)
{
  if ((nullptr == mountPoint) || (nullptr == deviceFileSystemCombination))
  {
    return EFAULT;
  }
  if (FS_FAT == fileSystem)
  {
    deviceFileSystemCombination->fileSystem = new(std::nothrow) FATFileSystem(mountPoint);
  }
  else if (FS_LITTLEFS == fileSystem)
  {
    deviceFileSystemCombination->fileSystem = new(std::nothrow) LittleFileSystem(mountPoint);
  }
  else
  {
    return ENODEV;    // This shouldn't happen unless there is a bug in the code
  }
  if (nullptr == (deviceFileSystemCombination->fileSystem))
  {
    return ENODEV;
  }
  // Check before use in mount(), umount(), or reformat() calls below
  if (nullptr == deviceFileSystemCombination->device)
  {
    // Ok to delete with base class pointer because the destructor of the base class is virtual
    delete deviceFileSystemCombination->fileSystem;
    deviceFileSystemCombination->fileSystem = nullptr;
    return EFAULT;
  }
  if (ACTION_MOUNT == mountOrFormat)
  {
    // See note (1) at the bottom of the file
    int mountReturn = deviceFileSystemCombination->fileSystem->mount(deviceFileSystemCombination->device);
    if (0 != mountReturn)
    {
      // Ok to delete with base class pointer because the destructor of the base class is virtual
      delete deviceFileSystemCombination->fileSystem;
      deviceFileSystemCombination->fileSystem = nullptr;
      // mbed's mount() returns negative errno codes
      return (-mountReturn);    // See note (1) at the bottom of the file
    }
    return 0;
  }   // End of ACTION_MOUNT
  else if (ACTION_FORMAT == mountOrFormat)
  {
    int reformatReturn = -1;    // See note (1) at the bottom of the file
    if (FS_FAT == fileSystem)
    {
      // Ok to downcast with static_cast because we know for sure that fileSystem isn't pointing to a
      // base-class object, and dynamic_cast wouldn't work anyway because compilation is done with -fno-rtti      
      FATFileSystem *fatFileSystem = static_cast<FATFileSystem*>(deviceFileSystemCombination->fileSystem);
      // FS_FAT needs an allocation unit size specified and 0 just asks for the default one
      reformatReturn = fatFileSystem->reformat(deviceFileSystemCombination->device, 0);
    }
    else if (FS_LITTLEFS == fileSystem)
    {
      reformatReturn = deviceFileSystemCombination->fileSystem->reformat(deviceFileSystemCombination->device);
    }
    else  // This shouldn't happen unless there is a bug in the code
    {
      // Ok to delete with base class pointer because the destructor of the base class is virtual
      delete deviceFileSystemCombination->fileSystem;
      deviceFileSystemCombination->fileSystem = nullptr;
      return ENODEV;
    }
    if (0 != reformatReturn)
    {
      // Ok to delete with base class pointer because the destructor of the base class is virtual
      delete deviceFileSystemCombination->fileSystem;
      deviceFileSystemCombination->fileSystem = nullptr;
      // mbed's reformat() returns negative errno codes
      return (-reformatReturn);   // See note (1) at the bottom of the file
    }
    if (0 == deviceFileSystemCombination->fileSystem->unmount())
    {
      // Ok to delete with base class pointer because the destructor of the base class is virtual
      delete deviceFileSystemCombination->fileSystem;
      deviceFileSystemCombination->fileSystem = nullptr;
      deleteDevice(deviceName, deviceFileSystemCombination);
    }
    else
    {
      // If unmount() fails, we have left the filesystem mounted, but there's no good way to signal that
      // to the user with errno codes, so we fail this step silently in those (presumably) very rare cases
    }
    return 0;
  }   // End of ACTION_FORMAT
  else
  {
    // Ok to delete with base class pointer because the destructor of the base class is virtual
    delete deviceFileSystemCombination->fileSystem;
    deviceFileSystemCombination->fileSystem = nullptr;
    return ENOTSUP;    // This shouldn't happen unless there's a bug in the code
  }
}   // End of mountOrFormatFileSystemOnDevice()

// WARNING: Don't set errno and return -1 in this function - just return 0 for success or the errno code!
int mountOrFormatSDCard(const enum FileSystems fileSystem,
                        const enum ActionTypes mountOrFormat)
{
  if (nullptr != sdcard.device)   // An SD card is already mounted at that mount point
  {
    return EBUSY;
  }

  // The Machine Control doesn't have an SD Card connector
#if defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_OPTA)
  if (true == runningOnMachineControl)
  {
    return ENOTBLK;
  }
#endif

  // Create the appropriate BlockDevice -->
#if defined(ARDUINO_PORTENTA_C33)
  sdcard.device = new(std::nothrow) SDCardBlockDevice(PIN_SDHI_CLK,
                                                       PIN_SDHI_CMD,
                                                       PIN_SDHI_D0,
                                                       PIN_SDHI_D1,
                                                       PIN_SDHI_D2,
                                                       PIN_SDHI_D3,
                                                       PIN_SDHI_CD,
                                                       PIN_SDHI_WP);
#elif defined(ARDUINO_PORTENTA_H7_M7) || !defined(ARDUINO_OPTA)
  sdcard.device = new(std::nothrow) SDMMCBlockDevice();
#else
  sdcard.device = nullptr;
#endif
  if (nullptr == sdcard.device)
  {
    return ENOTBLK;
  }
  // <--

  const int mountOrFormatReturn = mountOrFormatFileSystemOnDevice(DEV_SDCARD, &sdcard, fileSystem, "sdcard", mountOrFormat);
  if (0 != mountOrFormatReturn)
  {
    delete sdcard.device;
    sdcard.device = nullptr;
    return mountOrFormatReturn;
  }
  return 0;
}   // End of mountOrFormatSDCard()

// WARNING: Don't set errno and return -1 in this function - just return 0 for success or the errno code!
int mountOrFormatUSBDevice(const enum FileSystems fileSystem,
                           const enum ActionTypes mountOrFormat)
{
#if defined(ARDUINO_PORTENTA_H7_M7) 
  if (true == runningOnMachineControl)
  {
    // We need to apply power manually to the female USB A connector on the Machine Control
    mbed::DigitalOut enablePower(PB_14, 0);
  }
#endif

  // We'll need a USBHostMSD pointer because connect() and connected() we'll use later aren't member
  // functions of the base class BlockDevice
  USBHostMSD *usbHostDevice = nullptr;

  // If the device is registered for the hotplug event or on H7, we mustn't delete it even if
  // we fail to mount a filesystem to it
  bool hotplugKeep = false;

  if (nullptr != usb.device) // Already mounted or registered for the hotplug event
  {
    if (nullptr != usb.fileSystem)  // The device is already mounted
    {
      return EBUSY;
    }
    else // The device is registered for the hotplug event, or left intact on H7, so we can reuse it
    {
      // Ok to downcast with static_cast because we know for sure that usb.device isn't pointing to a
      // base-class object, and dynamic_cast wouldn't work anyway because compilation is done with -fno-rtti
      usbHostDevice = static_cast<USBHostMSD*>(usb.device);
      // Make sure not to remove the device object if mount() fails
      hotplugKeep = true;
    }
  }
  else {  // The device isn't used at all yet
    usbHostDevice = new(std::nothrow) USBHostMSD;
    if (nullptr == usbHostDevice)
    {
      return ENOTBLK;
    }
    usb.device = usbHostDevice;
  }
  if (false == (usbHostDevice->connected()))
  {
    if (false == (usbHostDevice->connect()))
    {
      // Only delete if the object was created by this function
      if (false == hotplugKeep)
      {
        deleteDevice(DEV_USB, &usb);
      }
      return ENOTBLK;
    }
  }
  const int mountOrFormatReturn = mountOrFormatFileSystemOnDevice(DEV_USB, &usb, fileSystem, "usb", mountOrFormat);
  if (0 != mountOrFormatReturn)
  {
    // Only delete if the object was created by this function
    if (false == hotplugKeep)
    {
      deleteDevice(DEV_USB, &usb);
    }
    return mountOrFormatReturn;
  }
  return 0;
}   // End of mountOrFormatUSB()

// WARNING: Don't set errno and return -1 in this function - just return 0 for success or the errno code!
int mountOrFormat(const enum StorageDevices deviceName,
                  const enum FileSystems fileSystem,
                  const enum ActionTypes mountOrFormat)
{
  // Determine if we're running on Machine Control or not on the first call to mount() or mkfs() -->
  if (false == hasMountedBefore)
  {
    hasMountedBefore = true;
    if (BOARD_MACHINE_CONTROL == detectBoardType())
    {
      runningOnMachineControl = true;
    }
  }
  // <--
  switch (deviceName)
  {
    case DEV_SDCARD:
      return mountOrFormatSDCard(fileSystem, mountOrFormat);
    case DEV_USB:
      return mountOrFormatUSBDevice(fileSystem, mountOrFormat);
    default:
      return ENOTBLK;
  }
}   // End of mountOrFormat()

}   // End of unnamed namespace

/*
*********************************************************************************************************
*                                  Non-retargeted storage API functions
*********************************************************************************************************
*/

int mount(const enum StorageDevices deviceName,
          const enum FileSystems fileSystem,
          const enum MountFlags mountFlags)
{
  // Only MNT_DEFAULT is allowed on this platform, but other platforms could also allow MNT_RDONLY
  if (MNT_DEFAULT != mountFlags)
  {
    errno = ENOTSUP;
    return -1;
  }
  const int mountOrFormatReturn = mountOrFormat(deviceName, fileSystem, ACTION_MOUNT);
  if (0 != mountOrFormatReturn)
  {
    errno = mountOrFormatReturn;
    return -1;
  }
  return 0;
}   // End of mount()

int mkfs(const enum StorageDevices deviceName, const enum FileSystems fileSystem)
{
  const int mountOrFormatReturn = mountOrFormat(deviceName, fileSystem, ACTION_FORMAT);
  if (0 != mountOrFormatReturn)
  {
    errno = mountOrFormatReturn;
    return -1;
  }
  return 0;
}   // End of mkfs()

int umount(const enum StorageDevices deviceName)
{
  struct DeviceFileSystemCombination *deviceFileSystemCombination = nullptr;

  switch (deviceName)
  {  
    case DEV_SDCARD:
      deviceFileSystemCombination = &sdcard;
      break;
    case DEV_USB:
      deviceFileSystemCombination = &usb;
      break;
    default:
      errno = EINVAL; // This shouldn't happen unless there's a bug in the code
      return -1;
  }
  // Error if the device isn't mounted
  if ((nullptr == deviceFileSystemCombination->device) || (nullptr == deviceFileSystemCombination->fileSystem))
  {
    errno = EINVAL;
    return -1;
  }
  // See note (1) at the bottom of the file
  int unmountRet = deviceFileSystemCombination->fileSystem->unmount();
  if (0 == unmountRet)
  {
    // Ok to delete with base class pointer because the destructor of the base class is virtual
    delete deviceFileSystemCombination->fileSystem;
    deviceFileSystemCombination->fileSystem = nullptr;
    deleteDevice(deviceName, deviceFileSystemCombination);
    return 0;
  }
  else
  {
    // mbed's mount() returns negative errno codes
    errno = -unmountRet;   // See note (1) at the bottom of the file
    return -1;
  }
}   // End of umount()

int register_hotplug_callback(const enum StorageDevices deviceName, void (* const callbackFunction)())
{
  if (true == hotplugCallbackAlreadyRegistered)
  {
    errno = EBUSY;
    return -1;
  }
  if (nullptr == callbackFunction)
  {
    errno = EFAULT;
    return -1;
  }
  switch (deviceName)
  {
    case DEV_SDCARD:    // There is no support for callbacks in the any of the SD Card block device classes
      errno = ENOTSUP;
      return -1;
    case DEV_USB:
      { // Curly braces necessary to keep new variables inside the case statement

      // A USB mass storage device is already mounted at that mount point, or
      // registered for the hotplug event
      if (nullptr != usb.device)
      {
        errno = EBUSY;
        return -1;
      }
#if defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_OPTA)
      // There is no support for callbacks in the USBHostMSD class on this platform
      errno = ENOTSUP;
      return -1;
#else      
      // We must create a USBHostMSD object to attach the callback to, but we
      // don't create a file system object because we don't fully mount() anything
      USBHostMSD *usbHostDevice = nullptr;
      usbHostDevice = new(std::nothrow) USBHostMSD;
      if (nullptr == usbHostDevice)
      {
        errno = ENOTBLK;
        return -1;
      }
      if (false == (usbHostDevice->attach_detected_callback(callbackFunction)))
      {
        delete usbHostDevice;   // Ok to delete, because this isn't on the H7
        usbHostDevice = nullptr;
        errno = EINVAL;
        return -1;
      }
      // This is necessary for future calls to mount(), umount(), and mkfs()
      usb.device = usbHostDevice;
      // Prevent multiple registrations
      hotplugCallbackAlreadyRegistered = true;
      return 0;
#endif
      } // Curly braces necessary to keep new variables inside the case statement
    default:
      errno = ENOTBLK;
      return -1;
  }        
}   // End of hotplug_register_callback()

// Not supported by the layer below on these platforms, but might be on other platforms
int deregister_hotplug_callback(const enum StorageDevices deviceName)
{
  (void) deviceName;    // Remove when implemented, only here to silence -Wunused-parameter
  errno = ENOSYS;
  return -1;
}   // End of hotplug_deregister_callback()

/*
*********************************************************************************************************
*                                                Notes
*********************************************************************************************************
*/

//
// Note (1):
//
// The mbed POSIX implementation has a potential problem, because it uses the fat_error_remap()
// function to remap error codes from ChaN's FatFs to negative errno numbers. It does this with
// a long switch statement that ends with the default "return -res;" where res is the return code
// from FatFs. The integer ranges used by these two types of error codes overlap, and should
// FatFs add a new error code, it will be turned into an erroneous errno. At the time of writing,
// fat_error_remap() remaps every possible FatFs code. But if FatFs adds another code, it will
// become number 20, which will be automatically remapped to -20, which incorrectly corresponds to
// -ENOTDIR. And so on. This potentially affects several of the POSIX functions in mbed. By the time
// we get the code, it will be impossible to tell correct and incorrect apart. The return codes
// from LittleFS are, in general, negative errno numbers. New ones are just passed along as-is,
// so the same potential problem doesn't exist for LittleFS.
//
