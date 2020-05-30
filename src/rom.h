
#ifndef _ROM_H
#define _ROM_H

#define ROM_BANK_SIZE   0x4000
#define ROM_BANKS       128

#define GB_ROM_H_CART_TYPE  0x147
#define GB_ROM_H_SIZE       0x148
#define GB_ROM_H_RAM_SIZE   0x149

extern char     _dmg_bootSegmentRomStart[];
extern char     _dmg_bootSegmentRomEnd[];

struct VirtualBank
{
    unsigned char bankMemory[ROM_BANK_SIZE];
    int bankIndex;
    struct VirtualBank* nextBank;
    struct VirtualBank* prevBank;
};

struct ROMLayout
{
    unsigned char mainBank[ROM_BANK_SIZE];
    struct VirtualBank* firstVirtualBank;
    struct VirtualBank* lastVirtualBank;
    struct VirtualBank** romBankToVirtualBank;
    int romBankCount;
    void* romLocation;
};

extern struct ROMLayout gGBRom;

void initRomLayout(struct ROMLayout* romLayout, void *romLocation);

void finishRomLoad(struct ROMLayout* romLayout);

char* getROMBank(struct ROMLayout* romLayout, int bankIndex);

int getROMBankCount(struct ROMLayout* romLayout);
int getRAMBankCount(struct ROMLayout* romLayout);
int getCartType(struct ROMLayout* romLayout);

void loadBIOS(struct ROMLayout* romLayout, int gbc);
extern void unloadBIOS(struct ROMLayout* romLayout);

#endif