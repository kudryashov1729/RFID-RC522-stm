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
uint8_t addr1, card_id[5];
volatile uint8_t val_1 = 1, card_is_here = 0;
 

void led_init(void)
{
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOG);
  LL_GPIO_SetPinMode(GPIOG, LL_GPIO_PIN_13, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(GPIOG, LL_GPIO_PIN_14, LL_GPIO_MODE_OUTPUT);
}
  
void main()
{
  //Initialize MFRC522 RFID
  TM_MFRC522_Init();
  led_init();

  addr1 = 0x37;
  val_1 = TM_MFRC522_ReadRegister( addr1);
  addr1 = 0x35;
  val_1 = TM_MFRC522_ReadRegister( addr1);
  
  
  uint8_t result;
  
  while(1) {
    // Check card
    result = TM_MFRC522_Check(card_id);
    if (result == MI_OK)
    {
      card_is_here = 1;
      LL_GPIO_SetOutputPin(GPIOG, LL_GPIO_PIN_14);
    }
    else  
    {
      card_is_here = 0;
      LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_14);
    }
    
    // Check device
    addr1 = 0x37;
    val_1 = TM_MFRC522_ReadRegister( addr1);
    addr1 = 0x35;
    val_1 = TM_MFRC522_ReadRegister( addr1);
    if (val_1 == 0xEA)
    {
      LL_GPIO_SetOutputPin(GPIOG, LL_GPIO_PIN_13);
    }
    else
    {
      LL_GPIO_ResetOutputPin(GPIOG, LL_GPIO_PIN_13);
    }
    
  }
};


