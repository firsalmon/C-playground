#include <stdint.h>

typedef struct {
  uint8_t memory[4096]; // RAM, программа с 0x200
  uint8_t V[16];        // V0..VF, VF = флаг переноса
  uint16_t I;           // адресный регистр
  uint16_t pc;          // счётчик, старт 0x200
  uint16_t stack[16];   // стек возвратов
  uint8_t sp;
  uint8_t delay, sound;     // таймеры
  uint8_t display[64 * 32]; // 1 бит на пиксель
  uint8_t keypad[16];
} chip8_t;
