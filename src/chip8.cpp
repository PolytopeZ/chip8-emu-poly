#include "chip8.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>

Chip8::Handler Chip8::main_table[16];
Chip8::Handler Chip8::alu_table[16];
Chip8::Handler Chip8::e_table[256];
Chip8::Handler Chip8::f_table[256];

void Chip8::init_tables()
{
    // default to unknown
    for (auto &h : main_table)
    {
        h = &Chip8::op_unknown;
    }
    for (auto &h : alu_table)
    {
        h = &Chip8::op_unknown;
    }
    for (auto &h : e_table)
    {
        h = &Chip8::op_unknown;
    }
    for (auto &h : f_table)
    {
        h = &Chip8::op_unknown;
    }

    // main_table
    main_table[0x0] = &Chip8::op_0_group;
    main_table[0x1] = &Chip8::op_1NNN;
    main_table[0x2] = &Chip8::op_2NNN;
    main_table[0x3] = &Chip8::op_3XNN;
    main_table[0x4] = &Chip8::op_4XNN;
    main_table[0x5] = &Chip8::op_5XY0;
    main_table[0x6] = &Chip8::op_6XNN;
    main_table[0x7] = &Chip8::op_7XNN;
    main_table[0x8] = &Chip8::op_8_group;
    main_table[0x9] = &Chip8::op_9XY0;
    main_table[0xA] = &Chip8::op_ANNN;
    main_table[0xB] = &Chip8::op_BNNN;
    main_table[0xC] = &Chip8::op_CXNN;
    main_table[0xD] = &Chip8::op_DXYN;
    main_table[0xE] = &Chip8::op_E_group;
    main_table[0xF] = &Chip8::op_F_group;

    // 8XY_
    alu_table[0x0] = &Chip8::op_8XY0;
    alu_table[0x1] = &Chip8::op_8XY1;
    alu_table[0x2] = &Chip8::op_8XY2;
    alu_table[0x3] = &Chip8::op_8XY3;
    alu_table[0x4] = &Chip8::op_8XY4;
    alu_table[0x5] = &Chip8::op_8XY5;
    alu_table[0x6] = &Chip8::op_8XY6;
    alu_table[0x7] = &Chip8::op_8XY7;
    alu_table[0xE] = &Chip8::op_8XYE;

    // 0xEX__
    e_table[0x9E] = &Chip8::op_EX9E;
    e_table[0xA1] = &Chip8::op_EXA1;

    // 0xFX__
    f_table[0x07] = &Chip8::op_FX07;
    f_table[0x0A] = &Chip8::op_FX0A;
    f_table[0x15] = &Chip8::op_FX15;
    f_table[0x18] = &Chip8::op_FX18;
    f_table[0x1E] = &Chip8::op_FX1E;
    f_table[0x29] = &Chip8::op_FX29;
    f_table[0x33] = &Chip8::op_FX33;
    f_table[0x55] = &Chip8::op_FX55;
    f_table[0x65] = &Chip8::op_FX65;
}

void Chip8::reset()
{
    // Setup tables
    static bool tables_ready = false;
    if (!tables_ready)
    {
        init_tables();
        tables_ready = true;
    }

    // Reset everthing from Chip8
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
    opcode = 0;

    // Init fonts
    std::memcpy(memory + FONT_BEGIN, font, sizeof(font));
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

void Chip8::op_unknown()
{
    std::fprintf(stderr, "Unknown opcode %04X at PC=%03X\n", opcode, programCounter - 2);
    // Todo : maybe change with a bool running instead of exit
    std::exit(1);
}

void Chip8::tick_timers()
{
    if (delay > 0)
    {
        delay--;
    }
    if (sound > 0)
    {
        sound--;
    }
}

void Chip8::cycle()
{
    // fetch
    opcode = (memory[programCounter] << 8) | memory[programCounter + 1];
    programCounter += 2;

    // decode + execute
    uint16_t high_nibble = (opcode & 0xF000) >> 12;
    (this->*main_table[high_nibble])();
}

void Chip8::op_0_group()
{
    switch (opcode & 0x00FF)
    {
    case 0xE0:
        op_00E0();
        break;
    case 0xEE:
        op_00EE();
        break;
    default:
        op_unknown();
        break;
    }
}
void Chip8::op_8_group()
{
    (this->*alu_table[opcode & 0x000F])();
}
void Chip8::op_E_group()
{
    (this->*e_table[opcode & 0x00FF])();
}
void Chip8::op_F_group()
{
    (this->*f_table[opcode & 0x00FF])();
}

// Todo : Code the impl
// 0x0___
void Chip8::op_00E0() {}
void Chip8::op_00EE() {}

// 0x1..7___
void Chip8::op_1NNN() {}
void Chip8::op_2NNN() {}
void Chip8::op_3XNN() {}
void Chip8::op_4XNN() {}
void Chip8::op_5XY0() {}
void Chip8::op_6XNN() {}
void Chip8::op_7XNN() {}

// 0x8___
void Chip8::op_8XY0() {}
void Chip8::op_8XY1() {}
void Chip8::op_8XY2() {}
void Chip8::op_8XY3() {}
void Chip8::op_8XY4() {}
void Chip8::op_8XY5() {}
void Chip8::op_8XY6() {}
void Chip8::op_8XY7() {}
void Chip8::op_8XYE() {}

// 0x9..D___
void Chip8::op_9XY0() {}
void Chip8::op_ANNN() {}
void Chip8::op_BNNN() {}
void Chip8::op_CXNN() {}
void Chip8::op_DXYN() {}

// 0xE___
void Chip8::op_EX9E() {}
void Chip8::op_EXA1() {}

// 0xF___
void Chip8::op_FX07() {}
void Chip8::op_FX0A() {}
void Chip8::op_FX15() {}
void Chip8::op_FX18() {}
void Chip8::op_FX1E() {}
void Chip8::op_FX29() {}
void Chip8::op_FX33() {}
void Chip8::op_FX55() {}
void Chip8::op_FX65() {}