
# File: main/CMakeLists.txt
# LilyGO THMI IDF Template Project
cmake_minimum_required(VERSION 3.16)
  
# Set usual component variables
set(app_sources "app_main.cpp" "init.cpp")
set(app_include_dirs "." "")
set(app_requires driver fatfs spi_flash esp_driver_usb_serial_jtag esp_lcd_touch_xpt2046 esp_lcd esp_lcd_touch lvgl ESP32_Resource_Monitor cmd_system)
set(app_priv_requires esp_bootloader_format nvs_flash esp_wifi console)

set(app_sources_resmon "cmd_resmon/cmd_resmon.c")
set(app_include_dirs_resmon "cmd_resmon")

idf_component_register(
    SRCS ${app_sources} ${app_sources_resmon}
    INCLUDE_DIRS ${app_include_dirs} ${app_include_dirs_resmon}
    REQUIRES  ${app_requires}
    PRIV_REQUIRES ${app_priv_requires}
    WHOLE_ARCHIVE
)

