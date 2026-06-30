#include <cstdio>
#include "chip8.hpp"
#include <thread>
#include <chrono>

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

    // Todo : swap with a bool running
    while (1)
    {
        for (int i = 0; i < 10; i++)
        {
            cpu.cycle();
        }
        cpu.tick_timers();

        if (cpu.draw_flag)
        {
            // Todo : Temp display to replace
            std::printf("\x1b[H");
            for (int y = 0; y < 32; y++)
            {
                for (int x = 0; x < 64; x++)
                {
                    std::putchar(cpu.display[y * 64 + x] ? '#' : ' ');
                }
                std::putchar('\n');
            }
            std::fflush(stdout);
            cpu.draw_flag = false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // approx 60fps
    }
    return 0;
}