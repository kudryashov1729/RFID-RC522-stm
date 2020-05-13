#include "stm32f429xx.h"

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_gpio.h"
#include "tm_stm32f4_mfrc522.h"

#include "main.h"
#include "stm32f429i_discovery_lcd.h"
#include "stdio.h"


static void SystemClock_Config(void);

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
  /**
  HAL_Init();
  //Appropriate clock configuration
  SystemClock_Config();
  //Display initialization
  BSP_LCD_Init();
  
  BSP_LCD_LayerDefaultInit(LCD_FOREGROUND_LAYER, LCD_FRAME_BUFFER);
  BSP_LCD_SelectLayer(LCD_FOREGROUND_LAYER);
  BSP_LCD_DisplayOn();
  */
  
  TM_MFRC522_Init(); //Initialize MFRC522 RFID
  led_init();

  //TM_MFRC522_WriteRegister(MFRC522_REG_T_MODE, 0x8D);
  addr1 = 0x37;
  val_1 = TM_MFRC522_ReadRegister( addr1);
  addr1 = 0x35;
  val_1 = TM_MFRC522_ReadRegister( addr1);
  addr1 = MFRC522_REG_SERIALSPEED; //0x1F
  val_1 = TM_MFRC522_ReadRegister( addr1);
  addr1 = MFRC522_REG_T_MODE; //0x2A
  val_1 = TM_MFRC522_ReadRegister( addr1);
  
  uint8_t CardID[5];
  
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
  }
};

static void SystemClock_Config(void)
{
RCC_ClkInitTypeDef RCC_ClkInitStruct;
RCC_OscInitTypeDef RCC_OscInitStruct;
/* Enable Power Control clock */
__HAL_RCC_PWR_CLK_ENABLE();
/* The voltage scaling allows optimizing the power consumption when the device is
clocked below the maximum system frequency, to update the voltage scaling value
regarding system frequency refer to product datasheet. */
__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
/* Enable HSE Oscillator and activate PLL with HSE as source */
RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
RCC_OscInitStruct.HSEState = RCC_HSE_ON;
RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
RCC_OscInitStruct.PLL.PLLM = 8;
RCC_OscInitStruct.PLL.PLLN = 360;
RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
RCC_OscInitStruct.PLL.PLLQ = 7;
HAL_RCC_OscConfig(&RCC_OscInitStruct);
/* Activate the Over-Drive mode */
HAL_PWREx_EnableOverDrive();
/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
clocks dividers */
RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}


