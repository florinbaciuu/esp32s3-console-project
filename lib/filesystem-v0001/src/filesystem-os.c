
#include "defines.h"
#include "filesystem-os.h"

// --------------------------------------- //

#include "argtable3/argtable3.h"
#include "linenoise/linenoise.h"
#include "driver/sdmmc_host.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

// --------------------------------------- //



/**********************
 *   FILESYSTEM
 **********************/


// --------------------------------------- //

static const char* FS_SDMMC_TAG = "SD MMC";
// Init_SDMMC_FS
esp_err_t initialize_filesystem_sdmmc() {
    esp_vfs_fat_mount_config_t mount_config = {.format_if_mount_failed = false,
        .max_files                                                     = 4,
        .allocation_unit_size                                          = 16 * 1024,
        .disk_status_check_enable                                      = false,
        .use_one_fat                                                   = false};
    sdmmc_card_t*              card;
    const char                 mount_point[] = SD_MOUNT_PATH;
    sdmmc_host_t               host          = SDMMC_HOST_DEF();
    host.max_freq_khz                        = SD_FREQ_DEFAULT; // reducere la 20 MHz
    sdmmc_slot_config_t slot_config          = SDMMC_SLOT_CONFIG_DEF();
    slot_config.clk                          = (gpio_num_t) SDIO_SCLK_PIN;
    slot_config.cmd                          = (gpio_num_t) SDIO_CMD_PIN;
    slot_config.d0                           = (gpio_num_t) SDIO_DATA0_PIN;
    slot_config.width                        = 1;
    gpio_set_pull_mode((gpio_num_t) SDIO_CMD_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t) SDIO_DATA0_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t) SDIO_SCLK_PIN, GPIO_PULLUP_ONLY);
    esp_err_t InitMountFlagErr =
        esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (InitMountFlagErr != ESP_OK)
    {
        ESP_LOGE(FS_SDMMC_TAG, "Failed to mount SDMMC (%s)", esp_err_to_name(InitMountFlagErr));
        sdmmc_host_deinit();
        return InitMountFlagErr;
    }
    ESP_LOGI(FS_SDMMC_TAG, "SD card mounted at %s", mount_point);
    sdmmc_card_print_info(stdout, card);
    return ESP_OK;
}

// --------------------------------------- //

/*************************************************************************/
/*************************************************************************/

// FAT

static const char* FS_FFAT_TAG = "FFAT";

static wl_handle_t s_wl_handle = WL_INVALID_HANDLE; // Handle of the wear levelling library instance

esp_err_t initialize_internal_fat_filesystem() {
    const esp_vfs_fat_mount_config_t config = {
        .format_if_mount_failed   = true,
        .max_files                = 5,
        .allocation_unit_size     = CONFIG_WL_SECTOR_SIZE,
        .disk_status_check_enable = false,
        .use_one_fat              = false,
    };
    esp_err_t InitMountFlagErr =
        esp_vfs_fat_spiflash_mount_rw_wl(FAT_MOUNT_PATH, PARTITION_LABEL, &config, &s_wl_handle);
    if (InitMountFlagErr != ESP_OK)
    {
        ESP_LOGE(FS_FFAT_TAG, "Failed to mount FATFS (%s)", esp_err_to_name(InitMountFlagErr));
        return InitMountFlagErr;
    }
    ESP_LOGI(FS_FFAT_TAG, "Mounted FATFS at %s", FAT_MOUNT_PATH);
    return ESP_OK;
}

// --------------------------------------- //

bool init_filesystem_sys()
{
    initialize_filesystem_sdmmc();
    vTaskDelay(100);
    initialize_internal_fat_filesystem();
    ESP_LOGI("File System", "Filesystem mounted ALL");
    return 1;
}

// --------------------------------------- //