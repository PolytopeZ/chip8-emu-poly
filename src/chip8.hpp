#pragma once
#include <cstdint>

const uint16_t MEMORY_BEGIN = 0x200;

class Chip8
{
public:
    uint8_t memory[4096]{};  // RAM memory of 4096 bytes
    uint8_t registers[16]{}; // Registers V0..VF
    uint8_t stackPointer = 0;
    uint16_t index = 0;
    uint16_t programCounter = MEMORY_BEGIN;
    uint16_t stack[16]{};
    uint8_t delay = 0;
    uint8_t sound = 0;
    bool display[64 * 32]{}; // monochrome display on/off
    bool keys[16]{};         // keys 0..F
    bool draw_flag = false;

    void reset();
    bool load_rom(const char *path);
};