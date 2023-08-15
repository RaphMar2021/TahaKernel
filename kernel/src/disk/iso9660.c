#include "iso9660.h"

void list_iso9660_files()
{
        primary_volume_descriptor pvd;
        atapi_read_sector(ISO9660_PVD_SECTOR, &pvd);

        if (strncmp(pvd.id, "CD001", 5) != 0)
        {
                printf("This is not a ISO 9660 CD-ROM !\n");
                return;
        }

        uint8_t buffer[ISO9660_SECTOR_SIZE];
        atapi_read_sector(pvd.root_directory_record.extent_location, buffer);

        uint32_t offset = 0;
        while (offset < ISO9660_SECTOR_SIZE)
        {
                directory_entry *entry = (directory_entry *)(buffer + offset);
                if (entry->length == 0)
                {
                        break;
                }

                if (entry->flags & 0x2)
                {
                }
                else
                {
                        char filename[entry->name_len + 1];
                        strncpy(filename, (char *)(entry + 1), entry->name_len);
                        filename[entry->name_len] = '\0';
                        printf("Fichier: %s\n", filename);
                }

                offset += entry->length;
        }
}