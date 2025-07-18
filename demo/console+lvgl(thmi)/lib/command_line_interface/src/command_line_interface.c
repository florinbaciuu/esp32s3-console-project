



#include "command_line_interface.h"


#include "modules/version/version.h"
#include "modules/restart/restart.h"
#include "modules/tasks_nfo/tasks_nfo.h"
#include "modules/ram_nfo/ram_nfo.h"

#include "modules/info/info.h"

// -------------------------------------------------

void cli_register_all_commands(void)
{
    cli_register_version_command(); // inclusa din "modules/version/version.h"
    cli_register_restart_command();
    cli_register_tasks_info_command();
    cli_register_ram_info_command();
}