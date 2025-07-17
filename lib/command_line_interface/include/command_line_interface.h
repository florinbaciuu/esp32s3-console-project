#pragma once
#ifndef COMMAND_LINE_INTERFACE_H_
#define COMMAND_LINE_INTERFACE_H_

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"

#define CONSOLE_MAX_CMDLINE_ARGS 8
#define CONSOLE_MAX_CMDLINE_LENGTH 256
#define CONSOLE_PROMPT_MAX_LEN (32)

#define CONFIG_CONSOLE_STORE_HISTORY 1
#define CONFIG_CONSOLE_IGNORE_EMPTY_LINES 1
#define PROMPT_STR CONFIG_IDF_TARGET

#define MOUNT_PATH   "/sdcard"
#define HISTORY_PATH MOUNT_PATH "/history.txt"

extern char prompt[CONSOLE_PROMPT_MAX_LEN]; // 

#define MY_ESP_CONSOLE_CONFIG_DEFAULT() \
    {.max_cmdline_length = 256, .max_cmdline_args = 32, .heap_alloc_caps = MALLOC_CAP_DEFAULT, .hint_color = 39, .hint_bold = 0}

void initialize_console_peripheral(void);
void initialize_console_library(const char *history_path);
char *setup_prompt(const char *prompt_str);



// ------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif /* #ifdef __cplusplus */

    // register all commands
    void cli_register_all_commands(void);

    void StartCLI(bool activate);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */
#endif /* #ifndef COMMAND_LINE_INTERFACE_H_ */