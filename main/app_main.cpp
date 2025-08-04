/**
 * @file : app_main.cpp.
 * @author : baf
 * board : t-hmi
 */

/*********************
 *      DEFINES
 *********************/
#define PWR_EN_PIN (10) // connected to the battery alone
#define PWR_ON_PIN (14)
#define Dellp_OFF_PIN (21) // ? IDK what it is ???
//---------
//---------

/*********************
 *      INCLUDES
 *********************/
extern "C" {
#include <stdio.h>
#include "driver/gpio.h"
#include "esp_bootloader_desc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "argtable3/argtable3.h"
#include "linenoise/linenoise.h"
#include "driver/sdmmc_host.h"
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "esp_console.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_vfs_fat.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "sdmmc_cmd.h"
#include "soc/soc_caps.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "command_line_interface.h"
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
// ----------------------------------------------------------
// ----------------------------------------------------------

/**********************
 *   SD MMC DEFINES
 **********************/
#define SD_CS_PIN (15)
#define SD_MISO_PIN (13)
#define SD_MOSI_PIN (11)
#define SD_SCLK_PIN (12)
#define SDIO_DATA0_PIN (13)
#define SDIO_CMD_PIN (11)
#define SDIO_SCLK_PIN (12)
//---
#define SD_FREQ_DEFAULT 20000   /*!< SD/MMC Default speed (limited by clock divider) */
#define SD_FREQ_HIGHSPEED 40000 /*!< SD High speed (limited by clock divider) */

/**********************
 *   Console history
 **********************/
/* Console command history can be stored to and loaded from a file.
 * The easiest way to do this is to use FATFS filesystem on top of
 * wear_levelling library.
 */
#if CONFIG_CONSOLE_STORE_HISTORY
#ifdef SDCARD_USE
void initialize_filesystem_sdmmc2() {
    esp_vfs_fat_mount_config_t mount_config = {.format_if_mount_failed = false,
        .max_files                                                     = 4,
        .allocation_unit_size                                          = 16 * 1024,
        .disk_status_check_enable                                      = false,
        .use_one_fat                                                   = false};
    sdmmc_card_t*              card;
    const char                 mount_point[] = MOUNT_PATH;
    sdmmc_host_t               host          = SDMMC_HOST_DEFAULT(); // Configurare SDMMC host
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();   // Configurare pini slot SDMMC
    slot_config.clk                 = (gpio_num_t) SDIO_SCLK_PIN;
    slot_config.cmd                 = (gpio_num_t) SDIO_CMD_PIN;
    slot_config.d0                  = (gpio_num_t) SDIO_DATA0_PIN;
    slot_config.width               = 1; // 1-bit mode
    gpio_set_pull_mode((gpio_num_t) SDIO_CMD_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t) SDIO_DATA0_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t) SDIO_SCLK_PIN, GPIO_PULLUP_ONLY);
    esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK)
    {
        ESP_LOGE("SD", "Failed to mount SDMMC (%s)", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI("SD", "SD card mounted at %s", mount_point);
    sdmmc_card_print_info(stdout, card);
}

esp_err_t initialize_filesystem_sdmmc() {
    esp_vfs_fat_mount_config_t mount_config = {.format_if_mount_failed = false,
        .max_files                                                     = 4,
        .allocation_unit_size                                          = 16 * 1024,
        .disk_status_check_enable                                      = false,
        .use_one_fat                                                   = false};
    sdmmc_card_t*              card;
    const char                 mount_point[] = MOUNT_PATH;
    sdmmc_host_t               host          = SDMMC_HOST_DEFAULT();
    host.max_freq_khz                        = SD_FREQ_DEFAULT; // reducere la 20 MHz
    sdmmc_slot_config_t slot_config          = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.clk                          = (gpio_num_t) SDIO_SCLK_PIN;
    slot_config.cmd                          = (gpio_num_t) SDIO_CMD_PIN;
    slot_config.d0                           = (gpio_num_t) SDIO_DATA0_PIN;
    slot_config.width                        = 1;
    gpio_set_pull_mode((gpio_num_t) SDIO_CMD_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t) SDIO_DATA0_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t) SDIO_SCLK_PIN, GPIO_PULLUP_ONLY);
    esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK)
    {
        ESP_LOGE("SD", "Failed to mount SDMMC (%s)", esp_err_to_name(ret));
        sdmmc_host_deinit();
        return ret;
    }
    ESP_LOGI("SD", "SD card mounted at %s", mount_point);
    sdmmc_card_print_info(stdout, card);
    return ESP_OK;
}
#else  /* !(SDCARD_USE) */
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE; // Handle of the wear levelling library instance

esp_err_t initialize_filesystem() {
    const esp_vfs_fat_mount_config_t config = {
        .format_if_mount_failed   = true,
        .max_files                = 5,
        .allocation_unit_size     = CONFIG_WL_SECTOR_SIZE,
        .disk_status_check_enable = false,
        .use_one_fat              = false,
    };
    esp_err_t err =
        esp_vfs_fat_spiflash_mount_rw_wl(MOUNT_PATH, PARTITION_LABEL, &config, &s_wl_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE("ffat", "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return err;
    }
    ESP_LOGI("ffat", "Mounted FATFS at %s", MOUNT_PATH);
    return ESP_OK;
}
#endif /* (SDCARD_USE) */
#else  /*  !(CONFIG_CONSOLE_STORE_HISTORY) */
#endif /* (CONFIG_CONSOLE_STORE_HISTORY) */

/**********************
 *   FS
 **********************/
void init_filesystem() {
#if CONFIG_CONSOLE_STORE_HISTORY
    esp_err_t history_fs_ok = ESP_OK;
#ifdef SDCARD_USE
    history_fs_ok = initialize_filesystem_sdmmc();
    if (history_fs_ok == ESP_OK)
    {
        ESP_LOGI("File System", "File system enabled on SDCARD");
    } else
    {
        ESP_LOGW("File System",
            "Failed to enable file system on SDCARD (%s)",
            esp_err_to_name(history_fs_ok));
    }
#else  /*  !(SDCARD_USE) */
    history_fs_ok = initialize_filesystem();
    if (history_fs_ok == ESP_OK)
    {
        ESP_LOGI("File System", "File system enabled on FFAT");
    } else
    {
        ESP_LOGW("File System",
            "Failed to enable file system on FFAT (%s)",
            esp_err_to_name(history_fs_ok));
    }
#endif /* (SDCARD_USE) */
    if (history_fs_ok == ESP_OK)
    {
        cli_set_history_path(MOUNT_PATH "/history.txt");
        ESP_LOGI("CLI", "Command history enabled on " MOUNT_PATH);
    } else
    {
        ESP_LOGW("CLI", "⚠️ Filesystem not mounted, disabling command history");
        cli_set_history_path(NULL); // fără history
    }
#else  /* !(CONFIG_CONSOLE_STORE_HISTORY)  */
    ESP_LOGI("CONSOLE", "Command history disabled");
    //// #define HISTORY_PATH NULL
#endif /* if (CONFIG_CONSOLE_STORE_HISTORY) */
}

static void initialize_nvs(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

// ----------------------------------------------------------

esp_err_t initialize_eeproom() {
    ESP_LOGI("eeproom", "🔧 Initializing NVS partition 'eeproom'...");
    esp_err_t err = nvs_flash_init_partition("eeproom");
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGW("eeproom", "⚠️ NVS partition is full or version mismatch. Erasing...");
        err = nvs_flash_erase_partition("eeproom");
        if (err != ESP_OK)
        {
            ESP_LOGE("eeproom", "❌ Failed to erase 'eeproom' partition: %s", esp_err_to_name(err));
            return err;
        }
        err = nvs_flash_init_partition("eeproom");
        if (err != ESP_OK)
        {
            ESP_LOGE("eeproom",
                "❌ Failed to re-initialize 'eeproom' after erase: %s",
                esp_err_to_name(err));
            return err;
        }
    }
    if (err == ESP_OK)
    {
        ESP_LOGI("eeproom", "✅ NVS partition 'eeproom' initialized successfully");
    } else
    {
        ESP_LOGE("eeproom", "❌ Failed to initialize NVS: %s", esp_err_to_name(err));
        return err;
    }
    nvs_handle_t handle; // Deschidem un handle ca să verificăm spațiul
    err = nvs_open_from_partition("eeproom", "diagnostic", NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGE("eeproom", "❌ Can't open NVS handle for diagnostics: %s", esp_err_to_name(err));
        return err;
    }
    nvs_close(handle);
    return ESP_OK;
}

//---------

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
//---------
void power_latch_init_5V(uint32_t mode) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PWR_EN_PIN) | (1ULL << PWR_ON_PIN), // Setăm ambii pini ca output
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t) PWR_EN_PIN, mode);
    gpio_set_level((gpio_num_t) PWR_ON_PIN, mode); // nu e nevoie de el daca alimentam usb
}
//---------
void power_latch_init_Battery() {
    gpio_config_t io_conf = {.pin_bit_mask = 1ULL << PWR_EN_PIN,
        .mode                              = GPIO_MODE_OUTPUT,
        .pull_up_en                        = GPIO_PULLUP_DISABLE,
        .pull_down_en                      = GPIO_PULLDOWN_DISABLE,
        .intr_type                         = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);
    gpio_set_level((gpio_num_t) PWR_EN_PIN, 1); // ⚡ ține placa aprinsă
}
//---------

/*
███████ ██████  ███████ ███████ ██████ ████████  ██████  ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██ ██      
█████   ██████  █████   █████   ██████    ██    ██    ██ ███████ 
██      ██   ██ ██      ██      ██   ██   ██    ██    ██      ██ 
██      ██   ██ ███████ ███████ ██   ██   ██     ██████  ███████ 
'// ⚡
*/

/*********************
 *  ...
 *********************/
//---------
/****************************/
//--------------------------------------

/*
███    ███  █████  ██ ███    ██ 
████  ████ ██   ██ ██ ████   ██ 
██ ████ ██ ███████ ██ ██ ██  ██ 
██  ██  ██ ██   ██ ██ ██  ██ ██ 
██      ██ ██   ██ ██ ██   ████ 
  ! This is the main entry point of the application.
  ! The application will run indefinitely until the device is powered off or reset.
*/
extern "C" void app_main(void) {
    vTaskDelay(pdMS_TO_TICKS(100));
    power_latch_init_5V(true);
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_log_level_set("*", ESP_LOG_INFO);
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_bootloader_desc_t bootloader_desc;
    ESP_LOGI("Bootloader", "DEtails about bootloader:\n");
    esp_rom_printf("\tESP-IDF version from 2nd stage bootloader: %s\n", bootloader_desc.idf_ver);
    esp_rom_printf("\tESP-IDF version from app: %s\n", IDF_VER);
    //// start_resource_monitor();
    heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
    vTaskDelay(pdMS_TO_TICKS(100));
    init_filesystem();
    //// initialize_nvs();
    //// ESP_ERROR_CHECK(initialize_eeproom());
    vTaskDelay(pdMS_TO_TICKS(100));
    StartCLI();
} // app_main
/********************************************** */
/**********************
 *   END OF FILE
 **********************/
