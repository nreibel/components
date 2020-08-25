#include "os.h"
#include "app.h"
#include "serial.h"
#include "bits.h"
#include "gpio.h"
#include "spi.h"
#include "st7735.h"
#include "hc595.h"
#include "pwm.h"
#include "eeprom.h"
#include "i2c_slave.h"

#include "nyan_cat.xpm"

#define DEVICE_ID_LENGTH 16

typedef struct {
    byte deviceId[DEVICE_ID_LENGTH];
    byte screenInverted;
    byte screenOffsetX;
    byte screenOffsetY;
} DeviceConfig_t;

bool eepromUpdated = FALSE;
bool transmitting = FALSE;

DeviceConfig_t devCfg = {0};

void I2C_Slave_StartCallback()
{
    transmitting = TRUE;
}

void I2C_Slave_StopCallback()
{
    transmitting = FALSE;
}

byte I2C_Slave_TransmitCallback(int offset)
{
    if (offset < DEVICE_ID_LENGTH)
        return devCfg.deviceId[offset];

    if (offset == 0x10)
        return devCfg.screenInverted;

    if (offset == 0x11)
        return devCfg.screenOffsetX;

    if (offset == 0x12)
        return devCfg.screenOffsetY;

    return 0xFF;
}

void I2C_Slave_ReceiveCallback(int offset, byte data)
{
    if (offset < DEVICE_ID_LENGTH)
        devCfg.deviceId[offset] = data;

    if (offset == 0x10)
        devCfg.screenInverted = data;

    if (offset == 0x11)
        devCfg.screenOffsetX = data;

    if (offset == 0x12)
        devCfg.screenOffsetY = data;

    eepromUpdated = TRUE;
}

// App entry point
void App_Init()
{
    // Init communication
    I2C_Slave_Init(0x10);
    Serial_Init();
    Spi_Init();

    // Debug dump EEPROM data over Serial
    EEPROM_DumpEEPROM(0, 0x20, 16);

    // Read app config from EEPROM
    EEPROM_SyncRead(0, &devCfg, sizeof(DeviceConfig_t));

    // Read calibration data
    if (devCfg.screenOffsetX != 0xFF)
    {
        ST7735_CalibrationData.Offset_X = devCfg.screenOffsetX;
    }

    if (devCfg.screenOffsetY != 0xFF)
    {
        ST7735_CalibrationData.Offset_Y = devCfg.screenOffsetY;
    }

    if (devCfg.screenInverted != 0xFF)
    {
        ST7735_CalibrationData.InvertScreen = devCfg.screenInverted;
    }

    // Init TFT
    Spi_Configure(SPI_CLOCK_DIV_2, SPI_MODE_0);
    Spi_EnableSlave(SPI_Slave_TFT);
    ST7735_Init();
    ST7735_SetBackgroundColor(ST7735_COLOR_PINK);
    ST7735_ClearScreen();
    ST7735_DrawXPM(nyan_cat, 0, 20, 4);
    ST7735_DrawChars(2, 2, &devCfg.deviceId, DEVICE_ID_LENGTH, ST7735_COLOR_BLACK);
    Spi_DisableSlave(SPI_Slave_TFT);

    // Init HC595
    HC595_Init();

    // Backlight ON
    PWM_Init(PWM_6, FALSE);
    PWM_SetPWM(PWM_6, 0x80);

    // Set up tasks
    Os_SetupTask(Timer_MainTask, 1000, &Task_MainCyclic, NULL_PTR);
}

// Main task
Std_ReturnType Task_MainCyclic(void* data)
{
    UNUSED(data);

    static word out = 1;
    static bool state = TRUE;

    HC595_WriteWord(out++);

    if (state) Serial_PrintLine("Tick");
    else Serial_PrintLine("Tock");
    state = !state;

    if (eepromUpdated && !transmitting)
    {
        EEPROM_SyncWrite(0, &devCfg, sizeof(DeviceConfig_t));
        eepromUpdated = FALSE;
    }

    return Status_OK;
}
