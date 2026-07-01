#include "chip8.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

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
    // Rand seed for op_CXNN
    std::srand(std::time(nullptr));

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
void Chip8::op_00E0()
{
    std::memset(display, false, sizeof(display));
    draw_flag = true;
}

void Chip8::op_00EE()
{
    stackPointer--;
    programCounter = stack[stackPointer];
}

// 0x1..7___
void Chip8::op_1NNN()
{
    programCounter = get_nnn(opcode);
}

void Chip8::op_2NNN()
{
    stack[stackPointer] = programCounter;
    stackPointer++;
    programCounter = get_nnn(opcode);
}

void Chip8::op_3XNN()
{
    if (registers[get_x(opcode)] == get_nn(opcode))
    {
        programCounter += 2;
    }
}

void Chip8::op_4XNN()
{
    if (registers[get_x(opcode)] != get_nn(opcode))
    {
        programCounter += 2;
    }
}

void Chip8::op_5XY0()
{
    if (registers[get_x(opcode)] == registers[get_y(opcode)])
    {
        programCounter += 2;
    }
}

void Chip8::op_6XNN()
{
    registers[get_x(opcode)] = get_nn(opcode);
}

void Chip8::op_7XNN()
{
    registers[get_x(opcode)] += get_nn(opcode);
}

// 0x8___
void Chip8::op_8XY0()
{
    registers[get_x(opcode)] = registers[get_y(opcode)];
}

void Chip8::op_8XY1()
{
    registers[get_x(opcode)] |= registers[get_y(opcode)];
}

void Chip8::op_8XY2()
{
    registers[get_x(opcode)] &= registers[get_y(opcode)];
}

void Chip8::op_8XY3()
{
    registers[get_x(opcode)] ^= registers[get_y(opcode)];
}

void Chip8::op_8XY4()
{
    uint16_t res = registers[get_x(opcode)] + registers[get_y(opcode)];
    registers[get_x(opcode)] = res & 0xFF;
    if (res > 0xFF)
    {
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }
}

void Chip8::op_8XY5()
{
    uint8_t vx = registers[get_x(opcode)];
    uint8_t vy = registers[get_y(opcode)];
    registers[get_x(opcode)] = vx - vy;
    if (vx >= vy)
    {
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }
}

void Chip8::op_8XY6()
{
    uint8_t vy = registers[get_y(opcode)];
    registers[get_x(opcode)] = vy >> 1;
    registers[0xF] = vy & 1;
}

void Chip8::op_8XY7()
{
    uint8_t vx = registers[get_x(opcode)];
    uint8_t vy = registers[get_y(opcode)];
    registers[get_x(opcode)] = vy - vx;
    if (vy >= vx)
    {
        registers[0xF] = 1;
    }
    else
    {
        registers[0xF] = 0;
    }
}

void Chip8::op_8XYE()
{
    uint8_t vy = registers[get_y(opcode)];
    registers[get_x(opcode)] = vy << 1;
    registers[0xF] = (vy >> 7) & 1;
}

// 0x9..D___
void Chip8::op_9XY0()
{
    if (registers[get_x(opcode)] != registers[get_y(opcode)])
    {
        programCounter += 2;
    }
}

void Chip8::op_ANNN()
{
    index = get_nnn(opcode);
}

void Chip8::op_BNNN()
{
    programCounter = get_nnn(opcode) + registers[0x0];
}

void Chip8::op_CXNN()
{
    registers[get_x(opcode)] = (std::rand() & 0xFF) & get_nn(opcode);
}

void Chip8::op_DXYN()
{
    uint8_t x0 = registers[get_x(opcode)] % 64;
    uint8_t y0 = registers[get_y(opcode)] % 32;
    uint8_t n = get_n(opcode);
    registers[0xF] = 0;

    for (uint8_t row = 0; row < n; row++)
    {
        if (y0 + row >= 32)
            break;
        uint8_t sprite_byte = memory[index + row];

        for (uint8_t col = 0; col < 8; col++)
        {
            if (x0 + col >= 64)
                break;
            bool pixel = (sprite_byte >> (7 - col)) & 1;
            size_t idx = (y0 + row) * 64 + (x0 + col);

            if (pixel && display[idx])
                registers[0xF] = 1;
            display[idx] ^= pixel;
        }
    }
    draw_flag = true;
}

// 0xE___
void Chip8::op_EX9E()
{
    // Todo
}

void Chip8::op_EXA1()
{
    // Todo
}

// 0xF___
void Chip8::op_FX07()
{
    registers[get_x(opcode)] = delay;
}

void Chip8::op_FX0A()
{
    // Todo
}

void Chip8::op_FX15()
{
    delay = registers[get_x(opcode)];
}

void Chip8::op_FX18()
{
    sound = registers[get_x(opcode)];
}

void Chip8::op_FX1E()
{
    index += registers[get_x(opcode)];
}

void Chip8::op_FX29()
{
    index = FONT_BEGIN + registers[get_x(opcode)] * 5; // *5 'cause font are 5 bytes
}

void Chip8::op_FX33()
{
    uint8_t v = registers[get_x(opcode)];
    memory[index] = v / 100;
    memory[index + 1] = (v / 10) % 10;
    memory[index + 2] = v % 10;
}

void Chip8::op_FX55()
{
    uint8_t x = get_x(opcode);
    for (uint8_t i = 0; i <= x; i++)
    {
        memory[index + i] = registers[i];
    }
    index += x + 1;
}

void Chip8::op_FX65()
{
    uint8_t x = get_x(opcode);
    for (uint8_t i = 0; i <= x; i++)
    {
        registers[i] = memory[index + i];
    }
    index += x + 1;
}