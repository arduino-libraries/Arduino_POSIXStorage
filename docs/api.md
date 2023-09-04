# Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`enum ` [`StorageDevices`](#_arduino___p_o_s_i_x_storage_8h_1a97a26676f4f644e3db23bb63b9227546)            | Enum to select the storage device to use.
`enum ` [`FileSystems`](#_arduino___p_o_s_i_x_storage_8h_1ac01996562b852a6b36ad87908429ad35)            | Enum to select the file system to use.
`enum ` [`MountFlags`](#_arduino___p_o_s_i_x_storage_8h_1a069889b849809b552adf0513c6db2b85)            | Enum to select the mount mode to use. The default mode is Read/Write.
`public int ` [`mount`](#_arduino___p_o_s_i_x_storage_8h_1a22178afb74ae05ab1dcf8c50eb4a9d1f)`(const enum ` [`StorageDevices`](#_arduino___p_o_s_i_x_storage_8h_1a97a26676f4f644e3db23bb63b9227546)` deviceName, const enum ` [`FileSystems`](#_arduino___p_o_s_i_x_storage_8h_1ac01996562b852a6b36ad87908429ad35)` fileSystem, const enum ` [`MountFlags`](#_arduino___p_o_s_i_x_storage_8h_1a069889b849809b552adf0513c6db2b85)` mountFlags)`            | Attach a file system to a device.
`public int ` [`umount`](#_arduino___p_o_s_i_x_storage_8h_1a57b5f0c881dedaf55fe1b9c5fa59e1f8)`(const enum ` [`StorageDevices`](#_arduino___p_o_s_i_x_storage_8h_1a97a26676f4f644e3db23bb63b9227546)` deviceName)`            | Remove the attached file system from a device.
`public int ` [`register_hotplug_callback`](#_arduino___p_o_s_i_x_storage_8h_1a1a914f0970d317b6a74bef4368cbcae8)`(const enum ` [`StorageDevices`](#_arduino___p_o_s_i_x_storage_8h_1a97a26676f4f644e3db23bb63b9227546)` deviceName, void(*)() callbackFunction)`            | Register a hotplug callback function. Currently only supported for DEV_USB on Portenta C33.
`public int ` [`deregister_hotplug_callback`](#_arduino___p_o_s_i_x_storage_8h_1ae80d0ace82aad5ef4a130953290efbd7)`(const enum ` [`StorageDevices`](#_arduino___p_o_s_i_x_storage_8h_1a97a26676f4f644e3db23bb63b9227546)` deviceName)`            | Deregister a previously registered hotplug callback function. Not currently supported on any platform.
`public int ` [`mkfs`](#_arduino___p_o_s_i_x_storage_8h_1a834ae6d0e65c5b47f9d8932f7ad0c499)`(const enum ` [`StorageDevices`](#_arduino___p_o_s_i_x_storage_8h_1a97a26676f4f644e3db23bb63b9227546)` deviceName, const enum ` [`FileSystems`](#_arduino___p_o_s_i_x_storage_8h_1ac01996562b852a6b36ad87908429ad35)` fileSystem)`            | Format a device (make file system).

## Members

#### `enum ` [`StorageDevices`](#_arduino___p_o_s_i_x_storage_8h_1a97a26676f4f644e3db23bb63b9227546) <a id="_arduino___p_o_s_i_x_storage_8h_1a97a26676f4f644e3db23bb63b9227546" class="anchor"></a>

Enum to select the storage device to use.

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
DEV_SDCARD            | SD Card.
DEV_USB            | USB Thumb Drive.

<hr />

#### `enum ` [`FileSystems`](#_arduino___p_o_s_i_x_storage_8h_1ac01996562b852a6b36ad87908429ad35) <a id="_arduino___p_o_s_i_x_storage_8h_1ac01996562b852a6b36ad87908429ad35" class="anchor"></a>

Enum to select the file system to use.

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
FS_FAT            | FAT file system.
FS_LITTLEFS            | LittleFS file system.

<hr />

#### `enum ` [`MountFlags`](#_arduino___p_o_s_i_x_storage_8h_1a069889b849809b552adf0513c6db2b85) <a id="_arduino___p_o_s_i_x_storage_8h_1a069889b849809b552adf0513c6db2b85" class="anchor"></a>

Enum to select the mount mode to use. The default mode is Read/Write.

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
MNT_DEFAULT            | Default mount mode (Read/Write)
MNT_RDONLY            | Read only mode.

<hr />

#### `public int ` [`mount`](#_arduino___p_o_s_i_x_storage_8h_1a22178afb74ae05ab1dcf8c50eb4a9d1f)`(const enum ` [`StorageDevices`](#_arduino___p_o_s_i_x_storage_8h_1a97a26676f4f644e3db23bb63b9227546)` deviceName, const enum ` [`FileSystems`](#_arduino___p_o_s_i_x_storage_8h_1ac01996562b852a6b36ad87908429ad35)` fileSystem, const enum ` [`MountFlags`](#_arduino___p_o_s_i_x_storage_8h_1a069889b849809b552adf0513c6db2b85)` mountFlags)` <a id="_arduino___p_o_s_i_x_storage_8h_1a22178afb74ae05ab1dcf8c50eb4a9d1f" class="anchor"></a>

Attach a file system to a device.

#### Parameters
* `deviceName` The device to attach to: DEV_SDCARD or DEV_USB. 

* `fileSystem` The file system type to attach: FS_FAT or FS_LITTLEFS. 

* `mountFlags` The only valid flag at this time: MNT_DEFAULT. 

#### Returns
On success: 0. On failure: -1 with an error code in the errno variable.
<hr />

#### `public int ` [`umount`](#_arduino___p_o_s_i_x_storage_8h_1a57b5f0c881dedaf55fe1b9c5fa59e1f8)`(const enum ` [`StorageDevices`](#_arduino___p_o_s_i_x_storage_8h_1a97a26676f4f644e3db23bb63b9227546)` deviceName)` <a id="_arduino___p_o_s_i_x_storage_8h_1a57b5f0c881dedaf55fe1b9c5fa59e1f8" class="anchor"></a>

Remove the attached file system from a device.

#### Parameters
* `deviceName` The device to remove from: DEV_SDCARD or DEV_USB. 

#### Returns
On success: 0. On failure: -1 with an error code in the errno variable.
<hr />

#### `public int ` [`register_hotplug_callback`](#_arduino___p_o_s_i_x_storage_8h_1a1a914f0970d317b6a74bef4368cbcae8)`(const enum ` [`StorageDevices`](#_arduino___p_o_s_i_x_storage_8h_1a97a26676f4f644e3db23bb63b9227546)` deviceName, void(*)() callbackFunction)` <a id="_arduino___p_o_s_i_x_storage_8h_1a1a914f0970d317b6a74bef4368cbcae8" class="anchor"></a>

Register a hotplug callback function. Currently only supported for DEV_USB on Portenta C33.

#### Parameters
* `deviceName` The device to register for: DEV_SDCARD or DEV_USB. 

* `callbackFunction` A function pointer to the callback. 

#### Returns
On success: 0. On failure: -1 with an error code in the errno variable.
<hr />

#### `public int ` [`deregister_hotplug_callback`](#_arduino___p_o_s_i_x_storage_8h_1ae80d0ace82aad5ef4a130953290efbd7)`(const enum ` [`StorageDevices`](#_arduino___p_o_s_i_x_storage_8h_1a97a26676f4f644e3db23bb63b9227546)` deviceName)` <a id="_arduino___p_o_s_i_x_storage_8h_1ae80d0ace82aad5ef4a130953290efbd7" class="anchor"></a>

Deregister a previously registered hotplug callback function. Not currently supported on any platform.

#### Parameters
* `deviceName` The device to deregister for: DEV_SDCARD or DEV_USB. 

#### Returns
On success: 0. On failure: -1 with an error code in the errno variable.
<hr />

#### `public int ` [`mkfs`](#_arduino___p_o_s_i_x_storage_8h_1a834ae6d0e65c5b47f9d8932f7ad0c499)`(const enum ` [`StorageDevices`](#_arduino___p_o_s_i_x_storage_8h_1a97a26676f4f644e3db23bb63b9227546)` deviceName, const enum ` [`FileSystems`](#_arduino___p_o_s_i_x_storage_8h_1ac01996562b852a6b36ad87908429ad35)` fileSystem)` <a id="_arduino___p_o_s_i_x_storage_8h_1a834ae6d0e65c5b47f9d8932f7ad0c499" class="anchor"></a>

Format a device (make file system).

#### Parameters
* `deviceName` The device to format: DEV_SDCARD or DEV_USB. 

* `fileSystem` The file system type to format: FS_FAT or FS_LITTLEFS. FS_FAT is probably the better choice for both SD Cards and USB thumb drives in most cases. 

#### Returns
On success: 0. On failure: -1 with an error code in the errno variable.
<hr />
