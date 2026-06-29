#include <cstdio>
#include "chip8.hpp"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::fprintf(stderr, "usage: %s <rom.ch8>\n", argv[0]);
        return 1;
    }

    Chip8 cpu;
    cpu.reset();

    if (!cpu.load_rom(argv[1]))
    {
        std::fprintf(stderr, "failed to load %s\n", argv[1]);
        return 1;
    }

    // TODO : debug remove after
    uint16_t op = (cpu.memory[MEMORY_BEGIN] << 8) | cpu.memory[MEMORY_BEGIN + 1];
    std::printf(("first op: %04X\n"), op);
    std::printf("font[0]=%02X, font[1]=%02X\n", cpu.memory[FONT_BEGIN], cpu.memory[FONT_BEGIN + 1]);
    return 0;
}