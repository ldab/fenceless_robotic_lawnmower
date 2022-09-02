# fenceless-robotic-lawnmower



* SPP (Ble) server
* SPP (Ble) client

# Flashing the firmware

```
$python esptool.py esp32s3 -p COMx write_flash 0x0 bootloader/bootloader.bin 0x10000 spp_XXXX.bin 0x8000 partition_table/partition-table.bin
```

# How to use it:

1. Flash board with the firmware;
2. SPP (Ble) server waits for incoming connection;
3. SPP (Ble) client connects and start sending data forever;
4. Reset both units to start again.

# Scrapbook:

* Board dfinition: https://github.com/platformio/platform-espressif32/blob/master/boards/esp32-s3-devkitc-1.json

# Credits:

* Mars Rover: https://github.com/jakkra/Mars-Rover