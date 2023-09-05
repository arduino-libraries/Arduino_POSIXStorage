# Arduino_POSIXStorage Library

The Arduino_POSIXStorage Library complements the POSIX storage functions already included in the Renesas core and the Arduino_USBHostMbed5 library. It also makes them available to use in sketches.

The library supports and is tested on:
- Portenta C33 with Portenta Breakout Board (SD Card and USB Thumb Drive)
- Portenta C33 with Portenta Vision Shield  (SD Card)
- Portenta H7 with Portenta Breakout Board  (SD Card and USB Thumb Drive)
- Portenta H7 with Portenta Vision Shield   (SD Card)
- Portenta Machine Control                  (USB Thumb Drive)
- Opta                                      (USB Thumb Drive)


## Usage

For detailed information on usage, see the API and Examples sections below. This is a very basic example of how to include and use the library:

```cpp
#include "Arduino_POSIXStorage.h"

void setup() {
    mount(DEV_SDCARD, FS_FAT, MNT_DEFAULT);
//    ...
//    Use various POSIX storage functions.
//    ...
    umount(DEV_SDCARD);
}

void loop() {  
}
```

It is necessary to install the Arduino_USBHostMbed5 library to use Arduino_POSIXStorage on Portenta H7 and Portenta Machine Control. No additional library is needed on the Portenta C33.

The library automatically detects different types of Portenta H7 / Portenta Machine Control boards. This detection should work in the absolute majority of cases, but if you have trouble with USB on the Portenta Machine control you can try to add #define AUTOMATIC_OVERRIDE_PORTENTA_MACHINE_CONTROL just before #include "Arduino_POSIXStorage.h". The automatic detection should work even with custom boards, but if you have trouble with USB on a custom board, try adding #define AUTOMATIC_OVERRIDE_PORTENTA_H7 in a similar manner.

## API

The following POSIX functions are not a part of the library but are made available and work more or less according to the specification: close, closedir, fcntl, fsync, fstat, ftruncate, isatty, lseek, mkdir, open, opendir, poll, read, remove, rewinddir, seekdir, stat, statvfs, telldir, and write.

Many ISO C standard functions are also made available. For example: fopen, fprintf, and fclose.

Start pathnames on SD Cards with "/sdcard/" and pathnames on USB thumb drives with "/usb/". See the Examples section below for examples.

See [here](./api.md) for a complete description of the API.

## Examples

- **SD_Card_Example:** This example shows how to mount an SD Card and write to and read from a file.
- **USB_No_Hotplug_Example:** This example shows how to mount a USB thumb drive, without hotplug registration, and write to and read from a file.
- **USB_Hotplug_Example:** This example shows how to mount a USB thumb drive, with hotplug registration, and write to and read from a file.

## License

This library is released under the [LGPLv2.1 license](https://www.gnu.org/licenses/old-licenses/lgpl-2.1-standalone.html).
