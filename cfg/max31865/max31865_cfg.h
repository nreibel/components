#ifndef SRC_MAX31865_CFG_MAX31865_CFG_H_
#define SRC_MAX31865_CFG_MAX31865_CFG_H_

// A and B coeffs
#define MAX31865_RTD_A MAX31865_RTD_A_DIN43760
#define MAX31865_RTD_B MAX31865_RTD_B_DIN43760

// RTD resistance in Ohms at 0°C (100 for PT100, 1000 for PT1000)
#define MAX31865_RTD_RES_0 100

// Reference resistor in Ohms
#define MAX31865_RES_REF 430

#endif // SRC_MAX31865_CFG_MAX31865_CFG_H_
