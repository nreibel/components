#ifndef __I2C_API_I2C_H__
#define __I2C_API_I2C_H__

#include "types.h"

Std_ReturnType I2C_Master_Init();

Std_ReturnType I2C_Master_ReadSync(uint8_t addr, buffer_t buffer, int writeLen, int readLen, int delay);
Std_ReturnType I2C_Master_WriteSync(uint8_t addr, buffer_t buffer, int len);

Std_ReturnType I2C_Master_ReadRegister(uint8_t addr, uint8_t reg, uint8_t *val);
Std_ReturnType I2C_Master_WriteRegister(uint8_t addr, uint8_t reg, uint8_t val);

#endif // __I2C_API_I2C_H__
