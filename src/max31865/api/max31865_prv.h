#ifndef SRC_MAX31865_API_MAX31865_PRV_H_
#define SRC_MAX31865_API_MAX31865_PRV_H_

#include "max31865.h"
#include "max31865_cfg.h"

#define MAX31865_REG_CONFIG       0x00
#define MAX31865_REG_RTD_MSB      0x01
#define MAX31865_REG_RTD_LSB      0x02
#define MAX31865_REG_HFT_MSB      0x03
#define MAX31865_REG_HFT_LSB      0x04
#define MAX31865_REG_LFT_MSB      0x05
#define MAX31865_REG_LFT_LSB      0x06
#define MAX31865_REG_FAULT_STATUS 0x07

#define MAX31865_READ  0x00
#define MAX31865_WRITE 0x80

#define MAX31865_ADC_RESOLUTION (1ULL << 15)

#endif // SRC_MAX31865_API_MAX31865_PRV_H_
