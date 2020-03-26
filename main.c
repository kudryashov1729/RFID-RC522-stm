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
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
  
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
  
  
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_7, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_6, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_7, LL_GPIO_AF_5);
  LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_6, LL_GPIO_AF_5);
  LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_4, LL_GPIO_AF_5);
  LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_5,LL_GPIO_AF_5);
  
  
  //SPI SETTING
  LL_SPI_SetMode(SPI1, LL_SPI_MODE_MASTER);
  LL_SPI_SetClockPhase(SPI1, LL_SPI_PHASE_1EDGE); // CPHA = 0
  LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_LOW); // CPOL = 0
  LL_SPI_SetBaudRatePrescaler(SPI1,  LL_SPI_BAUDRATEPRESCALER_DIV256);//??????????
  LL_SPI_SetTransferBitOrder(SPI1,LL_SPI_MSB_FIRST);
  LL_SPI_SetTransferDirection(SPI1, LL_SPI_FULL_DUPLEX);
  LL_SPI_SetDataWidth(SPI1, LL_SPI_DATAWIDTH_8BIT);
  LL_SPI_SetNSSMode (SPI1, LL_SPI_NSS_HARD_OUTPUT);
  
  LL_SPI_EnableIT_RXNE(SPI1);
  LL_SPI_Enable(SPI1);
  k = 0;
  
  __NVIC_EnableIRQ(SPI1_IRQn);
  

  //37h = 110111b
  //11101110b = EEh
  int data = 0xEE;
  LL_SPI_TransmitData8(SPI1, data);
  for(int k = 0; k < 100000; k++);
  data = 0x01;
  LL_SPI_TransmitData8(SPI1, data);
  
  while(1){
//    if(k < 16){
//      data = 0x76;
//      LL_SPI_TransmitData8(SPI1, k);
//      data = 0x00;
//      LL_SPI_TransmitData8(SPI1, data);
//    }
  };
}

void SPI1_IRQHandler() {
  resived_data[k] = LL_SPI_ReceiveData8(SPI1);
  k++;
}
