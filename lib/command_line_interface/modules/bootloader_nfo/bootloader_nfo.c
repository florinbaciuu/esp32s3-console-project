
#include "esp_console.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bootloader_nfo.h"

static const char *TAG = "Bootloader Info";

static int bootloader_info(int argc, char **argv)
{
    esp_bootloader_desc_t bootloader_desc;
    printf("\n");
    ESP_LOGI("Bootloader description", "\tESP-IDF version from 2nd stage bootloader: %s\n", bootloader_desc.idf_ver);
    ESP_LOGI("Bootloader description", "\tESP-IDF version from app: %s\n", IDF_VER);
    // printf("\tESP-IDF version from 2nd stage bootloader: %s\n", bootloader_desc.idf_ver);
    // printf("\tESP-IDF version from app: %s\n", IDF_VER);
    return 0;
}

static void register_bootloader_info(void)
{
    const esp_console_cmd_t cmd = {
        .command = "bootloader info",
        .help = "Get information about bootloader",
        .hint = NULL,
        .func = &bootloader_info,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/*
** Asta e functia principala de inregistrare a comenzilor
** pentru CLI-ul de versiune.
*/
void cli_register_bootloader_info_command(void)
{
    register_bootloader_info(); // Register the 'register_ram_info' command
}