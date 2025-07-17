

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include "esp_log.h"
#include "esp_console.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cmd_system.h"
#include "sdkconfig.h"

#include "cmd_resmon.h"

#include "version.h"

static const char *TAG = "cmd_resmon";

static void register_version(void);


//===========================================================
void register_all_commands(void)
{
    register_version(); // Register the 'version' command
}

