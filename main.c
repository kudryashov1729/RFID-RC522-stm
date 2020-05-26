#include "stm32f429xx.h"


#include "stm32f4xx_ll_spi.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_rcc.h"

#include "tm_stm32f4_mfrc522.h"

#include "main.h"
#include "stm32f429i_discovery_lcd.h"
#include "stdio.h"


static void SystemClock_Config(void);
void display_print(TM_MFRC522_Status_t status_card_detected, TM_MFRC522_Status_t auth, TM_MFRC522_Status_t read, uint8_t * id, uint8_t block, uint8_t data[64][17] );
uint8_t card_id[5];

volatile uint8_t data_UART;
char uart1_rx_buf[64];
char uart1_rx_bit;

volatile uint8_t _blockAddr;

uint8_t card_read_alldata[64][17];

uint8_t Data [18];
uint8_t DataR [18];
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

void button_init(void)
{
  GPIOA->MODER &= ~(GPIO_MODER_MODER0_0 |GPIO_MODER_MODER0_1); //PAO mode is input
  EXTI->IMR |= EXTI_IMR_MR0; //Interrupt request from line 0 is not masked
  EXTI->RTSR |= EXTI_RTSR_TR0; //Rising trigger enabled (for Event and Interrupt) for input line
  EXTI->FTSR &= ~EXTI_FTSR_TR0; //Falling trigger enabled (for Event and Interrupt) for input line.
  
  NVIC_EnableIRQ(EXTI0_IRQn); // Enables a device specific interrupt in the NVIC interrupt controller.
}


void UART_init( void){
  //CLOCKING
  __HAL_RCC_USART1_CLK_ENABLE();
  
  /*
PA9     USART1_TX       AF7
PA10    USART1_RX       AF7
*/

  
  //GPIO SETTING
  GPIOC->MODER |= GPIO_MODER_MODER8_0; //PC8 mode is output
  GPIOC->ODR |= GPIO_ODR_OD8; //PC8 is set to use as 5V for UART
  GPIO_InitTypeDef GPIO_Init_for_UART;
  GPIO_Init_for_UART.Pin = (GPIO_PIN_10 | GPIO_PIN_9);
  GPIO_Init_for_UART.Mode = GPIO_MODE_AF_PP;
  GPIO_Init_for_UART.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOA, &GPIO_Init_for_UART);
  
  //USART SETTING
  LL_USART_EnableDirectionTx(USART1);
  LL_USART_EnableDirectionRx(USART1);
  LL_USART_SetParity(USART1, LL_USART_PARITY_NONE);   // PARITY - CHETNOST
  LL_USART_SetDataWidth(USART1, LL_USART_DATAWIDTH_8B);
  LL_USART_SetStopBitsLength(USART1, LL_USART_STOPBITS_1);
  //
  LL_RCC_ClocksTypeDef RCCClocks;
  LL_RCC_GetSystemClocksFreq(&RCCClocks);                                       // APB2 clock frequecy will be in RCCClocks.PCLK2_Frequency.
  //
  LL_USART_SetBaudRate(USART1, 90000000, LL_USART_OVERSAMPLING_16, 9600);//Baud Rate = Skorost' peredachi
  LL_USART_EnableIT_RXNE(USART1);                                                         //Enable RX Not Empty Interrupt.
  LL_USART_Enable (USART1);
  
  //NVIC SETTING
  __NVIC_EnableIRQ(USART1_IRQn);
}

void send_to_uart(uint8_t data)
{
 while(!(USART1->SR & USART_SR_TC)); //Transmission is complete
 USART1->DR=data;
}
 
void send_str(char * string)
{
 uint8_t i=0;
 while(string[i]) 
{
  send_to_uart(string[i]);
  i++;
 }
}
void send_str_n(char * string, int n)
{
 for(int i = 0; i < n; i++){
  send_to_uart(string[i]);
 }
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
  button_init();
  UART_init();

  BSP_LCD_Clear(LCD_COLOR_WHITE);//Clear display
  BSP_LCD_SetBackColor(LCD_COLOR_WHITE);//Choose background color
  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);//Set work color, not only for text
  BSP_LCD_DisplayStringAt(0, LINE(0), "NO CARD!", CENTER_MODE);

  uint8_t block_mem = 1;
  
  uint8_t Sectorkey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  _blockAddr = 0;
  
  Data[0] = 0x17;
  Data[1] = 0x29;
  for(int i = 0; i < 14; i++){
    Data[i+2] = 0x00;
  }
  //status_auth = MI_ERR;
  status_test_write= MI_ERR;
  status_read = MI_ERR;
  TM_MFRC522_Status_t status_memory;
  
  int i_block = 0;
  
  while(1) {
        
    // Check card
    result = TM_MFRC522_Check(card_id);
    if (result == MI_OK)
    { 
      HAL_GPIO_WritePin ( GPIOG, GPIO_PIN_13, GPIO_PIN_SET);
      TM_MFRC522_Halt();
      while(result == MI_OK && i_block < 64){
        result = TM_MFRC522_Check(card_id);
        TM_MFRC522_SelectTag(card_id);
        //status_auth = TM_MFRC522_Auth( PICC_AUTHENT1A, i_block, Sectorkey, card_id); //authorizate card
        status_auth == MI_OK;
        if(status_auth == MI_OK){ //if authorizated
          status_read = TM_MFRC522_Read( i_block, DataR);
          if(status_read == MI_OK){
            for(int k = 0; k < 16; k++){
              card_read_alldata[i_block][k] = DataR[k];
            }
            card_read_alldata[i_block][16] = MI_OK;
          }else{ 
            TM_MFRC522_Halt();
            TM_MFRC522_StopCrypto1();
            card_read_alldata[i_block][16] = MI_ERR;// last byte is used to check data read
          } 
        }else{
          TM_MFRC522_Halt();
          TM_MFRC522_StopCrypto1();
          card_read_alldata[i_block][16] = MI_ERR; // last byte is used to check data read
        }
        i_block ++;
      }
      
//      //READ FOR LCD output
//      TM_MFRC522_SelectTag(card_id);
//      status_auth = TM_MFRC522_Auth( PICC_AUTHENT1A, _blockAddr, Sectorkey, card_id); //authorizate card
//      if(status_auth == MI_OK){ //if authorizated 
//        status_read = TM_MFRC522_Read( _blockAddr, DataR);
//        TM_MFRC522_Halt();
//        TM_MFRC522_StopCrypto1();
//        /*
//        status_test_write = TM_MFRC522_Write( _blockAddr, Data);
//        if( status_test_write == MI_OK) {
//          status_test_read = TM_MFRC522_Read( _blockAddr, DataR);
//        }*/
//      }
      
    }
    else  
    {
      HAL_GPIO_WritePin ( GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);
      i_block = 0;
    }
    
  
    
    if(status_memory != result){
      display_print(result, status_auth, status_read, card_id, _blockAddr, card_read_alldata);
      block_mem = _blockAddr;
    }   
    if(block_mem != _blockAddr) {
      status_memory = ~result;
    }else{
      status_memory  = result;
    }
  }
};

void display_print(TM_MFRC522_Status_t status_card_detected, TM_MFRC522_Status_t auth, TM_MFRC522_Status_t read, uint8_t * id, uint8_t block, uint8_t  data[64][17] ){
  if( status_card_detected == MI_OK) {
    char str[15];
    //UART send
    send_str("\n-------------------------------------------\n\r");
    send_str( "Card UID: ");
    sprintf(str, "%X %X %X %X %X\n\r", id[0], id[1], id[2], id[3], id[4]);
    send_str( str);
    send_str("-------------------------------------------\n\r");
    for(int i = 0; i < 64; i++){
      send_str("Block ");
      sprintf(str, "%d: ", i);
      send_str(str);
      if(data[i][16] == MI_ERR){
        send_str("No access to data.\n\r");
      }else{
        for(int k = 0 ; k < 16; k++){
          if(data[i][k] <= 32){ 
            sprintf(str, " [%X]\t", data[i][k]);
            send_str(str);
          }
          else{
            sprintf(str, "%c[%X]\t", data[i][k], data[i][k]);
            send_str(str);
          }
        }
        send_str("\n\r");
      }
    }

    
    BSP_LCD_DisplayStringAt(0, LINE(0), "DETECTED!", CENTER_MODE);
    //LINE 2 PRINT UID
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
            sprintf(str, "%X", data[block][ (i * 4) + k]);
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


