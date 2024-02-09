# elecrow_esp32_5inch_display_stub


LGVL based UI for the Elecrow ESP32 5" display based based on the original elecrow factory provided version (which includes the lvgl demo app).




https://www.elecrow.com/esp32-display-5-inch-hmi-display-rgb-tft-lcd-touch-screen-support-lvgl.html

https://www.elecrow.com/wiki/index.php?title=ESP32_Display_5.0%27%27_Intelligent_Touch_Screen_Wi-Fi%26BLE_800*480_HMI_Display


# Board setup 
Board: ESP32S3 Dev Module
PSRAM: OPI PSRAM
Partition Scheme: Huge APP


# Libraries
Built on Arduino 2.3.0
lgvl: 8.3.11
lv_conf.h for the board saved as lv_conf_for_elecrow_esp32-5inh.h_original in the project folder.
lv_conf.h must be present in the root libraries directory of Arduino. 


Adrafruit GFX Library 1.11.9

