/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-02-22 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
#ifndef __W25Q64FLASH_H__
#define __W25Q64FLASH_H__

/*****************************************************************************/
/*                                                                           */
/*  Definitions                                                              */
/*                                                                           */
/*****************************************************************************/
#define IO_SIMULATE_SPI //ʹ��IOģ��SPI


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
#ifdef IO_SIMULATE_SPI
#else
#define W25Q_SPI    SPI2
#endif
/*****************************************************************************/
/*                                                                           */
/*  Function Declarations                                                    */
/*                                                                           */
/*****************************************************************************/
#ifdef IO_SIMULATE_SPI
void SPI_Send_Byte(unsigned char out);
unsigned char SPI_Get_Byte(void);
#endif
void SPI_Wait_Busy(void);
void SPI_Write_Enable(void);		//дʹ��
void SPI_Read_nBytes(unsigned int Dst_Addr, unsigned char nBytes,unsigned char *header);//��flashоƬ��ȡ����
void SPI_Write_nBytes(unsigned int Dst_Addr, unsigned char nBytes, unsigned char *header);//��flashоƬд����
void SPI_Erase_Block(unsigned int Dst_Addr);	//������
void SPI_Erase_Half_Block(unsigned int Dst_Addr);	//�������
void SPI_Erase_Sector(unsigned int Dst_Addr);		//��������
void SPI_Erase_All(void);			//��������оƬ
void SPI_erase(unsigned int address,unsigned short numbers,unsigned char mode);
void SPI_WriteW25X_Disable(void);
void SPI_init(void);
#endif /*__W25Q64FLASH_H__*/
