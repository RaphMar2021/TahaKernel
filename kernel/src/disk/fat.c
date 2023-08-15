#include "fat.h"

struct boot_sector *bs;
uint32_t current_directory_cluster;
char current_path[256] = "/";

void read_sector(uint32_t sector, void *buffer)
{
        outb(ATA_REG_DRIVESEL, 0xE0 | ((sector & 0x0F000000) >> 24));
        outb(ATA_REG_SECTORCOUNT, 1);
        outb(ATA_REG_LBA_LO, sector & 0xFF);
        outb(ATA_REG_LBA_MID, (sector & 0xFF00) >> 8);
        outb(ATA_REG_LBA_HI, (sector & 0xFF0000) >> 16);
        outb(ATA_REG_COMMAND, ATA_CMD_READ_PIO);

        while ((inb(ATA_REG_COMMAND) & 0x08) == 0)
                ;

        insw(ATA_REG_DATA, buffer, 256);
}

void write_sector(uint32_t sector, void *buffer)
{
        outb(ATA_REG_DRIVESEL, 0xE0 | ((sector & 0x0F000000) >> 24));
        outb(ATA_REG_SECTORCOUNT, 1);
        outb(ATA_REG_LBA_LO, sector & 0xFF);
        outb(ATA_REG_LBA_MID, (sector & 0xFF00) >> 8);
        outb(ATA_REG_LBA_HI, (sector & 0xFF0000) >> 16);
        outb(ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

        while ((inb(ATA_REG_COMMAND) & 0x08) == 0)
                ;

        outsw(ATA_REG_DATA, buffer, 256);
}

void wait_for_drive_ready()
{
        while (inb(ATA_REG_STATUS) & 0x80)
                ;
}

bool atapi_identify()
{
        outb(ATA_REG_DRIVESEL, 0xA0);
        outb(ATA_REG_COUNT, 0);
        outb(ATA_REG_LBA1, 0);
        outb(ATA_REG_LBA2, 0);
        outb(ATA_REG_LBA3, 0);

        outb(ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);

        wait_for_drive_ready();

        if (inb(ATA_REG_STATUS) & 0x01)
        {
                return false;
        }

        uint16_t data[256];
        for (int i = 0; i < 256; i++)
        {
                data[i] = inw(ATA_REG_DATA);
        }

        return true;
}

void atapi_read_sector(uint32_t lba, void *buffer)
{
        outb(ATA_REG_DRIVESEL, 0xA0);
        outb(ATA_REG_COUNT, 1);
        outb(ATA_REG_LBA1, lba & 0xFF);
        outb(ATA_REG_LBA2, (lba >> 8) & 0xFF);
        outb(ATA_REG_LBA3, (lba >> 16) & 0xFF);
        // outb(ATA_REG_LBA4, (lba >> 24) & 0xFF);

        outb(ATA_REG_COMMAND, ATA_CMD_READ_SECTORS_EXT);

        wait_for_drive_ready();

        insw(ATA_REG_DATA, buffer, 256);
}

void read_specific_sector(uint8_t bus, bool is_master, uint32_t sector, void *buffer)
{
        uint16_t io_base = bus == 0 ? PRIMARY_BUS_IO_BASE : SECONDARY_BUS_IO_BASE;
        uint8_t drive_select = is_master ? 0xA0 : 0xB0;

        outb(io_base + ATA_REG_DRIVESEL, drive_select | ((sector & 0x0F000000) >> 24));
        outb(io_base + ATA_REG_SECTORCOUNT, 1);
        outb(io_base + ATA_REG_LBA_LO, sector & 0xFF);
        outb(io_base + ATA_REG_LBA_MID, (sector & 0xFF00) >> 8);
        outb(io_base + ATA_REG_LBA_HI, (sector & 0xFF0000) >> 16);
        outb(io_base + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

        while ((inb(io_base + ATA_REG_COMMAND) & 0x08) == 0)
                ;

        insw(io_base + ATA_REG_DATA, buffer, 256);
}

void format_filename_83(const char *input, char *output)
{
        memset(output, ' ', 11);
        int i, j;

        for (i = 0, j = 0; i < 8 && input[j] != '.' && input[j] != '\0'; i++, j++)
        {
                output[i] = toupper(input[j]);
        }

        while (input[j] != '.' && input[j] != '\0')
        {
                j++;
        }

        if (input[j] == '.')
        {
                j++;
                for (i = 8; i < 11 && input[j] != '\0'; i++, j++)
                {
                        output[i] = toupper(input[j]);
                }
        }
        output[11] = '\0';
}

void format_entry_name(const char *entry_name, char *formatted_name)
{
        int i, j;

        for (i = 0, j = 0; i < 8 && entry_name[j] != ' '; j++)
        {
                formatted_name[i++] = tolower(entry_name[j]);
        }

        if (entry_name[8] != ' ')
        {
                formatted_name[i++] = '.';
        }

        for (j = 8; j < 11 && entry_name[j] != ' '; j++)
        {
                formatted_name[i++] = tolower(entry_name[j]);
        }
        formatted_name[i] = '\0';
}

void ls(uint32_t first_cluster, struct boot_sector *_bs)
{
        uint32_t sector = _bs->reserved_sector_count + (_bs->fat_count * _bs->table_size_32) + ((first_cluster - 2) * _bs->sectors_per_cluster);

        struct dir_entry entries[_bs->bytes_per_sector / sizeof(struct dir_entry)];

        printf("Listing directory:\n");
        printf("-------------------\n");

        for (uint32_t i = 0; i < _bs->sectors_per_cluster; i++)
        {
                read_sector(sector + i, entries);
                for (uint32_t j = 0; j < _bs->bytes_per_sector / sizeof(struct dir_entry); j++)
                {
                        if (entries[j].name[0] == 0x00 || (uint8_t)entries[j].name[0] == 0xE5)
                        {
                                continue;
                        }

                        if (entries[j].attr & 0x10)
                        { // Directory
                                char formatted_name[13];
                                format_entry_name(entries[j].name, formatted_name);
                                printf("[DIR] %s/\n", formatted_name);
                        }
                }
        }

        for (uint32_t i = 0; i < _bs->sectors_per_cluster; i++)
        {
                read_sector(sector + i, entries);
                for (uint32_t j = 0; j < _bs->bytes_per_sector / sizeof(struct dir_entry); j++)
                {
                        if (entries[j].name[0] == 0x00 || (uint8_t)entries[j].name[0] == 0xE5)
                        {
                                continue;
                        }

                        if (!(entries[j].attr & 0x10))
                        { // File
                                char formatted_name[13];
                                format_entry_name(entries[j].name, formatted_name);
                                printf("[FILE] %s\n", formatted_name);
                        }
                }
        }
}

uint32_t cluster_to_sector(uint32_t cluster, struct boot_sector *_bs)
{
        uint32_t first_sector_of_cluster_2 = _bs->reserved_sector_count + (_bs->fat_count * _bs->table_size_32);

        return (cluster - 2) * _bs->sectors_per_cluster + first_sector_of_cluster_2;
}

void cat(const char *filename, struct boot_sector *_bs)
{
        char formatted_name[12];
        format_filename_83(filename, formatted_name);

        uint32_t sector = _bs->reserved_sector_count + (_bs->fat_count * _bs->table_size_32);
        struct dir_entry entries[_bs->bytes_per_sector / sizeof(struct dir_entry)];
        bool file_found = false;

        for (uint32_t i = 0; i < _bs->sectors_per_cluster && !file_found; i++)
        {
                read_sector(sector + i, entries);

                for (uint32_t j = 0; j < _bs->bytes_per_sector / sizeof(struct dir_entry); j++)
                {
                        if (entries[j].name[0] == 0x00)
                        {
                                break;
                        }
                        else if ((uint8_t)entries[j].name[0] == 0xE5)
                        {
                                continue;
                        }

                        char entry_name[12];
                        memset(entry_name, 0, sizeof(entry_name));
                        memcpy(entry_name, entries[j].name, 11);

                        if (strcmp(entry_name, formatted_name) == 0)
                        {
                                file_found = true;
                                uint32_t cluster = (entries[j].cluster_high << 16) | entries[j].cluster_low;
                                uint32_t file_size = entries[j].size;
                                char buffer[_bs->bytes_per_sector];

                                while (file_size > 0)
                                {
                                        read_sector(cluster_to_sector(cluster, _bs), buffer);
                                        uint32_t size_to_print = (file_size > _bs->bytes_per_sector) ? _bs->bytes_per_sector : file_size;
                                        for (uint32_t k = 0; k < size_to_print; k++)
                                        {
                                                print_char(buffer[k]);
                                        }
                                        file_size -= size_to_print;

                                        uint32_t fat_sector = _bs->reserved_sector_count + (cluster * 4) / _bs->bytes_per_sector;
                                        uint32_t fat_offset = (cluster * 4) % _bs->bytes_per_sector;
                                        read_sector(fat_sector, buffer);
                                        cluster = *(uint32_t *)&buffer[fat_offset] & 0x0FFFFFFF;

                                        if (cluster == 0x0FFFFFFF)
                                        {
                                                break;
                                        }
                                }
                                break;
                        }
                }
        }

        if (!file_found)
        {
                printf("File not found: %s\n", formatted_name);
        }
}

void cd(const char *dirname, struct boot_sector *_bs)
{
        if (strcmp(dirname, "/") == 0)
        {
                current_directory_cluster = bs->root_cluster;
                strcpy(current_path, "/");
                return;
        }

        char formatted_name[12];
        format_filename_83(dirname, formatted_name);

        for (int i = 0; formatted_name[i]; i++)
        {
                formatted_name[i] = toupper(formatted_name[i]);
        }

        uint32_t sector = cluster_to_sector(current_directory_cluster, _bs);
        struct dir_entry entries[_bs->bytes_per_sector / sizeof(struct dir_entry)];
        bool dir_found = false;

        for (uint32_t i = 0; i < _bs->sectors_per_cluster && !dir_found; i++)
        {
                read_sector(sector + i, entries);

                for (uint32_t j = 0; j < _bs->bytes_per_sector / sizeof(struct dir_entry); j++)
                {
                        if (entries[j].name[0] == 0x00)
                        {
                                break;
                        }
                        else if ((uint8_t)entries[j].name[0] == 0xE5)
                        {
                                continue;
                        }

                        char entry_name[12];
                        memset(entry_name, 0, sizeof(entry_name));
                        memcpy(entry_name, entries[j].name, 11);

                        if (strcmp(entry_name, formatted_name) == 0)
                        {
                                dir_found = true;

                                current_directory_cluster = (entries[j].cluster_high << 16) | entries[j].cluster_low;

                                strcat(current_path, dirname);
                                strcat(current_path, "/");

                                break;
                        }
                }
        }

        if (!dir_found)
        {
                printf("Directory not found: %s\n", formatted_name);
        }
}

DiskFormat detect_disk_format(uint32_t sector)
{
        uint8_t buffer[512];
        read_sector(sector, buffer);

        if (memcmp(&buffer[1], "CD001", 5) == 0)
        {
                return DISK_ISO9660;
        }

        uint16_t *fat_signature = (uint16_t *)(&buffer[510]);
        if (*fat_signature == 0xAA55)
        {
                if (memcmp(&buffer[82], "FAT32", 5) == 0)
                {
                        return DISK_FAT32;
                }
                else if (memcmp(&buffer[54], "FAT", 3) == 0)
                {
                        return DISK_FAT16;
                }
        }

        return DISK_UNKNOWN;
}

void detect_disks()
{
        DiskFormat format = detect_disk_format(0);
        switch (format)
        {
        case DISK_FAT16:
                printf("Detected FAT16 disk\n");
                break;
        case DISK_FAT32:
                printf("Detected FAT32 disk\n");
                break;
        case DISK_ISO9660:
                printf("Detected ISO 9660 disk\n");
                break;
        default:
                printf("Unknown disk format\n");
                break;
        }
}
