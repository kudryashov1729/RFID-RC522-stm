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
void display_print(TM_MFRC522_Status_t status_card_detected, TM_MFRC522_Status_t auth, TM_MFRC522_Status_t read, uint8_t * id, uint8_t block, uint8_t * data );
uint8_t card_id[5];

uint8_t _blockAddr;
uint8_t Data [18];
uint8_t DataR [20];
TM_MFRC522_Status_t status_read;
TM_MFRC522_Status_t status_auth;
TM_MFRC522_Status_t status_test_write;
TM_MFRC522_Status_t result;
volatile uint32_t x;

void led_init(void)
{
  GPIO_InitTypeDef GPIOG_Init_led;
  
  GPIOG_Init_led.Pin = (GPIO_PIN_13 | GPIO_PIN_14);
  GPIOG_Init_led.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOG, &GPIOG_Init_led);
}
  
void main()
{
 
  HAL_Init();
  //Appropriate clock configuration
  SystemClock_Config();
  //Display initialization
  BSP_LCD_Init();
  BSP_LCD_LayerDefaultInit(LCD_FOREGROUND_LAYER, LCD_FRAME_BUFFER);
  BSP_LCD_SelectLayer(LCD_FOREGROUND_LAYER);
  BSP_LCD_DisplayOn();
  
  
  TM_MFRC522_Init(); //Initialize MFRC522 RFID
  led_init();
  x = BSP_LCD_GetXSize();
  volatile uint32_t y = BSP_LCD_GetYSize();

  BSP_LCD_Clear(LCD_COLOR_WHITE);//Clear display
  BSP_LCD_SetBackColor(LCD_COLOR_WHITE);//Choose background color
  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);//Set work color, not only for text
  BSP_LCD_DisplayStringAt(0, LINE(0), "NO CARD!", CENTER_MODE);

  uint8_t CardID[4] = {0xD9, 0xC1, 0x00, 0xA3};
  
  uint8_t Sectorkey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  _blockAddr = 1;
  Data[0] = 0x17;
  Data[1] = 0x29;
  for(int i = 0; i < 14; i++){
    Data[i+2] = 0x00;
  }
  status_auth = MI_ERR;
  status_test_write= MI_ERR;
  status_read = MI_ERR;
  TM_MFRC522_Status_t status_memory;
  
  while(1) {
    // Check card
    status_memory  = result;
    result = TM_MFRC522_Check(card_id);
    if (result == MI_OK)
    {      
      HAL_GPIO_WritePin ( GPIOG, GPIO_PIN_13, GPIO_PIN_SET);
      TM_MFRC522_SelectTag(card_id);
      status_auth = TM_MFRC522_Auth( PICC_AUTHENT1A, _blockAddr, Sectorkey, card_id); //authorizate card
      if(status_auth == MI_OK){ //if authorizated
        status_read = TM_MFRC522_Read( _blockAddr, DataR);
        /*
        status_test_write = TM_MFRC522_Write( _blockAddr, Data);
        if( status_test_write == MI_OK) {
          status_test_read = TM_MFRC522_Read( _blockAddr, DataR);
        }*/
      }
    }
    else  
    {
      HAL_GPIO_WritePin ( GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);
    }
    if( !(status_memory == result) ){
      display_print(result, status_auth, status_read, card_id, _blockAddr, DataR);
    }
  }
};

void display_print(TM_MFRC522_Status_t status_card_detected, TM_MFRC522_Status_t auth, TM_MFRC522_Status_t read, uint8_t * id, uint8_t block, uint8_t * data ){
  
  if( status_card_detected == MI_OK) {
    BSP_LCD_DisplayStringAt(0, LINE(0), "DETECTED!", CENTER_MODE);
    //LINE 2 PRINT UID
    char str[15];
    sprintf(str, "Card UID:");
    BSP_LCD_DisplayStringAtLine(1, str);
    BSP_LCD_ClearStringLine(2);
    sprintf(str, "%X %X %X %X", id[0], id[1], id[2], id[3]);
    BSP_LCD_DisplayStringAt(0, LINE(2), str, CENTER_MODE);
    if(auth == MI_OK){
      sprintf(str, "Data Block %d:", block);
      BSP_LCD_ClearStringLine(4);
      BSP_LCD_DisplayStringAt(0, LINE(4), str, LEFT_MODE);
      for(int i = 5; i < 9; i++){
        BSP_LCD_ClearStringLine(i);
      }
      if(read == MI_OK){
        for(int i = 0; i < 4; i++){
          BSP_LCD_ClearStringLine(i + 5);
          for(int k = 0; k < 4; k ++ ){
            sprintf(str, "%X", data[ (i * 4) + k]);
            BSP_LCD_DisplayStringAt(240/4 * k, LINE(5 + i), str, LEFT_MODE);
          }
        }
      }else{
        for(int i = 4; i < 11; i++){
          BSP_LCD_ClearStringLine(i);
        } 
        BSP_LCD_DisplayStringAt(0, LINE(4), "Fail to", LEFT_MODE);
        BSP_LCD_DisplayStringAt(0, LINE(5), "read data.", LEFT_MODE);
      }
    }else{
      for(int i = 4; i < 11; i++){
        BSP_LCD_ClearStringLine(i);
      } 
      BSP_LCD_DisplayStringAt(0, LINE(4), "No access to", LEFT_MODE);
      BSP_LCD_DisplayStringAt(0, LINE(5), "card data.", LEFT_MODE);
      BSP_LCD_DisplayStringAt(0, LINE(6), "Incorrect key.", LEFT_MODE);
    }
  }
  else{
    BSP_LCD_ClearStringLine(0);
    BSP_LCD_DisplayStringAt(0, LINE(0), "NO CARD!", CENTER_MODE);
  }
}

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


