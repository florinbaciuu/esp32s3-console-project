/***
 * Code
 */

// include/command_line_interface.h
#include "command_line_interface.h"

// modules includes
#include "modules/version/version.h"
#include "modules/restart/restart.h"
#include "modules/tasks_nfo/tasks_nfo.h"
#include "modules/ram_nfo/ram_nfo.h"
#include "modules/uptime/uptime.h"

// -------------------------------------------------

void cli_register_all_commands(void)
{
    esp_console_register_help_command(); // asta efunctia predefinita a esp -idf.. aici trebuie lucrat in contionuare
    cli_register_version_command();      // inclusa din "modules/version/version.h"
    cli_register_restart_command();
    cli_register_tasks_info_command();
    cli_register_ram_info_command();
    cli_register_uptime_command();
}

// -------------------------------------------------

TaskHandle_t xHandle_esp32_cli;

void printStartupMessage()
{
    printf(
        "\n"
        "This is an example of ESP-IDF console component.\n"
        "Type 'help' to get the list of commands.\n"
        "Use UP/DOWN arrows to navigate through command history.\n"
        "Press TAB when typing command name to auto-complete.\n"
        "Ctrl+C will terminate the console environment.\n");

    if (linenoiseIsDumbMode())
    {
        printf(
            "\n"
            "Your terminal application does not support escape sequences.\n"
            "Line editing and history features are disabled.\n"
            "On Windows, try using Putty instead.\n");
    }
}

void rtos_init_cli(){
    /* Initialize console output periheral (UART, USB_OTG, USB_JTAG) */
  initialize_console_peripheral();

  /* Initialize linenoise library and esp_console*/
  initialize_console_library(HISTORY_PATH);

  /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
  const char *prompt = setup_prompt(PROMPT_STR ">");

  /* Register commands */
  // esp_console_register_help_command();
  //register_system_common();
  //cli_register_custom_help_command(); // aici am modificat ultima pt tine alua sa vezi
  cli_register_all_commands(); // my command
}

/********************************************** */
/*                   TASK                       */
/********************************************** */
void console_app(void *parameter)
{
    (void)parameter;
    rtos_init_cli();
    printStartupMessage();

    while (true)
    {
        char *line = linenoise(prompt);

#if CONFIG_CONSOLE_IGNORE_EMPTY_LINES
        if (line == NULL)
        { /* Ignore empty lines */
            continue;
            ;
        }
#else
        if (line == NULL)
        { /* Break on EOF or error */
            break;
        }
#endif // CONFIG_CONSOLE_IGNORE_EMPTY_LINES

        /* Add the command to the history if not empty*/
        if (strlen(line) > 0)
        {
            linenoiseHistoryAdd(line);
#if CONFIG_CONSOLE_STORE_HISTORY
            /* Save command history to filesystem */
            linenoiseHistorySave(HISTORY_PATH);
#endif // CONFIG_CONSOLE_STORE_HISTORY
        }

        /* Try to run the command */
        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND)
        {
            printf("Unrecognized command\n");
        }
        else if (err == ESP_ERR_INVALID_ARG)
        {
            // command was empty
        }
        else if (err == ESP_OK && ret != ESP_OK)
        {
            printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
        }
        else if (err != ESP_OK)
        {
            printf("Internal error: %s\n", esp_err_to_name(err));
        }
        /* linenoise allocates line buffer on the heap, so need to free it */
        linenoiseFree(line);
    }

    ESP_LOGE("CONSOLE", "Error or end-of-input, terminating console");
    esp_console_deinit();
}

// -------------------------------

void StartCLI(bool activate)
{
    if (activate == true)
    {
        xTaskCreatePinnedToCore(
            console_app,                           // Functia care ruleaza task-ul
            (const char *)"Console",               // Numele task-ului
            (uint32_t)(10000),                     // Dimensiunea stack-ului
            (NULL),                                // Parametri
            (UBaseType_t)configMAX_PRIORITIES - 6, // Prioritatea task-ului
            &xHandle_esp32_cli,                    // Handle-ul task-ului
            ((1))                                  // Nucleul pe care ruleaza (ESP32 e dual-core)
        );
    }
}