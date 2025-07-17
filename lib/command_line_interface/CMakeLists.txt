
# Se adauga version
set(version_srcs "modules/version/version.c")
set(version_includes "modules/version")

# Se adauga restart
set(restart_srcs "modules/restart/restart.c")
set(restart_includes "modules/restart")

idf_component_register(
    SRCS "command_line_interface.c" "init.c" "cli_rtos_task.c" ${version_srcs} ${restart_srcs}
    INCLUDE_DIRS "." ${version_includes} ${restart_includes}
    PRIV_INCLUDE_DIRS ${version_includes} ${restart_includes}
    REQUIRES driver freertos esp_timer console esp_hw_support spi_flash
)

# Asta adaug-o separat:
set(KCONFIG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Kconfig")

# Activează automat definirea opțiunilor Kconfig
#target_compile_definitions(${COMPONENT_LIB} PRIVATE "-DRESOURCE_MONITOR_CONFIG_FROM_KCONFIG")

