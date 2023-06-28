/*
 * This example shows how to mount a USB thumb drive, without hotplug registration, and write to and read from a file
 * 
 * Original author: A. Vidstrom
 */

#include "POSIXStorage.h"

void setup() {
  Serial.begin(9600);
  while (!Serial) ;

  Serial.println("Please insert a thumb drive");
  while (0 != mount(DEV_USB, FS_FAT, MNT_DEFAULT)) {
    delay(500);
  }
  umount(DEV_USB);
  Serial.println("Thank you!");

  FILE *fp;
  if (0 == mount(DEV_USB, FS_FAT, MNT_DEFAULT))
  {
    fp = fopen("/usb/testfile.txt", "w");
    if (nullptr != fp)
    {
      if (fprintf(fp, "Hello, USB thumb drive world") < 0)
      {
        Serial.println("Error writing to file");
        Serial.println(errno);        
      }
    }
    else
    {
      Serial.println("Error opening file for writing");
      Serial.println(errno);
    }
    fclose(fp);
  }
  else
  {
    Serial.println("Error mounting USB thumb drive");
    Serial.println(errno);
  }
  umount(DEV_USB);
  if (0 == mount(DEV_USB, FS_FAT, MNT_DEFAULT))
  {
    fp = fopen("/usb/testfile.txt", "r");
    if (nullptr != fp)
    {
      constexpr int bufferSize = 30;
      char buffer[bufferSize];
      if (NULL != fgets(buffer, bufferSize, fp))
      {
        Serial.println(buffer);
      }
      else
      {
        Serial.println("Error reading from file");
        Serial.println(errno);
      }
    }
    else
    {
      Serial.println("Error opening file for reading");
      Serial.println(errno);
    }
    fclose(fp);
  }
  else
  {
    Serial.println("Error mounting USB thumb drive");
    Serial.println(errno);
  }
  umount(DEV_USB);
}

void loop() {
}
