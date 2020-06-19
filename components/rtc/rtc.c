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

//���ʱ���Ƿ���ȷ
//ʱ��Ϊ����ʱ��
int checktime(char* currenttime)
{
    int year = 0,month = 0,day = 0,hour = 0,minute = 0,second = 0;
    char year_str[5] = {0x00};
    char month_str[3] = {0x00};
    char day_str[3] = {0x00};
    char hour_str[3] = {0x00};
    char minute_str[3] = {0x00};
    char second_str[3] = {0x00};

    //��ȡ��
    rt_memcpy(year_str,&currenttime[0],4);
    year = atoi(year_str);
    //��ȡ�·�
    rt_memcpy(month_str,&currenttime[4],2);
    month = atoi(month_str);
    //��ȡ����
    rt_memcpy(day_str,&currenttime[6],2);
    day = atoi(day_str);
    //��ȡСʱ
    rt_memcpy(hour_str,&currenttime[8],2);
    hour = atoi(hour_str);
    //��ȡ����
    rt_memcpy(minute_str,&currenttime[10],2);
    minute = atoi(minute_str);
    //��ȡ��
    rt_memcpy(second_str,&currenttime[12],2);
    second = atoi(second_str);

    //���С��2019��  �쳣
    if(year < 2019) return -1;
    //�·ݲ���1��12��֮��  �쳣
    if((month < 1) ||(month >12)) return -1;
    //���ڲ���1��31֮��  �쳣
    if((day < 1)||(day > 31)) return -1;
    //Сʱ����0��23֮�� �쳣
    if((hour < 0)||(hour > 23)) return -1;
    //���Ӳ���0��59֮�� �쳣
    if((minute < 0)||(minute > 59)) return -1;
    //�벻��0��59֮�� �쳣
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
