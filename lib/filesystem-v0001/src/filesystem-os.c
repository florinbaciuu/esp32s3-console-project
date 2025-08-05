
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

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_littlefs.h"
#include "esp_spiffs.h"

// --------------------------------------- //

/**********************
 *   FILESYSTEM
 **********************/

// --------------------------------------- //

static const char* SDMMC_TAG = "SDMMCFS";

bool fs_test_sdmmc(); //. prototype

sdmmc_card_t* card;

// Init_SDMMC_FS
esp_err_t initialize_filesystem_sdmmc() {
    ESP_LOGI(SDMMC_TAG, "Initializing eMMC");
    esp_vfs_fat_mount_config_t mount_config = {.format_if_mount_failed = false,
        .max_files                                                     = 4,
        .allocation_unit_size                                          = 16 * 1024,
        .disk_status_check_enable                                      = false,
        .use_one_fat                                                   = false};

    const char   mount_point[]      = SD_MOUNT_PATH;
    sdmmc_host_t host               = SDMMC_HOST_DEF();
    host.max_freq_khz               = SD_FREQ_DEFAULT; // reducere la 20 MHz
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEF();
    ESP_LOGI(SDMMC_TAG, "Using SDMMC peripheral.");
    slot_config.clk   = (gpio_num_t) SDIO_SCLK_PIN;
    slot_config.cmd   = (gpio_num_t) SDIO_CMD_PIN;
    slot_config.d0    = (gpio_num_t) SDIO_DATA0_PIN;
    slot_config.width = 1;
    slot_config.cd    = SDMMC_NO_CD;
    slot_config.wp    = SDMMC_NO_WP;
    ESP_LOGI(SDMMC_TAG, "Set GPIO.");
    gpio_set_pull_mode((gpio_num_t) SDIO_CMD_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t) SDIO_DATA0_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode((gpio_num_t) SDIO_SCLK_PIN, GPIO_PULLUP_ONLY);
    ESP_LOGI(SDMMC_TAG, "Mounting SDMMC filesystem");
    esp_err_t InitMountFlagErr =
        esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (InitMountFlagErr != ESP_OK)
    {
        ESP_LOGE(SDMMC_TAG,
            "Failed to mount SDMMC (%s)",
            "If you want the eMMC to be formatted, set the disk_status_check_enable = true.",
            "Make sure eMMC lines have pull-up resistors in place.",
            esp_err_to_name(InitMountFlagErr));
        sdmmc_host_deinit(); // de init the sdcard
        return InitMountFlagErr;
    }
    ESP_LOGI(SDMMC_TAG, "SD card mounted at %s", mount_point);
    sdmmc_card_print_info(stdout, card);
    // sdmmc_card_print_info(stdout, card);
    fs_test_sdmmc();
    return ESP_OK;
}

// --------------------------------------- //

esp_err_t deinitialize_filesystem_sdmmc() {
    // All done, unmount partition and disable SDMMC peripheral
    ESP_LOGI(SDMMC_TAG, "Unmounting SDMMC filesystem");
    esp_err_t DeInitMountFlagErr = esp_vfs_fat_sdcard_unmount(SD_MOUNT_PATH, card);
    ESP_LOGI(SDMMC_TAG, "Card unmounted");
    return DeInitMountFlagErr;
}

// --------------------------------------- //

bool fs_test_sdmmc() {
    //  First create a file.
    const char* file_hello = SD_MOUNT_PATH "/test_sd.txt";
    ESP_LOGI(SDMMC_TAG, "Opening file %s", file_hello);
    FILE* f = fopen(file_hello, "w");
    if (f == NULL)
    {
        ESP_LOGE(SDMMC_TAG, "Failed to open file for writing");
        return 0;
    }
    fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(SDMMC_TAG, "File written");
    // Open renamed file for reading
    ESP_LOGI(SDMMC_TAG, "Reading file %s", file_hello);
    f = fopen(file_hello, "r");
    if (f == NULL)
    {
        ESP_LOGE(SDMMC_TAG, "Failed to open file for reading");
        return 0;
    }
    char line[128];               // Set A 128 buffer
    fgets(line, sizeof(line), f); // Read a line from file
    fclose(f);
    char* pos = strchr(line, '\n'); // Strip newline
    if (pos)
    {
        *pos = '\0';
    }
    ESP_LOGI(SDMMC_TAG, "Read from file: '%s'", line);
    return 1;
}

/*************************************************************************/
/*************************************************************************/

// FAT INTERNAL

static const char* FFAT_TAG = "FATFS";

static wl_handle_t s_wl_handle = WL_INVALID_HANDLE; // Handle of the wear levelling library instance

esp_err_t initialize_internal_fat_filesystem() {
    const esp_vfs_fat_mount_config_t config = {
        .format_if_mount_failed   = true,
        .max_files                = 5,
        .allocation_unit_size     = CONFIG_WL_SECTOR_SIZE,
        .disk_status_check_enable = false,
        .use_one_fat              = false,
    };
    ESP_LOGI(FFAT_TAG, "Mounting internal filesystem");
    esp_err_t InitMountFlagErr =
        esp_vfs_fat_spiflash_mount_rw_wl(FAT_MOUNT_PATH, PARTITION_LABEL, &config, &s_wl_handle);
    if (InitMountFlagErr != ESP_OK)
    {
        ESP_LOGE(FFAT_TAG, "Failed to mount FATFS (%s)", esp_err_to_name(InitMountFlagErr));
        return InitMountFlagErr;
    }
    ESP_LOGI(FFAT_TAG, "Mounted FATFS at %s", FAT_MOUNT_PATH);
    return ESP_OK;
}

/*************************************************************************/
/*************************************************************************/

static const char* FS_TAG = "FS";

bool init_filesystem_sys() {
    initialize_filesystem_sdmmc();
    vTaskDelay(100);
    initialize_internal_fat_filesystem();
    ESP_LOGI(FS_TAG, "Filesystem mounted");
    return 1;
}

// --------------------------------------- //

/*************************************************************************/
/*************************************************************************/

static const char* LITTLEFS_TAG = "LITTLEFS";

esp_err_t initialize_filesystem_littlefs() {
    ESP_LOGI(LITTLEFS_TAG, "Initializing LittleFS");

    esp_vfs_littlefs_conf_t conf = {
        .base_path              = "/littlefs",
        .partition_label        = "littlefs",
        .format_if_mount_failed = true,
        .dont_mount             = false,
    };

    // Use settings defined above to initialize and mount LittleFS filesystem.
    // Note: esp_vfs_littlefs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(LITTLEFS_TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(LITTLEFS_TAG, "Failed to find LittleFS partition");
        } else
        {
            ESP_LOGE(LITTLEFS_TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(LITTLEFS_TAG,
            "Failed to get LittleFS partition information (%s)",
            esp_err_to_name(ret));
        esp_littlefs_format(conf.partition_label);
    } else
    {
        ESP_LOGI(LITTLEFS_TAG, "Partition size: total: %d, used: %d", total, used);
    }

    // Use POSIX and C standard library functions to work with files.
    // First create a file.
    ESP_LOGI(LITTLEFS_TAG, "Opening file");
    FILE *f = fopen("/littlefs/hello.txt", "w");
    if (f == NULL) {
        ESP_LOGE(LITTLEFS_TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, "Hello World!\n");
    fclose(f);
    ESP_LOGI(LITTLEFS_TAG, "File written");

    // Rename original file
    // ESP_LOGI(TAG, "Renaming file");
    // if (rename("/littlefs/hello.txt", "/littlefs/foo.txt") != 0) {
    //     ESP_LOGE(TAG, "Rename failed");
    //     return;
    // }

    // Open renamed file for reading
    ESP_LOGI(LITTLEFS_TAG, "Reading file");
    f = fopen("/littlefs/hello.txt", "r");
    if (f == NULL) {
        ESP_LOGE(LITTLEFS_TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }

    char line[128] = {0};
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char* pos = strpbrk(line, "\r\n");
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(LITTLEFS_TAG, "Read from file: '%s'", line);

    ESP_LOGI(LITTLEFS_TAG, "Reading from flashed filesystem example.txt");
    f = fopen("/littlefs/example.txt", "r");
    if (f == NULL) {
        ESP_LOGE(LITTLEFS_TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    pos = strpbrk(line, "\r\n");
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(LITTLEFS_TAG, "Read from file: '%s'", line);
    return ESP_OK;
}

/*************************************************************************/
/*************************************************************************/

static const char * SPIFFS_TAG = "SPIFFS";

esp_err_t initialize_filesystem_spiffs(){
    ESP_LOGI(SPIFFS_TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(SPIFFS_TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(SPIFFS_TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(SPIFFS_TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    ESP_LOGI(SPIFFS_TAG, "Performing SPIFFS_check().");
    ret = esp_spiffs_check(conf.partition_label);
    if (ret != ESP_OK) {
        ESP_LOGE(SPIFFS_TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
        return ESP_FAIL;
    } else {
        ESP_LOGI(SPIFFS_TAG, "SPIFFS_check() successful");
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(SPIFFS_TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
        esp_spiffs_format(conf.partition_label);
        return ESP_FAIL;
    } else {
        ESP_LOGI(SPIFFS_TAG, "Partition size: total: %d, used: %d", total, used);
    }

    // Check consistency of reported partition size info.
    if (used > total) {
        ESP_LOGW(SPIFFS_TAG, "Number of used bytes cannot be larger than total. Performing SPIFFS_check().");
        ret = esp_spiffs_check(conf.partition_label);
        // Could be also used to mend broken files, to clean unreferenced pages, etc.
        // More info at https://github.com/pellepl/spiffs/wiki/FAQ#powerlosses-contd-when-should-i-run-spiffs_check
        if (ret != ESP_OK) {
            ESP_LOGE(SPIFFS_TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
            return ESP_FAIL;
        } else {
            ESP_LOGI(SPIFFS_TAG, "SPIFFS_check() successful");
        }
    }

    // Use POSIX and C standard library functions to work with files.
    // First create a file.
    ESP_LOGI(SPIFFS_TAG, "Opening file");
    FILE* f = fopen("/spiffs/hello.txt", "w");
    if (f == NULL) {
        ESP_LOGE(SPIFFS_TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, "Hello World!\n");
    fclose(f);
    ESP_LOGI(SPIFFS_TAG, "File written");

    // // Check if destination file exists before renaming
    // struct stat st;
    // if (stat("/spiffs/foo.txt", &st) == 0) {
    //     // Delete it if it exists
    //     unlink("/spiffs/foo.txt");
    // }

    // // Rename original file
    // ESP_LOGI(SPIFFS_TAG, "Renaming file");
    // if (rename("/spiffs/hello.txt", "/spiffs/foo.txt") != 0) {
    //     ESP_LOGE(SPIFFS_TAG, "Rename failed");
    //     return;
    // }

    // Open renamed file for reading
    ESP_LOGI(SPIFFS_TAG, "Reading file");
    f = fopen("/spiffs/hello.txt", "r");
    if (f == NULL) {
        ESP_LOGE(SPIFFS_TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char* pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(SPIFFS_TAG, "Read from file: '%s'", line);
    
    vTaskDelay(100);

    ESP_LOGI(LITTLEFS_TAG, "Reading from flashed filesystem example.txt");
    f = fopen("/spiffs/example.txt", "r");
    if (f == NULL) {
        ESP_LOGE(LITTLEFS_TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    pos = strpbrk(line, "\r\n");
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(LITTLEFS_TAG, "Read from file: '%s'", line);

    return ESP_OK;
}







/*************************************************************************/
/*************************************************************************/