#pragma once
#ifndef FILESYSTEM_OS_H
#define FILESYSTEM_OS_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// PROTOTYPES
esp_err_t initialize_filesystem_sdmmc();        // init SD MMC FAT Partition Filesystem
esp_err_t initialize_internal_fat_filesystem(); // init internat FAT Partition Filesystem

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* FILESYSTEM_OS_H */