/*
 * This example shows how to mount an SD Card and write to and read from a file.
 * 
 * Original author: A. Vidstrom
 */

#include "Arduino_POSIXStorage.h"

void setup() {
  Serial.begin(9600);
  while (!Serial) ;

  FILE *fp;
  if (0 == mount(DEV_SDCARD, FS_FAT, MNT_DEFAULT))
  {
    fp = fopen("/sdcard/testfile.txt", "w");
    if (nullptr != fp)
    {
      if (fprintf(fp, "Hello, SD Card world") < 0)
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
    Serial.println("Error mounting SD Card");
    Serial.println(errno);
  }
  umount(DEV_SDCARD);
  if (0 == mount(DEV_SDCARD, FS_FAT, MNT_DEFAULT))
  {
    fp = fopen("/sdcard/testfile.txt", "r");
    if (nullptr != fp)
    {
      constexpr int bufferSize = 25;
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
    Serial.println("Error mounting SD Card");
    Serial.println(errno);
  }
  umount(DEV_SDCARD);
}

void loop() {
}
