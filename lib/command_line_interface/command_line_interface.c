



#include "command_line_interface.h"
#include "version/version.h"



void cli_register_all_commands(void)
{
    cli_register_version_commands(); // Register the 'version' command
}