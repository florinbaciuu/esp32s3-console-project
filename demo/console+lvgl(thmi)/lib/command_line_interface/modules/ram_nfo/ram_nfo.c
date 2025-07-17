
#include "esp_console.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ram_nfo.h"

static const char *TAG = "RAM_info";

void printRamInfoatBoot(void) {
  ESP_LOGI("SYSTEM", "Total RAM          memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_INTERNAL));
  ESP_LOGI("SYSTEM", "Free RAM           memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

  ESP_LOGI("SYSTEM", "Total RAM-DMA      memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_DMA));
  ESP_LOGI("SYSTEM", "Free RAM-DMA       memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_DMA));

  ESP_LOGI("SYSTEM", "Total RAM 8 bit    memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
  ESP_LOGI("SYSTEM", "Free RAM 8 bit     memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));

  ESP_LOGI("SYSTEM", "Total RAM 32 bit   memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT));
  ESP_LOGI("SYSTEM", "Free RAM 32 bit    memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT));

  ESP_LOGI("SYSTEM", "Total RTC RAM      memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_RTCRAM));
  ESP_LOGI("SYSTEM", "Free RTC RAM       memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_RTCRAM));

  ESP_LOGI("SYSTEM", "Total PSRAM        memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
  ESP_LOGI("SYSTEM", "Free PSRAM         memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

  ESP_LOGI("SYSTEM", "Total PSRAM 8 bit  memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  ESP_LOGI("SYSTEM", "Free PSRAM 8 bit   memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));

  ESP_LOGI("SYSTEM", "Total PSRAM 32 bit memory: %u bytes", heap_caps_get_total_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_32BIT));
  ESP_LOGI("SYSTEM", "Free PSRAM 32 bit  memory: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_32BIT));

  ESP_LOGI("STACK", "Main task stack left: %d bytes", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));
}  // printRamInfoatBoot


static int ram_info(int argc, char **argv)
{
    printRamInfoatBoot();
    return 0;
}

static void register_ram_info(void)
{
    const esp_console_cmd_t cmd = {
        .command = "ram_info",
        .help = "Get information about RAM and PSRAM",
        .hint = NULL,
        .func = &ram_info,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

/*
** Asta e functia principala de inregistrare a comenzilor
** pentru CLI-ul de versiune.
*/
void cli_register_ram_info_command(void)
{
    register_ram_info(); // Register the 'register_ram_info' command
}