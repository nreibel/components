#include "spi_cfg.h"
#include "gpio.h"

GPIO Spi_SCK  = GPIO_SCK;
GPIO Spi_MISO = GPIO_MISO;
GPIO Spi_MOSI = GPIO_MOSI;

GPIO SlaveSelect_Pins[SPI_NUMBER_OF_SLAVES] =
{
    [Spi_Slave_TFT] = GPIO_D8
};
