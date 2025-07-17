



#include "command_line_interface.h"
#include "modules/version/version.h"

// -------------------------------------------------

void cli_register_all_commands(void)
{
    cli_register_version_commands(); // inclusa din "modules/version/version.h"
}