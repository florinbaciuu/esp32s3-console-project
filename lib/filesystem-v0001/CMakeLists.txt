

set(
    srcs 
    "src/filesystem-os.c"
)

set(
    include_dirs
    "include"
)

set(
    requires
    esp_common
)

set(
    priv_requires
    esp_common
    driver
    esp_timer
    console
    log
    spi_flash
    fatfs
    littlefs
)

idf_component_register(
    SRCS 
    ${srcs}
    INCLUDE_DIRS 
    ${include_dirs}
    REQUIRES
    ${requires}
    PRIV_REQUIRES
    ${priv_requires}
)
