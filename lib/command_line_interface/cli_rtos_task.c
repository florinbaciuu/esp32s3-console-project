#include "command_line_interface.h"

TaskHandle_t xHandle_esp32_cli;

/********************************************** */
/*                   TASK                       */
/********************************************** */
void console_app(void *parameter)
{
    (void)parameter;
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
