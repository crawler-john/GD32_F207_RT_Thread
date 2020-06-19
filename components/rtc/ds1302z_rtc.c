/*****************************************************************************/
/* File      : ds1302z_rtc.c                                                 */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-02-20 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <rtthread.h>
#include <string.h>
#include <rthw.h>
#include <gd32f20x.h>
#include "stdlib.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define DS1302CLK GPIO_PIN_0   //与时钟线相连的芯片的管脚
#define DS1302DAT GPIO_PIN_8   //与数据线相连的芯片的管脚
#define DS1302RST GPIO_PIN_2   //与复位端相连的芯片的管脚

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
void my_gpio_bit_reset(uint32_t gpio_periph,uint32_t pin)
{
    gpio_bit_reset(gpio_periph,pin);
    __NOP();
    __NOP();
    __NOP();
    __NOP();
}

void my_gpio_bit_set(uint32_t gpio_periph,uint32_t pin)
{
    gpio_bit_set(gpio_periph,pin);
    __NOP();
    __NOP();
    __NOP();
    __NOP();
}

void ds1302_writebyte(unsigned char dat)
{
    unsigned char i = 0;
    //设置为推免输出
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, DS1302DAT);

    my_gpio_bit_reset(GPIOC,DS1302CLK);           //初始时钟线置为0
    for(i=0;i<8;i++)    //开始传输8个字节的数据
    {
        if(dat&0x01)	//取最低位，注意 DS1302的数据和地址都是从最低位开始传输的
        {
            my_gpio_bit_set(GPIOC,DS1302DAT);
        }else
        {
            my_gpio_bit_reset(GPIOC,DS1302DAT);
        }
        my_gpio_bit_set(GPIOC,DS1302CLK);       //时钟线拉高，制造上升沿，SDA的数据被传输
        my_gpio_bit_reset(GPIOC,DS1302CLK);       //时钟线拉低，为下一个上升沿做准备
        dat>>=1;        //数据右移一位，准备传输下一位数据
    }
}

unsigned char ds1302_readbyte(void)
{
    unsigned char i = 0,dat = 0;
    //设置为上拉输入
    gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, DS1302DAT);

    for(i=0;i<8;i++)
    {
        dat>>=1;        //要返回的数据左移一位
        if(gpio_input_bit_get(GPIOC,DS1302DAT) == SET)     //当数据线为高时，证明该位数据为 1
            dat|=0x80;  //要传输数据的当前值置为 1,若不是,则为 0
        my_gpio_bit_set(GPIOC,DS1302CLK);       //拉高时钟线
        my_gpio_bit_reset(GPIOC,DS1302CLK);       //制造下降沿
    }
    return dat;         //返回读取出的数据
}

unsigned char ds1302_read(unsigned char cmd)
{
    unsigned char data = 0;

    my_gpio_bit_reset(GPIOC,DS1302RST);
    my_gpio_bit_reset(GPIOC,DS1302CLK);
    my_gpio_bit_set(GPIOC,DS1302RST);
    ds1302_writebyte(cmd);
    data = ds1302_readbyte();
    my_gpio_bit_set(GPIOC,DS1302CLK);
    my_gpio_bit_reset(GPIOC,DS1302RST);

    return data;
}

void ds1302_write(unsigned char cmd, unsigned char data)
{
    my_gpio_bit_reset(GPIOC,DS1302RST);
    my_gpio_bit_reset(GPIOC,DS1302CLK);
    my_gpio_bit_set(GPIOC,DS1302RST);
    ds1302_writebyte(cmd);
    ds1302_writebyte(data);
    my_gpio_bit_set(GPIOC,DS1302CLK);
    my_gpio_bit_reset(GPIOC,DS1302RST);
}


void rt_rtc_read(void* buffer, rt_size_t size)
{
    unsigned char year, month, day, hour, minute, second;
    char datetime[20] = {'\0'};
    char temp[5] = {'\0'};

    rt_interrupt_enter();
    year = ds1302_read(0x8D);
    month = ds1302_read(0x89);
    day = ds1302_read(0x87);
    hour = ds1302_read(0x85);
    minute = ds1302_read(0x83);
    second = ds1302_read(0x81);

    strcat(datetime, "20");

    rt_sprintf(temp, "%d", ((year >> 4) & 0x0f));
    strcat(datetime, temp);
    memset(temp, '\0', 5);

    rt_sprintf(temp, "%d", (year & 0x0f));
    strcat(datetime, temp);
    memset(temp, '\0', 5);

    rt_sprintf(temp, "%d", ((month >> 4) & 0x0f));
    strcat(datetime, temp);
    memset(temp, '\0', 5);

    rt_sprintf(temp, "%d", (month & 0x0f));
    strcat(datetime, temp);
    memset(temp, '\0', 5);

    rt_sprintf(temp, "%d", ((day >> 4) & 0x0f));
    strcat(datetime, temp);
    memset(temp, '\0', 5);

    rt_sprintf(temp, "%d", (day & 0x0f));
    strcat(datetime, temp);
    memset(temp, '\0', 5);

    rt_sprintf(temp, "%d", ((hour >> 4) & 0x07));
    strcat(datetime, temp);
    memset(temp, '\0', 5);

    rt_sprintf(temp, "%d", (hour & 0x0f));
    strcat(datetime, temp);
    memset(temp, '\0', 5);

    rt_sprintf(temp, "%d", ((minute >> 4) & 0x0f));
    strcat(datetime, temp);
    memset(temp, '\0', 5);

    rt_sprintf(temp, "%d", (minute & 0x0f));
    strcat(datetime, temp);
    memset(temp, '\0', 5);

    rt_sprintf(temp, "%d", ((second >> 4) & 0x0f));
    strcat(datetime, temp);
    memset(temp, '\0', 5);

    rt_sprintf(temp, "%d", (second & 0x0f));
    strcat(datetime, temp);
    memset(temp, '\0', 5);

    rt_memcpy(buffer,datetime,14);
    rt_interrupt_leave();
}

void rt_rtc_write(const void       *buffer,
                              rt_size_t         size)
{
    unsigned year, month, day, hour, minute, second;
    unsigned char datetime[20] = {'\0'};

    rt_interrupt_enter();
    rt_memcpy(datetime,buffer,size);
    year = ((datetime[2] - 0x30) << 4) | (datetime[3] - 0x30);
    month = ((datetime[4] - 0x30) << 4) | (datetime[5] - 0x30);
    day = ((datetime[6] - 0x30) << 4) | (datetime[7] - 0x30);
    hour = ((datetime[8] - 0x30) << 4) | (datetime[9] - 0x30);
    minute = ((datetime[10] - 0x30) << 4) | (datetime[11] - 0x30);
    second = ((datetime[12] - 0x30) << 4) | (datetime[13] - 0x30);

    ds1302_write(0x8C, year);
    ds1302_write(0x88, month);
    ds1302_write(0x86, day);
    ds1302_write(0x84, hour);
    ds1302_write(0x82, minute);
    ds1302_write(0x80, second);
    rt_interrupt_leave();
}

/*******************************************************************************
* Function Name  : RTC_Configuration
* Description    : Configures the RTC.
* Input          : None
* Output         : None
* Return         : 0 reday,-1 error.
*******************************************************************************/
int RTC_Configuration(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, DS1302CLK);
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, DS1302RST);
    my_gpio_bit_set(GPIOC,DS1302CLK);
    my_gpio_bit_set(GPIOC,DS1302RST);

    ds1302_write(0x8e, 0x00);	//关闭写保护
    return 0;
}

void rt_hw_rtc_init(void)
{
    if ( RTC_Configuration() != 0)
    {
        return ;
    }
}


/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int set_time(const char* time)
{
    if(strlen(time) != 14)
    {
        return -1;
    }

    rt_rtc_write(time,14);
    return 0;
}

int apstime(char* currenttime)
{
    char time[15] = {'\0'};
    if(RT_NULL == currenttime)
    {
        return -1;
    }
    rt_rtc_read(time,14);
    rt_memcpy(currenttime,time,14);
    time[14] = '\0';
    return 0;
}

//检测时间是否正确
//时间为传入时间
int checktime(char* currenttime)
{
    int year = 0,month = 0,day = 0,hour = 0,minute = 0,second = 0;
    char year_str[5] = {0x00};
    char month_str[3] = {0x00};
    char day_str[3] = {0x00};
    char hour_str[3] = {0x00};
    char minute_str[3] = {0x00};
    char second_str[3] = {0x00};

    //获取年
    rt_memcpy(year_str,&currenttime[0],4);
    year = atoi(year_str);
    //获取月份
    rt_memcpy(month_str,&currenttime[4],2);
    month = atoi(month_str);
    //获取日期
    rt_memcpy(day_str,&currenttime[6],2);
    day = atoi(day_str);
    //获取小时
    rt_memcpy(hour_str,&currenttime[8],2);
    hour = atoi(hour_str);
    //获取分钟
    rt_memcpy(minute_str,&currenttime[10],2);
    minute = atoi(minute_str);
    //获取秒
    rt_memcpy(second_str,&currenttime[12],2);
    second = atoi(second_str);

    //年份小于2019年  异常
    if(year < 2019) return -1;
    //月份不在1到12月之间  异常
    if((month < 1) ||(month >12)) return -1;
    //日期不在1到31之间  异常
    if((day < 1)||(day > 31)) return -1;
    //小时不在0到23之间 异常
    if((hour < 0)||(hour > 23)) return -1;
    //分钟不在0到59之间 异常
    if((minute < 0)||(minute > 59)) return -1;
    //秒不在0到59之间 异常
    if((second < 0)||(second > 59)) return -1;

    return 1;
}

#ifdef RT_USING_FINSH
#include <finsh.h>

void gettime(void)
{
    char curtime[15] = {0x00};
    apstime(curtime);
    rt_kprintf("time:%s\n",curtime);
}
FINSH_FUNCTION_EXPORT(set_time, eg:set_time());
FINSH_FUNCTION_EXPORT(gettime, eg:gettime());
#endif
