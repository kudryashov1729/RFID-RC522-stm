#include "stm32f429xx.h"

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_gpio.h"

volatile int resived_data[16];
volatile int k;

void main()
{
  //CLOCKING
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_APB2_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);
  
  //GPIO SETTING
  //  AF5 SPI1_NSS  PA4
  //  AF5 SPI1_SCK  PA5
  //  AF5 SPI1_MISO PA6
  //  AF5 SPI1_MOSI PA7
  
  //  AF5 SPI1_NSS  PB12
  //  AF5 SPI1_SCK  PB13
  //  AF5 SPI1_MISO PB14
  //  AF5 SPI1_MOSI PB15
  
  //  AF6 SPI3_NSS  PA15
  //  AF6 SPI3_SCK  PB3
  //  AF6 SPI3_MISO PB4
  //  AF6 SPI3_MOSI PB5
  
  
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_15, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_3, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_4, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_5, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_15, LL_GPIO_AF_6);
  LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_3, LL_GPIO_AF_6);
  LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_4, LL_GPIO_AF_6);
  LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_5,LL_GPIO_AF_6);
  
  
  //SPI SETTING
  LL_SPI_SetMode(SPI3, LL_SPI_MODE_MASTER);
  LL_SPI_SetClockPhase(SPI3, LL_SPI_PHASE_1EDGE); // CPHA = 0
  LL_SPI_SetClockPolarity(SPI3, LL_SPI_POLARITY_HIGH); // CPOL = 0
  LL_SPI_SetBaudRatePrescaler(SPI3,  LL_SPI_BAUDRATEPRESCALER_DIV256);//??????????
  LL_SPI_SetTransferBitOrder(SPI3,LL_SPI_MSB_FIRST);
  LL_SPI_SetTransferDirection(SPI3, LL_SPI_FULL_DUPLEX);
  LL_SPI_SetDataWidth(SPI3, LL_SPI_DATAWIDTH_8BIT);
  LL_SPI_SetNSSMode (SPI3, LL_SPI_NSS_HARD_OUTPUT);
  
  LL_SPI_EnableIT_RXNE(SPI3);
  LL_SPI_Enable(SPI3);
  k = 0;
  
  __NVIC_EnableIRQ(SPI3_IRQn);
  

  //37h = 110111b
  //11101110b = EEh
  int data = 0xEE;
  LL_SPI_TransmitData8(SPI3, data);
  for(int k = 0; k < 100000; k++);
  data = 0x00;
  LL_SPI_TransmitData8(SPI3, data);
  
  while(1){
//    if(k < 16){
//      data = 0x76;
//      LL_SPI_TransmitData8(SPI1, k);
//      data = 0x00;
//      LL_SPI_TransmitData8(SPI1, data);
//    }
  };
}

void SPI3_IRQHandler() {
  resived_data[k] = LL_SPI_ReceiveData8(SPI3);
  k++;
}
