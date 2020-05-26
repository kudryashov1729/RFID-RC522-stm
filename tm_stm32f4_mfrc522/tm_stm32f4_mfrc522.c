/**	
 * |----------------------------------------------------------------------
 * | Copyright (C) Tilen Majerle, 2014
 * | 
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |  
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * | 
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */
#include "tm_stm32f4_mfrc522.h"


void TM_MFRC522_Init(void) {
	TM_MFRC522_InitPins();
        
        
	//SPI 
        LL_SPI_Disable (SPI1);
        LL_SPI_SetMode(SPI1, LL_SPI_MODE_MASTER);
        LL_SPI_SetClockPhase(SPI1, LL_SPI_PHASE_1EDGE); // CPHA = 0
        LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_LOW); // CPOL = 0
        LL_SPI_SetBaudRatePrescaler(SPI1,  LL_SPI_BAUDRATEPRESCALER_DIV32);//??????????
        LL_SPI_SetTransferBitOrder(SPI1,LL_SPI_MSB_FIRST);
        LL_SPI_SetTransferDirection(SPI1, LL_SPI_FULL_DUPLEX);
        LL_SPI_SetDataWidth(SPI1, LL_SPI_DATAWIDTH_8BIT);
        LL_SPI_SetNSSMode(SPI1, LL_SPI_NSS_SOFT);
        LL_SPI_Enable(SPI1);

	TM_MFRC522_Reset();
        
	TM_MFRC522_WriteRegister(MFRC522_REG_T_MODE, 0x8D);             //0x8D
	TM_MFRC522_WriteRegister(MFRC522_REG_T_PRESCALER, 0x3E);        //0x3E
	TM_MFRC522_WriteRegister(MFRC522_REG_T_RELOAD_L, 30);           //30   
	TM_MFRC522_WriteRegister(MFRC522_REG_T_RELOAD_H, 0);            //0

	// 48dB gain 
	TM_MFRC522_WriteRegister(MFRC522_REG_RF_CFG, 0x70);
	
	TM_MFRC522_WriteRegister(MFRC522_REG_TX_AUTO, 0x40);
	TM_MFRC522_WriteRegister(MFRC522_REG_MODE, 0x3D);

	TM_MFRC522_AntennaOn();		//Open the antenna
        
}

TM_MFRC522_Status_t TM_MFRC522_Check(uint8_t* id) {
	TM_MFRC522_Status_t status;
	//Find cards, return card type
	status = TM_MFRC522_Request(PICC_REQALL, id);	
	if (status == MI_OK) {
		//Card detected
		//Anti-collision, return card serial number 4 bytes
		status = TM_MFRC522_Anticoll(id);	
	}
	//TM_MFRC522_Halt();			//Command card into hibernation 

	return status;
}

TM_MFRC522_Status_t TM_MFRC522_Compare(uint8_t* CardID, uint8_t* CompareID) {
	uint8_t i;
	for (i = 0; i < 5; i++) {
		if (CardID[i] != CompareID[i]) {
			return MI_ERR;
		}
	}
	return MI_OK;
}

void TM_MFRC522_InitPins(void) {
  /**
	GPIO_InitTypeDef GPIO_InitStruct;
	//Enable clock
	RCC_AHB1PeriphClockCmd(MFRC522_CS_RCC, ENABLE);

	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	//CS pin
	GPIO_InitStruct.GPIO_Pin = MFRC522_CS_PIN;
	GPIO_Init(MFRC522_CS_PORT, &GPIO_InitStruct);	
*/      
        //GPIO SETTING for SPI1
//        MISO – PB4
//        MOSI – PA7
//        CS – PG2
//        CLK – PA5
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOG_CLK_ENABLE();
        __HAL_RCC_SPI1_CLK_ENABLE();
        
        GPIO_InitTypeDef GPIO_Init_for_SPI1;
        GPIO_Init_for_SPI1.Pin = (GPIO_PIN_7 | GPIO_PIN_5);
        GPIO_Init_for_SPI1.Mode = GPIO_MODE_AF_PP;
        GPIO_Init_for_SPI1.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOA, &GPIO_Init_for_SPI1);
        
        GPIO_Init_for_SPI1.Pin = GPIO_PIN_4;
        HAL_GPIO_Init(GPIOB, &GPIO_Init_for_SPI1);
        
        
        //NSS SETTING
        GPIO_InitTypeDef GPIO_Init_for_SPI2_NSS;
        GPIO_Init_for_SPI2_NSS.Pin = GPIO_PIN_2;
        GPIO_Init_for_SPI2_NSS.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_Init_for_SPI2_NSS.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_Init_for_SPI2_NSS.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOG, &GPIO_Init_for_SPI2_NSS);
        
        MFRC522_CS_HIGH;
}

void TM_MFRC522_WriteRegister(uint8_t addr, uint8_t val) {
	//CS low
	MFRC522_CS_LOW;
        
	//Send address
        /**TM_SPI_Send(MFRC522_SPI, (addr << 1) & 0x7E);*/
        while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {}
        LL_SPI_TransmitData8(SPI1, (addr << 1) & 0x7E);
	while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {}
        LL_SPI_ReceiveData8(SPI1);
        
        // Important! No NSS break here!
        // Software slave select should be used or TransmitData16.
        
	//Send data	
	/**TM_SPI_Send(MFRC522_SPI, val);*/
        while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {}
        LL_SPI_TransmitData8(SPI1, val);
        while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {}
        LL_SPI_ReceiveData8(SPI1);
        
	//CS high
	MFRC522_CS_HIGH;
}

uint8_t TM_MFRC522_ReadRegister(uint8_t addr) {
	uint8_t val;
	//CS low
	MFRC522_CS_LOW;
        
	/**TM_SPI_Send(MFRC522_SPI, ((addr << 1) & 0x7E) | 0x80);	*/
        while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {}
        LL_SPI_TransmitData8 (SPI1, (((addr << 1) & 0x7E) | 0x80));
        while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {}
        LL_SPI_ReceiveData8(SPI1);
        
        // Important! No NSS break here!
        // Software slave select should be used or TransmitData16.
        
        /**val = TM_SPI_Send(MFRC522_SPI, MFRC522_DUMMY);*/
        while(!LL_SPI_IsActiveFlag_TXE(SPI1)) {}
        LL_SPI_TransmitData8 (SPI1, MFRC522_DUMMY);
        while(!LL_SPI_IsActiveFlag_RXNE(SPI1)) {}
        val = LL_SPI_ReceiveData8(SPI1);
        
	//CS high
	MFRC522_CS_HIGH;

	return val;
}

void TM_MFRC522_SetBitMask(uint8_t reg, uint8_t mask) {
	TM_MFRC522_WriteRegister(reg, TM_MFRC522_ReadRegister(reg) | mask);
}

void TM_MFRC522_ClearBitMask(uint8_t reg, uint8_t mask){
	TM_MFRC522_WriteRegister(reg, TM_MFRC522_ReadRegister(reg) & (~mask));
} 

void TM_MFRC522_AntennaOn(void) {
	uint8_t temp;

	temp = TM_MFRC522_ReadRegister(MFRC522_REG_TX_CONTROL);
	if (!(temp & 0x03)) {
		TM_MFRC522_SetBitMask(MFRC522_REG_TX_CONTROL, 0x03);
	}
}

void TM_MFRC522_AntennaOff(void) {
	TM_MFRC522_ClearBitMask(MFRC522_REG_TX_CONTROL, 0x03);
}

void TM_MFRC522_Reset(void) {
	TM_MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_RESETPHASE);
}

TM_MFRC522_Status_t TM_MFRC522_Request(uint8_t reqMode, uint8_t* TagType) {
	TM_MFRC522_Status_t status;  
	uint16_t backBits;			//The received data bits

	TM_MFRC522_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x07);		//TxLastBists = BitFramingReg[2..0]	???

	TagType[0] = reqMode;
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

	if ((status != MI_OK) || (backBits != 0x10)) {    
		status = MI_ERR;
	}

	return status;
}

TM_MFRC522_Status_t TM_MFRC522_ToCard(uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint16_t* backLen) {
	TM_MFRC522_Status_t status = MI_ERR;
	uint8_t irqEn = 0x00;
	uint8_t waitIRq = 0x00;
	uint8_t lastBits;
	uint8_t n;
	uint16_t i;

	switch (command) {
		case PCD_AUTHENT: {
			irqEn = 0x12;
			waitIRq = 0x10;
			break;
		}
		case PCD_TRANSCEIVE: {
			irqEn = 0x77;
			waitIRq = 0x30;
			break;
		}
		default:
			break;
	}

	TM_MFRC522_WriteRegister(MFRC522_REG_COMM_IE_N, irqEn | 0x80);
	TM_MFRC522_ClearBitMask(MFRC522_REG_COMM_IRQ, 0x80);
	TM_MFRC522_SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80);

	TM_MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_IDLE);

	//Writing data to the FIFO
	for (i = 0; i < sendLen; i++) {   
		TM_MFRC522_WriteRegister(MFRC522_REG_FIFO_DATA, sendData[i]);    
	}

	//Execute the command
	TM_MFRC522_WriteRegister(MFRC522_REG_COMMAND, command);
	if (command == PCD_TRANSCEIVE) {    
		TM_MFRC522_SetBitMask(MFRC522_REG_BIT_FRAMING, 0x80);		//StartSend=1,transmission of data starts  
	}   

	//Waiting to receive data to complete
	i = 2000;	//i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms???
	do {
		//CommIrqReg[7..0]
		//Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
		n = TM_MFRC522_ReadRegister(MFRC522_REG_COMM_IRQ);
		i--;
	} while ((i!=0) && !(n&0x01) && !(n&waitIRq));

	TM_MFRC522_ClearBitMask(MFRC522_REG_BIT_FRAMING, 0x80);			//StartSend=0

	if (i != 0)  {
		if (!(TM_MFRC522_ReadRegister(MFRC522_REG_ERROR) & 0x1B)) {
			status = MI_OK;
			if (n & irqEn & 0x01) {   
				status = MI_NOTAGERR;			
			}

			if (command == PCD_TRANSCEIVE) {
				n = TM_MFRC522_ReadRegister(MFRC522_REG_FIFO_LEVEL);
				lastBits = TM_MFRC522_ReadRegister(MFRC522_REG_CONTROL) & 0x07;
				if (lastBits) {   
					*backLen = (n - 1) * 8 + lastBits;   
				} else {   
					*backLen = n * 8;   
				}

				if (n == 0) {   
					n = 1;    
				}
				if (n > MFRC522_MAX_LEN) {   
					n = MFRC522_MAX_LEN;   
				}

				//Reading the received data in FIFO
				for (i = 0; i < n; i++) {   
					backData[i] = TM_MFRC522_ReadRegister(MFRC522_REG_FIFO_DATA);    
				}
			}
		} else {   
			status = MI_ERR;  
		}
	}

	return status;
}

TM_MFRC522_Status_t TM_MFRC522_Anticoll(uint8_t* serNum) {
	TM_MFRC522_Status_t status;
	uint8_t i;
	uint8_t serNumCheck = 0;
	uint16_t unLen;

	TM_MFRC522_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x00);		//TxLastBists = BitFramingReg[2..0]

	serNum[0] = PICC_ANTICOLL;
	serNum[1] = 0x20;
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

	if (status == MI_OK) {
		//Check card serial number
		for (i = 0; i < 4; i++) {   
			serNumCheck ^= serNum[i];
		}
		if (serNumCheck != serNum[i]) {   
			status = MI_ERR;    
		}
	}
	return status;
} 

void TM_MFRC522_CalculateCRC(uint8_t*  pIndata, uint8_t len, uint8_t* pOutData) {
	uint8_t i, n;

	TM_MFRC522_ClearBitMask(MFRC522_REG_DIV_IRQ, 0x04);			//CRCIrq = 0
	TM_MFRC522_SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80);			//Clear the FIFO pointer
	//Write_MFRC522(CommandReg, PCD_IDLE);

	//Writing data to the FIFO	
	for (i = 0; i < len; i++) {   
		TM_MFRC522_WriteRegister(MFRC522_REG_FIFO_DATA, *(pIndata+i));   
	}
	TM_MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_CALCCRC);

	//Wait CRC calculation is complete
	i = 0xFF;
	do {
		n = TM_MFRC522_ReadRegister(MFRC522_REG_DIV_IRQ);
		i--;
	} while ((i!=0) && !(n&0x04));			//CRCIrq = 1

	//Read CRC calculation result
	pOutData[0] = TM_MFRC522_ReadRegister(MFRC522_REG_CRC_RESULT_L);
	pOutData[1] = TM_MFRC522_ReadRegister(MFRC522_REG_CRC_RESULT_M);
}

uint8_t TM_MFRC522_SelectTag(uint8_t* serNum) {
	uint8_t i;
	TM_MFRC522_Status_t status;
	uint8_t size;
	uint16_t recvBits;
	uint8_t buffer[9]; 

	buffer[0] = PICC_SElECTTAG;
	buffer[1] = 0x70;
	for (i = 0; i < 5; i++) {
		buffer[i+2] = *(serNum+i);
	}
	TM_MFRC522_CalculateCRC(buffer, 7, &buffer[7]);		//??
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);

	if ((status == MI_OK) && (recvBits == 0x18)) {   
		size = buffer[0]; 
	} else {   
		size = 0;    
	}

	return size;
}
/**
 * Executes the MFRC522 MFAuthent command.
 * This command manages MIFARE authentication to enable a secure communication to any MIFARE Mini, MIFARE 1K and MIFARE 4K card.
 * The authentication is described in the MFRC522 datasheet section 10.3.1.9 and http://www.nxp.com/documents/data_sheet/MF1S503x.pdf section 10.1.
 * For use with MIFARE Classic PICCs.
 * The PICC must be selected - ie in state ACTIVE(*) - before calling this function.
 * Remember to call PCD_StopCrypto1() after communicating with the authenticated PICC - otherwise no new communications can start.
 * 
 * All keys are set to FFFFFFFFFFFFh at chip delivery.
 * 
 * @return STATUS_OK on success, STATUS_??? otherwise. Probably STATUS_TIMEOUT if you supply the wrong key.
 */
TM_MFRC522_Status_t TM_MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t* Sectorkey, uint8_t* serNum) {
	TM_MFRC522_Status_t status;
	uint16_t recvBits;
	uint8_t i;
	uint8_t buff[12]; 

	//Verify the command block address + sector + password + card serial number
	buff[0] = authMode;
	buff[1] = BlockAddr;
	for (i = 0; i < 6; i++) {    
		buff[i+2] = *(Sectorkey+i);   
	}
	for (i=0; i<4; i++) {    
		buff[i+8] = *(serNum+i);   
	}
	status = TM_MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);

	if ((status != MI_OK) || (!(TM_MFRC522_ReadRegister(MFRC522_REG_STATUS2) & 0x08))) {   
		status = MI_ERR;   
	}

	return status;
}

TM_MFRC522_Status_t TM_MFRC522_Read(uint8_t blockAddr, uint8_t* recvData) {
	TM_MFRC522_Status_t status;
	uint16_t unLen;

	recvData[0] = PICC_READ;
	recvData[1] = blockAddr;
	TM_MFRC522_CalculateCRC(recvData,2, &recvData[2]);
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);

	if ((status != MI_OK) || (unLen != 0x90)) {
		status = MI_ERR;
	}
        TM_MFRC522_Halt();
        TM_MFRC522_StopCrypto1();
	return status;
}

TM_MFRC522_Status_t TM_MFRC522_Write(uint8_t blockAddr, uint8_t* writeData) {
	TM_MFRC522_Status_t status;
	uint16_t recvBits;
	uint8_t i;
	uint8_t buff[18]; 

	buff[0] = PICC_WRITE;
	buff[1] = blockAddr;
	TM_MFRC522_CalculateCRC(buff, 2, &buff[2]);
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

	if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) {   
		status = MI_ERR;   
	}

	if (status == MI_OK) {
		//Data to the FIFO write 16Byte
		for (i = 0; i < 16; i++) {    
			buff[i] = *(writeData+i);   
		}
		TM_MFRC522_CalculateCRC(buff, 16, &buff[16]);
		status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);

		if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) {   
			status = MI_ERR;   
		}
	}

	return status;
}

void TM_MFRC522_Halt(void) {
	uint16_t unLen;
	uint8_t buff[4]; 

	buff[0] = PICC_HALT;
	buff[1] = 0;
	TM_MFRC522_CalculateCRC(buff, 2, &buff[2]);

	TM_MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}
/**
 * Used to exit the PCD from its authenticated state.
 * Remember to call this function after communicating with an authenticated PICC - otherwise no new communications can start. Use after halt.
 */
void TM_MFRC522_StopCrypto1( void) {
  
	// Clear MFCrypto1On bit
	// Status2Reg[7..0] bits are: TempSensClear I2CForceHS reserved reserved MFCrypto1On ModemState[2:0]
        uint8_t tmp;
	tmp = TM_MFRC522_ReadRegister(MFRC522_REG_STATUS2);
	TM_MFRC522_WriteRegister(MFRC522_REG_STATUS2, tmp & (~ 0x08));		// clear bit mask
} // End TM_MFRC522_StopCrypto1()
