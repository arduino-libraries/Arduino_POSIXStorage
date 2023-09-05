/*
 *
 * POSIXStorage Tests
 *
 * Original Author: A. Vidstrom (Arduino SA. http://arduino.cc)
 *
 * This code is in the public domain
 *
 */

#include "Arduino_POSIXStorage.h"

enum TestTypes : uint8_t
{
  TEST_PORTENTA_C33_SDCARD,
  TEST_PORTENTA_C33_USB,
  TEST_PORTENTA_H7_SDCARD,
  TEST_PORTENTA_H7_USB,
  TEST_PORTENTA_MACHINE_CONTROL_SDCARD,
  TEST_PORTENTA_MACHINE_CONTROL_USB,
  TEST_OPTA_SDCARD,
  TEST_OPTA_USB
};

// !!! TEST CONFIGURATION !!! -->

constexpr enum TestTypes selectedTest = TEST_PORTENTA_C33_SDCARD;

// Notice that formtting tests can take a while to complete

// Uncomment the line below to include testing of LITTLEFS and FAT formatting with mkfs():

//#define PERFORM_FORMATTING_TESTS

// <-- !!! TEST CONFIGURATION !!!

volatile bool usbAttached = false;

void usbCallback()
{
  usbAttached = true;
}

void setup() {
  bool allTestsOk = true;
  enum StorageDevices deviceName;
  int fileDescriptor = 0;
  int retVal = -1;

  if ((TEST_PORTENTA_C33_USB == selectedTest) || (TEST_PORTENTA_H7_USB == selectedTest) || (TEST_PORTENTA_MACHINE_CONTROL_USB == selectedTest) || (TEST_OPTA_USB == selectedTest))
  {
    deviceName = DEV_USB;
  }
  else if ((TEST_PORTENTA_C33_SDCARD == selectedTest) || (TEST_PORTENTA_H7_SDCARD == selectedTest) || (TEST_PORTENTA_MACHINE_CONTROL_SDCARD == selectedTest) || (TEST_OPTA_SDCARD == selectedTest))
  {
    deviceName = DEV_SDCARD;
  }
  else
  {
    for ( ; ; ) ;   // Shouldn't get here unless there's a bug in the test code
  }

  Serial.begin(9600);
  // We can't have the Serial Monitor connected when we thest USB on the Opta, and this will cause
  // the test to freeze unless we skip it
  if (TEST_OPTA_USB != selectedTest)
  {
    while (!Serial) ; // Wait for the serial port to be ready
  }

  Serial.println("Testing started, please wait...");
  Serial.println();

  if ((TEST_PORTENTA_MACHINE_CONTROL_SDCARD == selectedTest) || (TEST_OPTA_SDCARD == selectedTest))
  {
    // Machine Control and Opta no SD Card supported test -->
    retVal = mount(DEV_SDCARD, FS_FAT, MNT_DEFAULT);
    if ((-1 != retVal) || (ENOTBLK != errno))
    {
      Serial.println("[FAIL] Machine Control and Opta no SD Card supported test failed");
    }
    else
    {
      Serial.println("Testing complete.");
      Serial.println();
      Serial.println("SUCCESS: Finished without errors");
      (void) umount(DEV_SDCARD);
      for ( ; ; ) ;   // Stop testing here
    }
    // <-- Machine Control and Opta no SD Card supported test
  }

  // Register hotplug callback for SD Card test -->
  if (DEV_SDCARD == deviceName)
  {
    // Using usbCallback() is fine because it doesn't get registered anyway
    retVal = register_hotplug_callback(DEV_SDCARD, usbCallback);
    if ((-1 != retVal) || (ENOTSUP != errno))
    {
      allTestsOk = false;
      Serial.print("[FAIL] Register hotplug callback for SD Card test failed");
      Serial.println();
    }
  }
  // <-- Register hotplug callback for SD Card test

  if (TEST_PORTENTA_C33_USB == selectedTest)
  {
    // Register nullptr callback test -->
    retVal = register_hotplug_callback(DEV_USB, nullptr);
    if ((-1 != retVal) || (EFAULT != errno))
    {
      allTestsOk = false;
      Serial.print("[FAIL] Register nullptr callback test failed");
      Serial.println();
    }
    // <-- Register nullptr callback test
  }

  if ((TEST_PORTENTA_H7_USB == selectedTest) || (TEST_PORTENTA_MACHINE_CONTROL_USB == selectedTest) || (TEST_OPTA_USB == selectedTest))
  {
    // Register unsupported callback test -->
    retVal = register_hotplug_callback(DEV_USB, usbCallback);
    if ((-1 != retVal) || (ENOTSUP != errno))
    {
      allTestsOk = false;
      Serial.println("[FAIL] Register unsupported callback test");
      Serial.println();
    }
    // <-- Register unsupported callback test
  }

  // This isn't a test, just wait for a USB thumb drive -->
  if (DEV_USB == deviceName)
  {
    Serial.println("Please insert a thumb drive");
    if (TEST_PORTENTA_C33_USB == selectedTest)
    {
      // This board supports hotplug callbacks
      (void) register_hotplug_callback(DEV_USB, usbCallback);
      while (false == usbAttached) {
        delay(500);
      }
    }
    else if ((TEST_PORTENTA_H7_USB == selectedTest) || (TEST_PORTENTA_MACHINE_CONTROL_USB == selectedTest) || (TEST_OPTA_USB == selectedTest))
    {
      // These boards don't support hotplug callbacks, so loop on mount() tries
      while (0 != mount(DEV_USB, FS_FAT, MNT_DEFAULT)) {
        delay(500);
      }
      (void) umount(DEV_USB);
    }
    else
    {
      for ( ; ;) ;  // Shouldn't get here unless there's a bug in the test code
    }
    Serial.println("Thank you!");
    Serial.println();
  }
  // <-- This isn't a test, just wait for a USB thumb drive
  
#if defined(PERFORM_FORMATTING_TESTS)
  Serial.println("The formatting tests you selected can take a while to complete");
  Serial.println();

  // LITTLEFS formatting test -->
  if (0 != mkfs(deviceName, FS_LITTLEFS))
  {
    allTestsOk = false;
    Serial.println("[FAIL] mkfs() with LITTLEFS failed");
  }
  if (0 != mount(deviceName, FS_LITTLEFS, MNT_DEFAULT))
  {
    allTestsOk = false;
    Serial.println("[FAIL] mount() after mkfs() with LITTLEFS failed");
  }
  if (DEV_USB == deviceName)
  {
    fileDescriptor = open("/usb/5395748341.txt", O_CREAT);
  }
  else if (DEV_SDCARD == deviceName)
  {
    fileDescriptor = open("/sdcard/5395748341.txt", O_CREAT);
  }
  else
  {
    for ( ; ;) ;  // Shouldn't get here unless there's a bug in the test code
  }
  if (fileDescriptor < 3)   // 0-2 are reserved
  {
    allTestsOk = false;
    Serial.println("[FAIL] open() after mkfs() with LITTLEFS failed");
  }
  if (0 != close(fileDescriptor))
  {
    allTestsOk = false;
    Serial.println("[FAIL] close() after mkfs() with LITTLEFS failed");
  }
  if (0 != umount(deviceName))
  {
    allTestsOk = false;
    Serial.println("[FAIL] umount() after mkfs() with LITTLEFS failed");
  }
  // <-- LITTLEFS formatting test

  // FAT formatting test -->
  if (0 != mkfs(deviceName, FS_FAT))
  {
    allTestsOk = false;
    Serial.println("[FAIL] mkfs() with FAT failed");
  }
  if (0 != mount(deviceName, FS_FAT, MNT_DEFAULT))
  {
    allTestsOk = false;
    Serial.println("[FAIL] mount() after mkfs() with FAT failed");
  }
  if (DEV_USB == deviceName)
  {
    fileDescriptor = open("/usb/5395748341.txt", O_CREAT);
  }
  else if (DEV_SDCARD == deviceName)
  {
    fileDescriptor = open("/sdcard/5395748341.txt", O_CREAT);
  }
  else
  {
    for ( ; ;) ;  // Shouldn't get here unless there's a bug in the test code
  }
  if (fileDescriptor < 3)   // 0-2 are reserved
  {
    allTestsOk = false;
    Serial.println("[FAIL] open() after mkfs() with FAT failed");
  }
  if (0 != close(fileDescriptor))
  {
    allTestsOk = false;
    Serial.println("[FAIL] close() after mkfs() with FAT failed");
  }
  if (0 != umount(deviceName))
  {
    allTestsOk = false;
    Serial.println("[FAIL] umount() after mkfs() with FAT failed");
  }  
  // <-- FAT formatting test
#else
  Serial.println("[WARN] Formatting tests were skipped");
#endif

  // Repeated mount() and umount() test -->
  bool repeatTestFailed = false;
  int ioErrorCount = 0;
  for (int i=0; i<100; i++)
  {
    retVal = mount(deviceName, FS_FAT, MNT_DEFAULT);
    if (0 != retVal)
    {
      if (EIO == errno)
      {
        ioErrorCount++;
      }
      repeatTestFailed = true;
    }
    if (DEV_USB == deviceName)
    {
      (void) remove("/usb/5395748341.txt");
      fileDescriptor = open("/usb/5395748341.txt", O_CREAT);
    }
    else if (DEV_SDCARD == deviceName)
    {
      (void) remove("/sdcard/5395748341.txt");
      fileDescriptor = open("/sdcard/5395748341.txt", O_CREAT);
    }
    else
    {
      for ( ; ;) ;  // Shouldn't get here unless there's a bug in the test code
    } 
    if (fileDescriptor < 3)   // 0-2 are reserved
    {
      repeatTestFailed = true;
    }
    else
    {
      (void) close(fileDescriptor);
    }
    retVal = umount(deviceName);
    if (0 != retVal)
    {
      repeatTestFailed = true;
    }
  }
  if (true == repeatTestFailed)
  {
    allTestsOk = false;
    Serial.println("[FAIL] Repeated mount() and umount() test failed");
    if (ioErrorCount > 0)
    {
      Serial.print("       Cause: ");
      Serial.print(ioErrorCount);
      Serial.println(" low-level I/O errors of unknown origin");
    }
  }
  // <-- Repeated mount() and umount() test

  // Mount read only not supported test -->
  retVal = mount(deviceName, FS_FAT, MNT_RDONLY);
  if ((-1 != retVal) || (ENOTSUP != errno))
  {
    allTestsOk = false;
    Serial.println("[FAIL] Mount read only not supported test failed");
  }
  // <-- Mount read only not supported test

  // umount() when not mounted test -->
  retVal = umount(deviceName);
  if ((-1 != retVal) || (EINVAL != errno))
  {
    allTestsOk = false;
    Serial.println("[FAIL] umount() when not mounted test failed");
  }
  // <-- umount() when not mounted test

  // mount() when already mounted test -->
  retVal = mount(deviceName, FS_FAT, MNT_DEFAULT);
  if (0 != retVal)  
  {
    allTestsOk = false;
    Serial.println("[FAIL] (first) mount() when already mounted test failed");
  }
  retVal = mount(deviceName, FS_FAT, MNT_DEFAULT);
  if ((-1 != retVal) || (EBUSY != errno))
  {
    allTestsOk = false;
    Serial.println("[FAIL] (second) mount() when already mounted test failed");
  }
  (void) umount(deviceName);
  // <-- mount() when already mounted test

  if (TEST_PORTENTA_C33_USB == selectedTest)
  {
    // Register multiple callbacks test -->
    retVal = register_hotplug_callback(DEV_USB, usbCallback);
    if ((-1 != retVal) || (EBUSY != errno))
    {
      allTestsOk = false;
      Serial.println("[FAIL] Register multiple callbacks test failed");
    }
    // <-- Register multiple callbacks test
  }

  // Deregister callback not supported test -->
  retVal = deregister_hotplug_callback(DEV_USB);
  if ((-1 != retVal) || (ENOSYS != errno))
  {
    allTestsOk = false;
    Serial.println("[FAIL] Deregister callback not supported test failed");
  }
  // <-- Deregister callback not supported test

  // Remove before persistent storage test -->
  (void) mount(deviceName, FS_FAT, MNT_DEFAULT);
  if (DEV_USB == deviceName)
  {
    retVal = remove("/usb/5395748341.txt");
  }
  else if (DEV_SDCARD == deviceName)
  {
    retVal = remove("/sdcard/5395748341.txt");
  }
  else
  {
    for ( ; ;) ;  // Shouldn't get here unless there's a bug in the test code
  }
  if (0 != retVal)
  {
    if (ENOENT != errno)
    {
      allTestsOk = false;
      Serial.println("[FAIL] Remove before persistent storage test failed");
    }
  }
  (void) umount(deviceName);
  // <-- Remove before persistent storage test

  // Persistent storage test -->
  FILE *fp;
  (void) mount(deviceName, FS_FAT, MNT_DEFAULT);
  if (DEV_USB == deviceName)
  {
    fp = fopen("/usb/5395748341.txt", "w");
  }
  else if (DEV_SDCARD == deviceName)
  {
    fp = fopen("/sdcard/5395748341.txt", "w");
  }
  else
  {
    for ( ; ;) ;  // Shouldn't get here unless there's a bug in the test code
  }
  if (nullptr != fp)
  {
    (void) fprintf(fp, "Test string");
    (void) fclose(fp);
  }
  else
  {
    allTestsOk = false;
    Serial.println("[FAIL] Persistent storage test failed on fopen() call");
  }
  if (0 != umount(deviceName))
  {
    allTestsOk = false;
    Serial.println("[FAIL] Persistent storage test failed on umount() call");
  }
  (void) mount(deviceName, FS_FAT, MNT_DEFAULT);
  struct stat sb;
  if (DEV_USB == deviceName)
  {
    retVal = stat("/usb/5395748341.txt", &sb);
    (void) remove("/usb/5395748341.txt");
  }
  else if (DEV_SDCARD == deviceName)
  {
    retVal = stat("/sdcard/5395748341.txt", &sb);
    (void) remove("/sdcard/5395748341.txt");
  }
  else
  {
    for ( ; ;) ;  // Shouldn't get here unless there's a bug in the test code
  }
  if (0 != retVal)
  {
    allTestsOk = false;
    Serial.println("[FAIL] Persistent storage test failed on stat() call");
  }
  // Signed to unsigned conversion -->
  size_t fileSize = 0;
  if (sb.st_size >= 0)
  {
    fileSize = static_cast<size_t>(sb.st_size);
  }
  else
  {
    fileSize = 0;
  }
  // <-- Signed to unsigned conversion
  if (fileSize != strlen("Test string"))
  {
    allTestsOk = false;
    Serial.println("[FAIL] Persistent storage test failed on size test");
  }
  (void) umount(deviceName);
  // <-- Persistent storage test

  // Final report -->
  Serial.println();
  Serial.println("Testing complete.");
  Serial.println();
  if (true == allTestsOk)
  {
    Serial.println("SUCCESS: Finished without errors");
  }
  else
  {
    Serial.println("FAILURE: Finished with errors (see list above for details)");
  }
  // <-- Final report

  // Opta final report -->
  if (TEST_OPTA_USB == selectedTest)
  {
    (void) mount(deviceName, FS_FAT, MNT_DEFAULT);
    FILE *logFile = fopen("/usb/testlog.txt", "w");
    if (true == allTestsOk)
    {
      fprintf(logFile, "SUCCESS: Finished without errors");
      fclose(logFile);      
    }
    else
    {
      fprintf(logFile, "FAILURE: Finished with errors");
      fclose(logFile);      
    }
    (void) umount(deviceName);
  }
  // <--
}

void loop() {
  // Empty
}
