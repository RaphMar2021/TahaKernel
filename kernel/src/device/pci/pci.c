#include "pci.h"

uint32_t pci_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
        uint32_t address;
        uint32_t lbus = (uint32_t)bus;
        uint32_t lslot = (uint32_t)slot;
        uint32_t lfunc = (uint32_t)func;

        address = (uint32_t)((lbus << 16) | (lslot << 11) |
                             (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

        outl(PCI_CONFIG_ADDRESS, address);
        return inl(PCI_CONFIG_DATA);
}

void pci_enum()
{
        uint16_t bus;
        uint8_t slot;
        uint8_t func;

        for (bus = 0; bus < 256; bus++)
        {
                for (slot = 0; slot < 32; slot++)
                {
                        for (func = 0; func < 8; func++)
                        {
                                uint32_t res = pci_read(bus, slot, func, 0);
                                uint16_t vendorID = res & 0xFFFF;
                                uint16_t deviceID = res >> 16;
                                if (vendorID == 0xFFFF)
                                        continue;

                                char buffer[16];

                                printf("PCI Device Found:\n");
                                printf("  Bus: ");
                                printf("%d", bus);
                                printf("\n");
                                printf("  Slot: ");
                                printf("%d", slot);
                                printf("\n");
                                printf("  Function: ");
                                printf("%d", func);
                                printf("\n");
                                printf("  Vendor ID: ");
                                printf("%d", vendorID);
                                printf("\n");
                                printf("  Device ID: ");
                                printf("%d", deviceID);
                                printf("\n");

                                serial_putsf("PCI Device Found:\n");
                                serial_putsf("  Bus: ");
                                serial_putsf("%d", bus);
                                serial_putsf("\n");
                                serial_putsf("  Slot: ");
                                serial_putsf("%d", slot);
                                serial_putsf("\n");
                                serial_putsf("  Function: ");
                                serial_putsf("%d", func);
                                serial_putsf("\n");
                                serial_putsf("  Vendor ID: ");
                                serial_putsf("%d", vendorID);
                                serial_putsf("\n");
                                serial_putsf("  Device ID: ");
                                serial_putsf("%d", deviceID);
                                serial_putsf("\n");
                        }
                }
        }
}