#ifndef ISO9660_H
#define ISO9660_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <video/video.h>

#define ISO9660_SECTOR_SIZE 2048
#define ISO9660_PVD_SECTOR 16

typedef struct
{
        uint8_t length;
        uint8_t ext_length;
        uint32_t extent_location;
        uint32_t data_length;
        uint8_t date[7];
        uint8_t flags;
        uint8_t file_unit_size;
        uint8_t interleave;
        uint16_t seq_number;
        uint8_t name_len;
} __attribute__((packed)) directory_entry;

typedef struct
{
        uint8_t type;
        char id[5];
        uint8_t version;
        uint8_t unused1;
        char system_id[32];
        char volume_id[32];
        uint8_t unused2[8];
        uint32_t volume_space_size;
        directory_entry root_directory_record;
} __attribute__((packed)) primary_volume_descriptor;

void list_iso9660_files();

#endif