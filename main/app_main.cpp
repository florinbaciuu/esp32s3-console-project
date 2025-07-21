/**
 * @file      app_main.cpp. 
 */

/*********************
 *      DEFINES
 *********************/
#define PWR_EN_PIN                 (10)                // connected to the battery alone
//---------
#define PWR_ON_PIN    (14)  // if you use an ext 5V power supply, you need to bring a magnet close to the ReedSwitch and set the PowerOn Pin (GPIO14) to HIGH
#define Dellp_OFF_PIN (21)  // connected to the battery and the USB power supply, used to turn off the device
//---------


//---------

/*********************
 *      INCLUDES
 *********************/
extern "C" {
#include <stdio.h>

#include "esp_bootloader_desc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "ResourceMonitor.h"

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "esp_system.h"
#include "esp_console.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "driver/sdmmc_host.h"
#include "esp_vfs_fat.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "soc/soc_caps.h"
#include "esp_event.h"
#include "sdmmc_cmd.h"

#include "command_line_interface.h"
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
// ----------------------------------------------------------
// ----------------------------------------------------------

#define SD_CS_PIN      (15)  // Chip Select pentru SPI
#define SD_MISO_PIN    (13)
#define SD_MOSI_PIN    (11)
#define SD_SCLK_PIN    (12)
#define SDIO_DATA0_PIN (13)
#define SDIO_CMD_PIN   (11)
#define SDIO_SCLK_PIN  (12)

#define SD_FREQ_DEFAULT   20000 /*!< SD/MMC Default speed (limited by clock divider) */
#define SD_FREQ_HIGHSPEED 40000 /*!< SD High speed (limited by clock divider) */





/* Console command history can be stored to and loaded from a file.
 * The easiest way to do this is to use FATFS filesystem on top of
 * wear_levelling library.
 */
#if CONFIG_CONSOLE_STORE_HISTORY

static void initialize_filesystem_sdmmc(void) {
  esp_vfs_fat_mount_config_t mount_config = {
    .format_if_mount_failed = false, .max_files = 4, .allocation_unit_size = 16 * 1024, .disk_status_check_enable = false, .use_one_fat = false
  };

  sdmmc_card_t *card;
  const char mount_point[] = MOUNT_PATH;

  // Configurare SDMMC host
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();

  // Configurare pini slot SDMMC
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  slot_config.clk = (gpio_num_t)SDIO_SCLK_PIN;
  slot_config.cmd = (gpio_num_t)SDIO_CMD_PIN;
  slot_config.d0 = (gpio_num_t)SDIO_DATA0_PIN;
  slot_config.width = 1;  // 1-bit mode

  // (daca ai pull-up externi, poți comenta linia de mai jos)
  gpio_set_pull_mode((gpio_num_t)SDIO_CMD_PIN, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode((gpio_num_t)SDIO_DATA0_PIN, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode((gpio_num_t)SDIO_SCLK_PIN, GPIO_PULLUP_ONLY);

  esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
  if (ret != ESP_OK) {
    ESP_LOGE("SD", "Failed to mount SDMMC (%s)", esp_err_to_name(ret));
    return;
  }

  ESP_LOGI("SD", "SD card mounted at %s", mount_point);
  sdmmc_card_print_info(stdout, card);
}

#else
//#define HISTORY_PATH NULL
#endif  // CONFIG_CONSOLE_STORE_HISTORY

static void initialize_nvs(void) {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
}






// ----------------------------------------------------------


//---------

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
//---------
void gpio_extra_set_init(uint32_t mode) {
  // Setăm ambii pini ca output
  gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << PWR_EN_PIN) | (1ULL << PWR_ON_PIN),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&io_conf);
  gpio_set_level((gpio_num_t)PWR_EN_PIN, mode);
  gpio_set_level((gpio_num_t)PWR_ON_PIN, mode);  // nu e nevoie de el daca alimentam usb
}
//---------
void power_latch_init() {
  gpio_config_t io_conf = {
    .pin_bit_mask = 1ULL << PWR_EN_PIN,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
  };
  gpio_config(&io_conf);
  gpio_set_level((gpio_num_t)PWR_EN_PIN, 1);  // ⚡ ține placa aprinsă
}
//---------
//---------



//---------



/*
███████ ██████  ███████ ███████ ██████ ████████  ██████  ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██ ██      
█████   ██████  █████   █████   ██████    ██    ██    ██ ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██      ██ 
██      ██   ██ ███████ ███████ ██   ██   ██     ██████  ███████ 
*/

/*********************
 *  rtos variables
 *********************/
TaskHandle_t xHandle_ResourceMonitor;
//---------

//---------

/****************************/
//--------------------------------------

/*
███    ███  █████  ██ ███    ██ 
████  ████ ██   ██ ██ ████   ██ 
██ ████ ██ ███████ ██ ██ ██  ██ 
██  ██  ██ ██   ██ ██ ██  ██ ██ 
██      ██ ██   ██ ██ ██   ████ 
  * This is the main entry point of the application.
  * It initializes the hardware, sets up the display, and starts the LVGL tasks.
  * The application will run indefinitely until the device is powered off or reset.
*/
extern "C" void app_main(void) {
  //// gpio_extra_set_init(1);
  power_latch_init();  // Inițializare latch pentru alimentare
  //// esp_log_level_set("*", ESP_LOG_MAX); //
  esp_log_level_set("*", ESP_LOG_INFO);

  //initialize_nvs();

#if CONFIG_CONSOLE_STORE_HISTORY
  //initialize_filesystem();
  initialize_filesystem_sdmmc(); 
  ESP_LOGI("CONSOLE", "Command history enabled");
#else
  ESP_LOGI("CONSOLE", "Command history disabled");
#endif
  

  esp_bootloader_desc_t bootloader_desc;
  printf("\n");
  ESP_LOGI("Bootloader description", "\tESP-IDF version from 2nd stage bootloader: %s\n", bootloader_desc.idf_ver);
  ESP_LOGI("Bootloader description", "\tESP-IDF version from app: %s\n", IDF_VER);
  // printf("\tESP-IDF version from 2nd stage bootloader: %s\n", bootloader_desc.idf_ver);
  // printf("\tESP-IDF version from app: %s\n", IDF_VER);


  // start_resource_monitor();
  //vTaskDelay(pdMS_TO_TICKS(1000));
  StartCLI();

}  // app_main




/********************************************** */

/**********************
 *   END OF FILE
 **********************/
