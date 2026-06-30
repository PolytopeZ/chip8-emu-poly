#pragma once
#include <cstdint>

const uint16_t MEMORY_BEGIN = 0x200;
const uint16_t FONT_BEGIN = 0x50;

// clang-format off
constexpr uint8_t  get_x  (uint16_t op) { return (op & 0x0F00) >> 8; }
constexpr uint8_t  get_y  (uint16_t op) { return (op & 0x00F0) >> 4; }
constexpr uint8_t  get_n  (uint16_t op) { return  op & 0x000F; }
constexpr uint8_t  get_nn (uint16_t op) { return  op & 0x00FF; }
constexpr uint16_t get_nnn(uint16_t op) { return  op & 0x0FFF; }

static const uint8_t font[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80,  // F
};
// clang-format on

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
    void tick_timers();
    void cycle();

private:
    uint16_t opcode = 0;

    using Handler = void (Chip8::*)();

    // Dispatch tables
    static Handler main_table[16]; // index = (opcode & 0xF000) >> 12
    static Handler alu_table[16];  // 8XY_ : index = opcode & Ox00F
    static Handler e_table[256];   // EX__ : index = opcode & 00FF
    static Handler f_table[256];   // FX__ : index = opcode & O0FF
    void init_tables();

    // Dispatch sub tables
    void op_0_group();
    void op_8_group();
    void op_E_group();
    void op_F_group();
    void op_unknown();

    // Instructions impl
    // 0x0__
    void op_00E0(); // CLS
    void op_00EE(); // RET

    // 0x1..7___
    void op_1NNN(); // Jump TO NNN
    void op_2NNN(); // Call NNN
    void op_3XNN(); // Skip if Vx = NN
    void op_4XNN(); // Skip if Vx != NN
    void op_5XY0(); // Skip if Vx = Vy
    void op_6XNN(); // Vx = NN
    void op_7XNN(); // Vx += NN

    // 0x8___
    void op_8XY0(); // Vx = Vy
    void op_8XY1(); // Vx |= Vy
    void op_8XY2(); // Vx &= Vy
    void op_8XY3(); // Vx ^= Vy
    void op_8XY4(); // Vx += Vy
    void op_8XY5(); // Vx -= Vy
    void op_8XY6(); // VX = Vy >> 1
    void op_8XY7(); // Vx = Vy - Vx
    void op_8XYE(); // VX = Vy << 1

    // 0x9..D___
    void op_9XY0(); // Skip if Vx != Vy
    void op_ANNN(); // index = NNN
    void op_BNNN(); // Jump yo NNN+V0
    void op_CXNN(); // Vx = rand
    void op_DXYN(); // Drawn Vx Vy with N bytes

    // 0xE___
    void op_EX9E(); // Skip if Vx = key pressed
    void op_EXA1(); // Skip if Vx = key not pressed

    // 0xF___
    void op_FX07(); // Vx = delay timer
    void op_FX0A(); // Vx = key pressed
    void op_FX15(); // Delay timer = Vx
    void op_FX18(); // Sound timer = Vx
    void op_FX1E(); // Index += Vx
    void op_FX29(); // I = location of sprite for digit Vx
    void op_FX33(); // Split Vx into its decimal digits and store
    void op_FX55(); // Dump V0..VX at mem[index]
    void op_FX65(); // Load starting at mem[index] into V0..VX
};