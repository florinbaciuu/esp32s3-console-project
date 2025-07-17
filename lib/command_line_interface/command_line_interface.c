



#include "command_line_interface.h"


#include "modules/version/version.h"
#include "modules/restart/restart.h"

// -------------------------------------------------

void cli_register_all_commands(void)
{
    cli_register_version_command(); // inclusa din "modules/version/version.h"
    cli_register_restart_command();
}