#ifndef _USER_REGISTER_ARRAY_H_
#define _USER_REGISTER_ARRAY_H_

#define REG_SOP  0xAA55CA5A
#define REG_HDR1 0x4EADE500
#define REG_EOP  0xBADC0FEE

uint32_t crc_init();
uint32_t crc_update(uint32_t crc, const void *data, size_t data_len);
uint32_t crc_finalize(uint32_t crc);
bool parity8(uint8_t data);
void ReadReg(int regAddr, uint32_t *data);
void singleWriteReg(uint32_t regAddr, uint32_t regContent);
int writeReg(uint32_t * pktContent, int pktLen);

#endif
