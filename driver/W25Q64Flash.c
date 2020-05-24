/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-02-22 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Include Files                                                            */
/*                                                                           */
/*****************************************************************************/
#include "W25Q64Flash.h"
#include <gd32f20x.h>
#include "rtthread.h"
#include "stdio.h"

/*****************************************************************************/
/*                                                                           */
/*  Definitions                                                              */
/*                                                                           */
/*****************************************************************************/
#define W25_RCC1                    RCU_GPIOA
#define W25_RCC2                    RCU_GPIOB
#define CLK_GPIO                   GPIOB
#define CLK_PIN                    (GPIO_PIN_3)

#define DO_GPIO                   GPIOB
#define DO_PIN                    (GPIO_PIN_4)

#define DI_GPIO                   GPIOB
#define DI_PIN                    (GPIO_PIN_5)

#define CS_GPIO                   GPIOA
#define CS_PIN                    (GPIO_PIN_15)

#define W25X_WriteEnable    0x06
#define W25X_WriteDisable   0x04
#define W25X_ReadStatusReg  0x05
#define W25X_ReadData       0x03
#define W25X_PageProgram    0x02
#define W25X_64KBErase      0xD8//0x20-sector-4KB*1024//0x52-1/2.block-32KB*128//0xd8-block-64KB*64//0xc7-all//
#define W25X_4KBErase       0x20//0x20-sector-4KB*1024//0x52-1/2.block-32KB*128//0xd8-block-64KB*64//0xc7-all//
#define W25X_32KBErase      0x52//0x20-sector-4KB*1024//0x52-1/2.block-32KB*128//0xd8-block-64KB*64//0xc7-all//
#define W25X_alKBErase      0xc7
#define FlashTestAdd        0x000000

/*****************************************************************************/
/*                                                                           */
/*  Function Implementations                                                 */
/*                                                                           */
/*****************************************************************************/
void SPI_Send_Byte(U8 out)
{
	U8 i = 0;
	
	gpio_bit_reset(CS_GPIO,CS_PIN);
	for (i = 0; i < 8; i++)
	{
		if ((out & 0x80) == 0x80)	                                 //check if MSB is high
			gpio_bit_set(DI_GPIO, DI_PIN);
		else
			gpio_bit_reset(DI_GPIO, DI_PIN);		                                     //if not, set to low
		__NOP();
		__NOP();
		gpio_bit_set(CLK_GPIO, CLK_PIN);			                                     //toggle clock high
		out = (out << 1);		                                     //shift 1 place for next bit
		__NOP();
		__NOP();
		__NOP();
	  __NOP();
		__NOP();
		gpio_bit_reset(CLK_GPIO, CLK_PIN);			                                     //toggle clock low
	}
}

U8 SPI_Get_Byte(void)
{	
	U8 i = 0, in = 0, temp = 0;
	
	gpio_bit_reset(CS_GPIO, CS_PIN);
	for (i = 0; i < 8; i++)
	{	
		__NOP();
		__NOP();
		in = (in << 1);						                         //shift 1 place to the left or shift in 0
		temp = gpio_input_bit_get(DO_GPIO, DO_PIN);;						                         //save input
		gpio_bit_set(CLK_GPIO, CLK_PIN);						                         //toggle clock high
		__NOP();
		__NOP();
		if (temp == 1)						                         //check to see if bit is high
			in |= 0x01;						                         //if high, make bit high
		gpio_bit_reset(CLK_GPIO, CLK_PIN);						                         //toggle clock low
	}	
	return in;
}

U8 SPI_Read_StatusReg(void)
{	
	U8 byte = 0;
	
	gpio_bit_reset(CS_GPIO, CS_PIN);							                         //enable device
	SPI_Send_Byte(W25X_ReadStatusReg);		                         //send Read Status Register command
	byte = SPI_Get_Byte();					                         //receive byte
	gpio_bit_set(CS_GPIO, CS_PIN);							                         //disable device	
	return byte;
}

void SPI_Wait_Busy(void)
{
 	//waste time until not busy WEL & Busy bit all be 1 (0x03).	
	//while(SPI_Read_StatusReg()==0x03)
	while ((SPI_Read_StatusReg()&0x03) == 0x03)
	{
		__NOP();
		__NOP();
		__NOP();
		__NOP();
	}
}


void SPI_Write_Enable(void)
{	
	gpio_bit_reset(CS_GPIO, CS_PIN);							                         //enable device
	SPI_Send_Byte(W25X_WriteEnable);		                         //send W25X_Write_Enable command
	gpio_bit_set(CS_GPIO, CS_PIN);							                         //disable device
}

void SPI_Read_nBytes(U32 Dst_Addr, U8 nBytes,U8 *header)//num=1,2
{
	U8 i;

		gpio_bit_reset(CS_GPIO, CS_PIN);										             //enable device
		SPI_Send_Byte(W25X_ReadData);						             //read command
		SPI_Send_Byte((U8)((Dst_Addr & 0xFFFFFF) >> 16));		     //send 3 address bytes
		SPI_Send_Byte((U8)((Dst_Addr & 0xFFFF) >> 8));
		SPI_Send_Byte((U8)(Dst_Addr & 0xFF));
		for (i = 0; i < nBytes; i++)			                         //read until no_bytes is reached
		{
			*(header+i) = SPI_Get_Byte();					         //receive byte and store at address 80H - FFH
		}
		gpio_bit_set(CS_GPIO, CS_PIN);									                 //disable device
	
}

void SPI_Write_nBytes(U32 Dst_Addr, U8 nBytes, U8 *header)
{	
	U8 i, byte;


		gpio_bit_reset(CS_GPIO, CS_PIN);					                                 //enable device
		SPI_Write_Enable();				                                 //set WEL
		gpio_bit_reset(CS_GPIO, CS_PIN);
		SPI_Send_Byte(W25X_PageProgram); 		                         //send Byte Program command
		SPI_Send_Byte((U8)((Dst_Addr & 0xFFFFFF) >> 16));	         //send 3 address bytes
		SPI_Send_Byte((U8)((Dst_Addr & 0xFFFF) >> 8));
		SPI_Send_Byte((U8)(Dst_Addr & 0xFF));
		
		for (i = 0; i < nBytes; i++)
		{
			byte = *(header+i);
			SPI_Send_Byte(byte);		                                 //send byte to be programmed
		}	
		gpio_bit_set(CS_GPIO, CS_PIN);			//disable device		
		
}

void SPI_Erase_Block(U32 Dst_Addr)
{

		gpio_bit_reset(CS_GPIO, CS_PIN);										             //enable device
		SPI_Write_Enable();									             //set WEL
		gpio_bit_reset(CS_GPIO, CS_PIN);
		SPI_Send_Byte(W25X_64KBErase);			                         //send Sector Erase command
		SPI_Send_Byte((U8)((Dst_Addr & 0xFFFFFF) >> 16)); 	         //send 3 address bytes
		SPI_Send_Byte((U8)((Dst_Addr & 0xFFFF) >> 8));
		SPI_Send_Byte((U8)(Dst_Addr & 0xFF));
		gpio_bit_set(CS_GPIO, CS_PIN);			//disable device

}

void SPI_Erase_Half_Block(U32 Dst_Addr)
{
		gpio_bit_reset(CS_GPIO, CS_PIN);										             //enable device
		SPI_Write_Enable();									             //set WEL
		gpio_bit_reset(CS_GPIO, CS_PIN);
		SPI_Send_Byte(W25X_32KBErase);			                         //send Sector Erase command
		SPI_Send_Byte((U8)((Dst_Addr & 0xFFFFFF) >> 16)); 	         //send 3 address bytes
		SPI_Send_Byte((U8)((Dst_Addr & 0xFFFF) >> 8));
		SPI_Send_Byte((U8)(Dst_Addr & 0xFF));
		gpio_bit_set(CS_GPIO, CS_PIN);					                                 //disable device
}

void SPI_Erase_Sector(U32 Dst_Addr)
{
		gpio_bit_reset(CS_GPIO, CS_PIN);										             //enable device
		SPI_Write_Enable();									             //set WEL
		gpio_bit_reset(CS_GPIO, CS_PIN);
		SPI_Send_Byte(W25X_4KBErase);			                         //send Sector Erase command
		SPI_Send_Byte((U8)((Dst_Addr & 0xFFFFFF) >> 16)); 	         //send 3 address bytes
		SPI_Send_Byte((U8)((Dst_Addr & 0xFFFF) >> 8));
		SPI_Send_Byte((U8)(Dst_Addr & 0xFF));
		gpio_bit_set(CS_GPIO, CS_PIN);					                                 //disable device

}

void SPI_Erase_All(void)
{
		gpio_bit_reset(CS_GPIO, CS_PIN);										             //enable device
		SPI_Write_Enable();									             //set WEL
		gpio_bit_reset(CS_GPIO, CS_PIN);
		SPI_Send_Byte(W25X_alKBErase);			                         //send Sector Erase command
		gpio_bit_set(CS_GPIO, CS_PIN);					                                 //disable device
}

void SPI_erase(U32 address,U16 numbers,U8 mode)
{
	U8 i;

	if(mode==1)
	{
		SPI_Erase_All();
		return;
	}
	if(mode==4)
	{
		for(i=0;i<numbers;i++)
		{
		 	SPI_Wait_Busy();                                         //0x010000 32KB  U16
			SPI_Erase_Sector(address);
			address=address+0x1000;                                  //0x001000 4 KB  U16
		}
		return;
	}
	if(mode==32)
	{
		for(i=0;i<numbers;i++)
		{
			SPI_Wait_Busy();
			SPI_Erase_Half_Block(address);
			address=address+0x8000;                                  //0x010000 32KB  U16                                   //0x001000 4 KB  U16
		}
		return;
	}
	if(mode==64)
	{
		for(i=0;i<numbers;i++)
		{
			SPI_Wait_Busy();
			SPI_Erase_Block(address);
			address=address+0x10000;                                 //0x010000 64KB  U16                                   //0x001000 4 KB  U16
		}
		return;
	}
}

void SPI_WriteW25X_Disable(void)
{
	gpio_bit_reset(CS_GPIO, CS_PIN);							                         //enable device
	SPI_Send_Byte(W25X_WriteDisable);		                         //send W25X_WriteW25X_DIsable command
	gpio_bit_set(CS_GPIO, CS_PIN);							                         //disable device
}

void SPI_init(void)
{
    rcu_periph_clock_enable(W25_RCC1);
    rcu_periph_clock_enable(W25_RCC2);
    rcu_periph_clock_enable(RCU_AF);

    gpio_init(CLK_GPIO, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, CLK_PIN);
    gpio_init(DI_GPIO, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, DI_PIN);
    gpio_init(CS_GPIO, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, CS_PIN);
    gpio_init(DO_GPIO, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, DO_PIN);
    gpio_pin_remap_config(GPIO_SWJ_NONJTRST_REMAP,ENABLE);
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP,ENABLE);

    gpio_bit_reset(CLK_GPIO, CLK_PIN);							                         //set clock to low initial state for SPI operation mode 0
    //	gpio_bit_set(CLK_GPIO, CLK_PIN);							                         //set clock to High initial state for SPI operation mode 3
    gpio_bit_set(CS_GPIO, CS_PIN);
    SPI_WriteW25X_Disable();

}
#ifdef RT_USING_FINSH
#include <finsh.h>
void init(void)
{
	SPI_init();
}
void test(int i)
{
	if(i == 0)
	{
		unsigned char buffer[8] = {0x00};
		SPI_Read_nBytes(0x000000 ,8,buffer);
		printf("Read:%02x %02x %02x %02x %02x %02x %02x %02x\n",
                buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]);
	}else if(i == 1)
	{
		unsigned char buffer[8]= {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07};
		SPI_Write_nBytes(0x000000,8,buffer);
		printf("Write\n");
	}else if(i == 2)
	{
		SPI_Erase_Sector(0x000000);
		printf("Erase\n");
	}else if(i == 3)
	{
		printf("RESET\n");
		gpio_bit_reset(CLK_GPIO, CLK_PIN);
		gpio_bit_reset(DI_GPIO, DI_PIN);
		gpio_bit_reset(CS_GPIO, CS_PIN);
	}else if(i == 4)
	{
		printf("SET\n");
		gpio_bit_set(CLK_GPIO, CLK_PIN);
		gpio_bit_set(DI_GPIO, DI_PIN);
		gpio_bit_set(CS_GPIO, CS_PIN);
	}else if(i == 5)
	{
		printf("%d %d %d %d\n",gpio_input_bit_get(DO_GPIO, DO_PIN),
                gpio_input_bit_get(CLK_GPIO, CLK_PIN),gpio_input_bit_get(DI_GPIO, DI_PIN),
                gpio_input_bit_get(CS_GPIO, CS_PIN));
	}
}
FINSH_FUNCTION_EXPORT(test, test);
FINSH_FUNCTION_EXPORT(init, init);
#endif


