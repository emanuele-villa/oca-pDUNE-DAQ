#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdbool.h>

void ReadReg(int regAddr, uint32_t *data)

int write_register(uint16_t reg, uint32_t *value);

uint32_t crc_init();
uint32_t crc_update(uint32_t crc, const void *data, size_t data_len);
uint32_t crc_finalize(uint32_t crc);

bool parity8(uint8_t data);
