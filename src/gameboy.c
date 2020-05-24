
#include "gameboy.h"

struct GameBoy gGameboy;

void initGameboy(struct GameBoy* gameboy, struct ROMLayout* rom)
{
    initializeZ80(&gameboy->cpu);
    initMemory(&gameboy->memory, rom);

    gameboy->cpu.a = 0x01;
    gameboy->cpu.f = 0xB0;
    gameboy->cpu.b = 0x00;
    gameboy->cpu.c = 0x13;
    gameboy->cpu.d = 0x00;
    gameboy->cpu.e = 0xD8;
    gameboy->cpu.h = 0x01;
    gameboy->cpu.l = 0x4D;
    gameboy->cpu.pc = 0x100;
    gameboy->cpu.sp = 0xFFFE;

    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF05, 0x00);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF06, 0x00);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF07, 0x00);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF10, 0x80);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF11, 0xBF);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF12, 0xF3);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF14, 0xBF);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF16, 0x3F);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF17, 0x00);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF19, 0xBF);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF1A, 0x7F);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF1B, 0xFF);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF1C, 0x9F);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF1E, 0xBF);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF20, 0xFF);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF21, 0x00);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF22, 0x00);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF23, 0xBF);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF24, 0x77);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF25, 0xF3);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF26, 0xF1);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF40, 0x91);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF42, 0x00);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF43, 0x00);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF45, 0x00);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF47, 0xFC);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF48, 0xFF);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF49, 0xFF);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF4A, 0x00);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFF4B, 0x00);
    WRITE_REGISTER_DIRECT(&gameboy->memory, 0xFFFF, 0x00);
}