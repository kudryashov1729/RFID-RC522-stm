#include "stm32f429xx.h"

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_gpio.h"
#include "tm_stm32f4_mfrc522.h"

//GPIO SETTING
        //  AF5 SPI1_NSS  PA4
        //  AF5 SPI1_SCK  PA5
        //  AF5 SPI1_MISO PA6
        //  AF5 SPI1_MOSI PA7
  uint8_t addr1;
  volatile uint8_t val_1 = 1;
void main()
{
  //Initialize MFRC522 RFID
  TM_MFRC522_Init();

  addr1 = 0x37;
  val_1 = TM_MFRC522_ReadRegister( addr1);
  

  while(1){
    }
};


