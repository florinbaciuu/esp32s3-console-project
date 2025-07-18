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

extern void printRamInfoatBoot(void);

static struct
{
    struct arg_str *subcmd;
    struct arg_end *end;
} info_args;

static int info_command(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&info_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, info_args.end, argv[0]);
        return 1;
    }

    const char *subcmd = info_args.subcmd->sval[0];

    if (strcmp(subcmd, "sys") == 0)
    {
        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);
        printf("System Info:\n");
        printf("  Model: %s\n", CONFIG_IDF_TARGET);
        printf("  Cores: %d\n", chip_info.cores);
        printf("  Revision: %d\n", chip_info.revision);
    }
    else if (strcmp(subcmd, "flash") == 0)
    {
        uint32_t flash_size;
        esp_flash_get_size(NULL, &flash_size);
        printf("Flash Info:\n  Size: %lu bytes\n", (unsigned long)flash_size);
    }
    else if (strcmp(subcmd, "cpu") == 0)
    {
        printf("CPU Info:de implemtentat\n");
    }
    else if (strcmp(subcmd, "psram") == 0)
    {
        printf("PSRAM Info:de implemtentat\n");
    }
    else if (strcmp(subcmd, "ram") == 0)
    {
        printRamInfoatBoot();
    }
    else
    {
        printf("Unknown subcommand: %s\n", subcmd);
    }

    return 0;
}

void cli_register_info_command(void)
{
    info_args.subcmd = arg_str1(NULL, NULL, "<subcommand>", "Subcommand: sys, flash, cpu, psram, ram");
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
