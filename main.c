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
  uint8_t addr = 0x37;
  volatile uint8_t val_1 = 0;
void main()
{
  //Initialize MFRC522 RFID
  TM_MFRC522_Init();
  //Recognized card ID
  uint8_t CardID[5];
  //My cards id
  //I read them with program below, and write this here
  uint8_t MyID[5] = {
      0x43, 0xdc, 0x52, 0xb6, 0x7b    //My card on my keys
  };
  
  val_1 = TM_MFRC522_ReadRegister( addr);
  

  while(1){
    if (TM_MFRC522_Check(CardID) == MI_OK) {
      //CardID is valid
      
      //Check if this is my card
      if (TM_MFRC522_Compare(CardID, MyID) == MI_OK) {
        
      } else {
        
      }
      //Print on LCD
      
    } else {
      //Some printing to delete content
      
    }
  }

};


