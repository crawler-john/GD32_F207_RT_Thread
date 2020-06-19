/*****************************************************************************/
/* File      : rtc.c                                                         */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-03-20 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <string.h>
#include <rtthread.h>
#include "stdlib.h"

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int set_time(const char* time)
{
    rt_device_t device;
    if(strlen(time) != 14)
    {
        return -1;
    }

    device = rt_device_find("rtc");
    if (device == RT_NULL)
    {
        return -1;
    }
    device->write(device, 0, time,14);
    return 0;
}

int apstime(char* currenttime)
{
    rt_device_t device;
    char time[15] = {'\0'};
    if(RT_NULL == currenttime)
    {
        return -1;
    }
    device = rt_device_find("rtc");
    if (device == RT_NULL)
    {
        return -1;
    }
    device->read(device, 0, time,14);
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
