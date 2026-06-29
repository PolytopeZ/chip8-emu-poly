#include "chip8.hpp"
#include <cstdio>
#include <cstring>

void Chip8::reset()
{
    std::memset(memory, 0, sizeof(memory));
    std::memset(registers, 0, sizeof(registers));
    stackPointer = 0;
    index = 0;
    programCounter = MEMORY_BEGIN;
    std::memset(stack, 0, sizeof(stack));
    delay = 0;
    sound = 0;
    std::memset(display, false, sizeof(display));
    std::memset(keys, false, sizeof(keys));
    draw_flag = false;

    // TODO: add fonts + loading
}

bool Chip8::load_rom(const char *path)
{
    std::FILE *f = std::fopen(path, "rb");
    if (!f)
    {
        return false;
    }
    std::fread(memory + MEMORY_BEGIN, 1, sizeof(memory) - MEMORY_BEGIN, f);
    std::fclose(f);
    return true;
}