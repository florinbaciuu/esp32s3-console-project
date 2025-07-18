#include "esp_console.h"
#include "esp_log.h"
#include "argtable3/argtable3.h"
#include <string.h>

#include "info.h"
#include "funct.h"

static const char *TAG = "CLI INFO";

// ------------------------------

void printInfoCommandList();

/***
 * Structura necesara functiei principale
 * Structura care contine alte 2 structuri
 */
static struct
{
    struct arg_str *subcommand;
    struct arg_lit *list; // <-- opțiunea nouă
    struct arg_end *end;
} info_args;

// -------------------------------

typedef void (*info_func_t)(void);
typedef struct
{
    const char *name;
    void (*function)(void);
    const char *description; // nou!
} info_command_entry_t;

static const info_command_entry_t info_cmds[] = {
    {"sys", printSysInfo, "Display chip model, cores, and revision"},
    {"flash", printFlashInfo, "Show flash memory size"},
    {"cpu", printCPUInfo, "Show CPU info (placeholder)"},
    {"ram", printInfoAboutMemory, "Print heap, DMA, RTC, PSRAM usage"},
    {"timers", print_esp_timers, "Dump all ESP timers info"},
    {"--list", printInfoCommandList, "List all available subcommands"},
};

//-------------------

void printInfoCommandList()
{
    printf("╔═══════════════════════════ AVAILABLE INFO COMMANDS ═══════════════════════════╗\n");
    printf("║ %-10s │ %-60s ║\n", "Command", "Description");
    printf("╟────────────┼──────────────────────────────────────────────────────────────────╢\n");

    for (size_t i = 0; i < sizeof(info_cmds) / sizeof(info_cmds[0]); ++i)
    {
        printf("║ %-10s │ %-60s ║\n", info_cmds[i].name, info_cmds[i].description ? info_cmds[i].description : "No description");
    }

    printf("╚══════════════════════════════════════════════════════════════════════════════╝\n");
}

//-------------------

#define INFO_CMD_COUNT (sizeof(info_cmds) / sizeof(info_cmds[0]))

static char info_cmds_help[128] = {0};

static void generate_info_cmds_help_text(void)
{
    strcpy(info_cmds_help, ":   ");
    for (size_t i = 0; i < INFO_CMD_COUNT; i++)
    {
        strcat(info_cmds_help, info_cmds[i].name);
        if (i < INFO_CMD_COUNT - 1)
            strcat(info_cmds_help, "; ");
    }
}

//-------------------

static int info_command(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **)&info_args);
    if (info_args.list->count > 0)
    {
        size_t num_cmds = sizeof(info_cmds) / sizeof(info_cmds[0]);
        printf("Available subcommands:\n");
        for (size_t i = 0; i < num_cmds; ++i)
        {
            printf("  - %s\n", info_cmds[i].name);
        }
        return 0;
    }
    if (nerrors != 0)
    {
        arg_print_errors(stderr, info_args.end, argv[0]);
        return 1;
    }
    const char *subcommand = info_args.subcommand->sval[0];
    size_t num_cmds = sizeof(info_cmds) / sizeof(info_cmds[0]);
    for (size_t i = 0; i < num_cmds; ++i)
    {
        if (strcmp(subcommand, info_cmds[i].name) == 0)
        {
            // verificăm dacă a cerut help pt subcmd
            if (argc > 2 && (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "-h") == 0))
            {
                printf("Help for '%s': %s\n", info_cmds[i].name, info_cmds[i].description);
                return 0;
            }
            info_cmds[i].function();
            return 0;
        }
    }
    printf("Unknown subcommand: %s\n", subcommand);
    printf("Type `info --list` to see available subcommands.\n");
    return 1;
}

// -------------------------------

void cli_register_info_command(void)
{
    generate_info_cmds_help_text();
    info_args.subcommand = arg_str1(
        NULL,            // nu are flag scurt, gen `-s
        NULL,            // nu are flag lung, gen `--subcmd`
        "<subcommand>",  // numele argumentului (pentru help/usage)
        info_cmds_help); // descrierea lui
    info_args.list = arg_lit0("l", "list", "List all available subcommands");
    info_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "info",
        .help = "System information commands",
        .hint = NULL,
        .func = &info_command,
        .argtable = &info_args,
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
    ESP_LOGI(TAG, "'%s' command registered.", cmd.command);
    return;
}

// -------------------------------
