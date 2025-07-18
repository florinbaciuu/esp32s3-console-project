#include "info.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_console.h"
#include "esp_log.h"
#include "argtable3/argtable3.h"
#include <stdio.h>
#include <string.h>
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "esp_cpu.h"
#include "esp_psram.h"
#include <inttypes.h>
#include "soc/rtc.h"

#include "freertos/timers.h"
#include "esp_timer.h"

#define USE_INFO_CMD_TABLE 1 // 0 = clasic if/else, 1 = tabel lookup

#define USE_PRINTF 1
#define CONFIG_DEBUG_ESP_TIMERS 1

// tagul principal
static const char *TAG = "CLI INFO";

// -------------------------------

/***
 * Structura necesara functiei principale
 * Structura care contine alte 2 structuri
 */
static struct
{
    struct arg_str *subcommand;
    struct arg_end *end;
} info_args;

// -------------------------------

void printSysInfo()
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("System Info:\n");
    printf("  Model: %s\n", CONFIG_IDF_TARGET);
    printf("  Cores: %d\n", chip_info.cores);
    printf("  Revision: %d\n", chip_info.revision);
    return;
}

// -------------------------------

void printFlashInfo()
{
    uint32_t flash_size;
    esp_flash_get_size(NULL, &flash_size);
    printf("Flash Info:\n  Size: %lu bytes\n", (unsigned long)flash_size);
    return;
}

// -------------------------------

void printCPUInfo()
{
    printf("CPU Info:de implemtentat\n"); // TODO: Implement alternative for IDF v5.4.2
    return;
}

// -------------------------------

#define BYTES_TO_KB(x) ((float)(x) / 1024.0f)
#define PERC(used, total) ((total) > 0 ? ((float)(used) / (float)(total)) * 100.0f : 0.0f)
const char *MEMORY_TAG = "MEMORY";

#if (USE_PRINTF == 1)
// Helper macro pentru printare frumoasă
#define MEM_PRINT_ROW(label, total, free)                                                                                                                         \
    do                                                                                                                                                            \
    {                                                                                                                                                             \
        size_t used = (total) - (free);                                                                                                                           \
        printf(                                                                                                                                                   \
            "║ %-13s│ %7.1f KB  │ %7.1f KB │ %7.1f KB │   %6.2f %%     ║\n", label, BYTES_TO_KB(total), BYTES_TO_KB(used), BYTES_TO_KB(free), PERC(used, total)); \
    } while (0)
#else
// Helper macro
#define MEM_PRINT_ROW(label, total, free)                                                                                                               \
    do                                                                                                                                                  \
    {                                                                                                                                                   \
        size_t used = (total) - (free);                                                                                                                 \
        ESP_LOGI(                                                                                                                                       \
            MEMORY_TAG, "║ %-13s│ %7.1f KB  │ %7.1f KB │ %7.1f KB │   %6.2f %%     ║", label, BYTES_TO_KB(total), BYTES_TO_KB(used), BYTES_TO_KB(free), \
            PERC(used, total));                                                                                                                         \
    } while (0)
#endif

/**
 * @brief Function to print memory information
 * @note This function prints the memory information in a formatted way
 * @note It uses ESP_LOGI for logging if USE_ESP_LOGI is defined, otherwise it uses printf
 */
void printInfoAboutMemory()
{
#if (USE_PRINTF == 1)
    printf("╔═══════════════════════════════ MEMORY STATS ═════════════════════════════╗\n");
    printf("║   Segment     │    Total     │    Used     │   Free      │ Utilizare %%   ║\n");
    printf("╟───────────────┼──────────────┼─────────────┼─────────────┼────────────────╢\n");
    MEM_PRINT_ROW("RAM", heap_caps_get_total_size(MALLOC_CAP_INTERNAL), heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    MEM_PRINT_ROW("RAM-DMA", heap_caps_get_total_size(MALLOC_CAP_DMA), heap_caps_get_free_size(MALLOC_CAP_DMA));
    MEM_PRINT_ROW("RAM 8 bit", heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT), heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    MEM_PRINT_ROW(
        "RAM 32 bit", heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT), heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT));
    MEM_PRINT_ROW("RTC RAM", heap_caps_get_total_size(MALLOC_CAP_RTCRAM), heap_caps_get_free_size(MALLOC_CAP_RTCRAM));
    MEM_PRINT_ROW("PSRAM", heap_caps_get_total_size(MALLOC_CAP_SPIRAM), heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    MEM_PRINT_ROW("PSRAM 8 bit", heap_caps_get_total_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT), heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    MEM_PRINT_ROW("PSRAM 32 bit", heap_caps_get_total_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_32BIT), heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_32BIT));
    printf("╚══════════════════════════════════════════════════════════════════════════╝\n");
    printf("\n\r");
#else
    ESP_LOGI(MEMORY_TAG, "╔═══════════════════════════════ MEMORY STATS ═════════════════════════════╗");
    ESP_LOGI(MEMORY_TAG, "║   Segment     │    Total     │    Used     │   Free      │ Utilizare %%   ║");
    ESP_LOGI(MEMORY_TAG, "╟───────────────┼──────────────┼─────────────┼─────────────┼────────────────╢");
    MEM_PRINT_ROW("RAM", heap_caps_get_total_size(MALLOC_CAP_INTERNAL), heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    MEM_PRINT_ROW("RAM-DMA", heap_caps_get_total_size(MALLOC_CAP_DMA), heap_caps_get_free_size(MALLOC_CAP_DMA));
    MEM_PRINT_ROW("RAM 8 bit", heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT), heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
    MEM_PRINT_ROW(
        "RAM 32 bit", heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT), heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT));
    MEM_PRINT_ROW("RTC RAM", heap_caps_get_total_size(MALLOC_CAP_RTCRAM), heap_caps_get_free_size(MALLOC_CAP_RTCRAM));
    MEM_PRINT_ROW("PSRAM", heap_caps_get_total_size(MALLOC_CAP_SPIRAM), heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    MEM_PRINT_ROW("PSRAM 8 bit", heap_caps_get_total_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT), heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    MEM_PRINT_ROW("PSRAM 32 bit", heap_caps_get_total_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_32BIT), heap_caps_get_free_size(MALLOC_CAP_SPIRAM | MALLOC_CAP_32BIT));
    ESP_LOGI(MEMORY_TAG, "╚══════════════════════════════════════════════════════════════════════════╝");
#endif /* #if (USE_PRINTF == 1) */
}

// -------------------------------

// -------------------------------

void print_esp_timers()
{
#if (CONFIG_DEBUG_ESP_TIMERS == 1)
    ESP_LOGI(TAG, "-----------------ESP Timer Dump Start------------");
    printf("\n\r");
    esp_timer_dump(stdout);
    printf("\n\r");
    ESP_LOGI(TAG, "-----------------ESP Timer Dump End--------------");
#endif
}

// -------------------------------

// -------------------------------

#if USE_INFO_CMD_TABLE
typedef void (*info_func_t)(void);

typedef struct
{
    const char *name;
    info_func_t func;
} info_command_entry_t;

static const info_command_entry_t info_cmds[] = {
    {"sys", printSysInfo},
    {"flash", printFlashInfo},
    {"cpu", printCPUInfo},
    {"ram", printInfoAboutMemory},
    {"timers", print_esp_timers},
};
#endif /* #if (USE_INFO_CMD_TABLE == 1) */

static int info_command(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&info_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, info_args.end, argv[0]);
        return 1;
    }
    const char *subcommand = info_args.subcommand->sval[0];
#if USE_INFO_CMD_TABLE
    size_t num_cmds = sizeof(info_cmds) / sizeof(info_cmds[0]);
    for (size_t i = 0; i < num_cmds; ++i)
    {
        if (strcmp(subcommand, info_cmds[i].name) == 0)
        {
            info_cmds[i].func();
            return 0;
        }
    }
    printf("Unknown subcommand: %s\n", subcommand);
    return 1;
#else
    if (strcmp(subcommand, "sys") == 0)
    {
        printSysInfo();
    }
    else if (strcmp(subcommand, "flash") == 0)
    {
        printFlashInfo();
    }
    else if (strcmp(subcommand, "cpu") == 0)
    {
        printCPUInfo();
    }
    else if (strcmp(subcommand, "ram") == 0)
    {
        printInfoAboutMemory();
    }
    else if (strcmp(subcommand, "timers") == 0)
    {
        print_esp_timers();
    }
    else
    {
        printf("Unknown subcommand: %s\n", subcommand);
        return 1;
    }
    return 0;
#endif /* #if USE_INFO_CMD_TABLE */
}

// -------------------------------

void cli_register_info_command(void)
{
    info_args.subcommand = arg_str1(NULL, NULL, "<subcommand>", "Subcommand: sys, flash, cpu, ram, timers");
    info_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "info",
        .help = "System information commands",
        .hint = NULL,
        .func = &info_command,
        .argtable = &info_args,
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
    ESP_LOGI("CLI_Info", "'info' command registered");
}

// -------------------------------
