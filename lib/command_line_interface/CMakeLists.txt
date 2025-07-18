
# Se adauga version
set(version_srcs "modules/version/version.c")
set(version_includes "modules/version")

# Se adauga restart
set(restart_srcs "modules/restart/restart.c")
set(restart_includes "modules/restart")

# Se adauga task info
set(tasks_srcs "modules/tasks_nfo/tasks_nfo.c")
set(tasks_includes "modules/tasks_nfo")

# Se adauga RAM info
set(ram_info_srcs "modules/ram_nfo/ram_nfo.c")
set(ram_info_includes "modules/ram_nfo")

# Se adauga uptime
set(uptime_srcs "modules/uptime/uptime.c")
set(uptime_includes "modules/uptime")

# ------------------------------ #

# Aduna toate modulele intr-un singur
set(modules_srcs ${version_srcs} ${restart_srcs} ${tasks_srcs} ${ram_info_srcs} ${uptime_srcs} )
set(modules_includes ${version_includes} ${restart_includes} ${tasks_includes} ${ram_info_includes} ${uptime_includes} )
set(modules_priv_includes ${version_includes} ${restart_includes} ${tasks_includes} ${ram_info_includes} ${uptime_includes} )

# ------------------------------- #

idf_component_register(
    SRCS "src/command_line_interface.c" "src/init.c" "src/config.c" ${modules_srcs}
    INCLUDE_DIRS "." "include" ${modules_includes}
    PRIV_INCLUDE_DIRS ${modules_priv_includes}
    REQUIRES driver freertos esp_timer console esp_hw_support spi_flash
)

# Asta adaug-o separat:
set(KCONFIG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Kconfig")

# Activează automat definirea opțiunilor Kconfig
#target_compile_definitions(${COMPONENT_LIB} PRIVATE "-DRESOURCE_MONITOR_CONFIG_FROM_KCONFIG")

