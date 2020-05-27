/**
  ******************************************************************************
  * @file    Templates/Src/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

extern volatile uint8_t block_Addr_to_write;

/*
void EXTI0_IRQHandler(){
  EXTI->PR |= EXTI_PR_PR0;
  if( GPIOA->IDR  & GPIO_IDR_ID0 ) // if PA0 is set
    {
      GPIOG->ODR |= GPIO_ODR_OD14;
    }
    else
    {
      GPIOG->ODR &=~ GPIO_ODR_OD14; 
    }
  block_Addr_to_write = (block_Addr_to_write + 1) % 4;
}*/

#include "stm32f4xx_ll_usart.h"
#include "string.h"
extern volatile uint8_t data_UART;
extern char uart1_rx_buf[64];
extern char uart1_rx_bit;
extern void send_to_uart(uint8_t data);
extern void send_str(char * string);
extern uint8_t Sectorkey[6];
extern uint8_t select_mode;
extern uint8_t Data [18];

int counter;

void USART1_IRQHandler()
{
  //LL_USART_ReceiveData8(USART1);
  if (LL_USART_IsActiveFlag_RXNE(USART1)){ //Check if the USART Read Data Register Not Empty Flag is set or not.
    data_UART = LL_USART_ReceiveData8(USART1);
    if(data_UART == 0x7F) 
    {
      if(uart1_rx_bit != 0) uart1_rx_bit = uart1_rx_bit - 1;
    }
    else
    {
      uart1_rx_buf[uart1_rx_bit]= data_UART;
      uart1_rx_bit = (uart1_rx_bit + 1) % 64;
    }
    
    while(!(USART1->SR & USART_SR_TC)); //Transmission is complete
    LL_USART_TransmitData8(USART1, data_UART); //Write in Transmitter Data Register
    
    switch(select_mode){
    case SET_KEY:
      if(data_UART == '\r')
      {
        send_str("\n\r");
        uart1_rx_buf[uart1_rx_bit - 1]= 0;
        uart1_rx_bit=0;
        int i = 0;
        Sectorkey[counter] = 0;
        while(uart1_rx_buf[i])
        {
          Sectorkey[counter] = Sectorkey[counter] * 10 + (uart1_rx_buf[i] - '0');
          i++;
        }
        if(counter > 4 )
        {
          counter = 0;
          select_mode = READ_16SECTORS_WITH_KEY;
          send_str("Key [ ");
          char str[25];
          sprintf(str, "%X %X %X %X %X %X ] is set.\n\r", Sectorkey[0], Sectorkey[1], Sectorkey[2], Sectorkey[3], Sectorkey[4], Sectorkey[5]);
          send_str( str);
        }
        else
        {
          counter = counter + 1;
        }
      }
      break;
    case SET_BLOCK:
      if(data_UART == '\r')
      {
        send_str("\n\r");
        uart1_rx_buf[uart1_rx_bit - 1]= 0;
        uart1_rx_bit=0;
        if(counter == 0 )
        {
          int i = 0;
          block_Addr_to_write = 0;
          while(uart1_rx_buf[i])
          {
            block_Addr_to_write = block_Addr_to_write * 10 + (uart1_rx_buf[i] - '0');
            i++;
          }
          send_str("Block is set. Expect data.\n\r");
        }
        if(counter == 1)
        {
          char str[10];
          select_mode = WRITE_BLOCK;
          send_str("Data: ");
          for(int i = 0; i < 16; i++)
          {
            Data[i] = uart1_rx_buf[i];
            sprintf(str, "%c", Data[i]);
            send_str(str);
          }
          send_str("\n\rReady to write.\n\r");
        }
        if(counter > 1)
        {
          counter = 0;
        }
        else
        {
          counter = counter + 1;
        }
      }
      break;
    default:
      if(data_UART == '\r')
      {
        uart1_rx_buf[uart1_rx_bit]= '\0';
        uart1_rx_bit=0;
        int flag = 1;
        if(strcmp(uart1_rx_buf, "read without key\r")==0)
        {
          select_mode = READ_16SECTORS_NO_KEY;
          send_str("\n\rCommand is active.\n\r");
          flag = 0;
        }
        if(strcmp(uart1_rx_buf, "read with key\r")==0)
        {
          select_mode = READ_16SECTORS_WITH_KEY;
          send_str("\n\rCommand is active.\n\r");
          flag = 0;
        }
        if(strcmp(uart1_rx_buf, "set key\r")==0)
        {
          select_mode = SET_KEY;
          send_str("\n\rCommand is active.Expect the key.\n\r");
          flag = 0;
        }
        if(strcmp(uart1_rx_buf, "get key\r")==0)
        {
          send_str("\n\rKey: ");
          char str[20];
          sprintf(str, "%X %X %X %X %X %X\n\r", Sectorkey[0], Sectorkey[1], Sectorkey[2], Sectorkey[3], Sectorkey[4], Sectorkey[5]);
          send_str( str);
          flag = 0;
        }
        if(strcmp(uart1_rx_buf, "write block\r")==0)
        {
          select_mode = SET_BLOCK;
          send_str("\n\rCommand is active. Expect block.\n\r");
          flag = 0;
        }
        if(flag){
          send_str("\n\rUnnknown commande\n\r");
        }
      }
    break;
    }
    
    
  }
} 

/**
  * @}
  */ 

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
