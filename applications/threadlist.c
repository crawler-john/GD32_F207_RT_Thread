/*****************************************************************************/
/* File      : threadlist.c                                                  */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2019-03-02 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <threadlist.h>
#include "gd32f20x.h"
#include <rtthread.h>
#include "SEGGER_RTT.h"
#include <rthw.h>
#include "shell.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
#ifdef THREAD_PRIORITY_DBG
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t dbg_stack[500];
static struct rt_thread dbg_thread;
#endif

extern struct rt_object_information rt_object_container[];
/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

#ifdef THREAD_PRIORITY_DBG
/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Debug display thread                                                    */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   list[in]   list node                                                    */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
void _list_thread(struct rt_list_node *list)
{
    rt_uint8_t *ptr;
    struct rt_thread *thread;
    struct rt_list_node *node;


    SEGGER_RTT_printf(0,"pri  status      sp     stack size max used left tick  error    thread\n"); 
    SEGGER_RTT_printf(0,"---  ------- ---------- ----------  ------  ---------- -----    ------\n");
    for (node = list->next; node != list; node = node->next)
    {
        thread = rt_list_entry(node, struct rt_thread, list);

        ptr = (rt_uint8_t *)thread->stack_addr;
        while (*ptr == '#')ptr ++;

        SEGGER_RTT_printf(0,"%3d %8d 0x%08x 0x%08x    %02d%%   0x%08x %05d    %8s\n",
			thread->current_priority,thread->stat,
                   thread->stack_size + ((rt_uint32_t)thread->stack_addr - (rt_uint32_t)thread->sp),
                   thread->stack_size,
                   (thread->stack_size - ((rt_uint32_t) ptr - (rt_uint32_t) thread->stack_addr)) * 100/ thread->stack_size,
                   thread->remaining_tick,
                   thread->error,thread->name);
    }
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Debug By JLink                                                          */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
void SEGGER_Debug(void)
{
    char buff[100] = {0x00};
    if(SEGGER_RTT_Read(0,buff,100) > 0)
    {
        if(!rt_memcmp(buff,"1",1))
        {
            SEGGER_RTT_WriteString(0,"list_thread\n");
            _list_thread(&rt_object_container[RT_Object_Class_Thread].object_list);
        }else if(!rt_memcmp(buff,"2",1))
        {
            SEGGER_RTT_WriteString(0,"reboot\n");
            rt_thread_delay(RT_TICK_PER_SECOND*5);
            reboot();
        }
    }
}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Debug program entry                                                     */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   parameter[in]   unused                                                  */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
static void dbg_thread_entry(void* parameter)
{
    int index = 0;
    /* enable the LED2 GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);
    /* configure LED2 GPIO port */ 
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_8);
    while (1)
    {
        /* reset LED2 GPIO pin */
        gpio_bit_reset(GPIOA,GPIO_PIN_8);
        rt_thread_delay( RT_TICK_PER_SECOND/2);
        /* reset LED2 GPIO pin */
        gpio_bit_set(GPIOA,GPIO_PIN_8);
        rt_thread_delay( RT_TICK_PER_SECOND/2);
        SEGGER_Debug();
        index++;
    }
}
#endif

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Create Application ALL Tasks                                            */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
void tasks_new(void)
{
    rt_err_t result;

#ifdef THREAD_PRIORITY_DBG
    /* init led thread */
    result = rt_thread_init(&dbg_thread,"dbg",dbg_thread_entry,RT_NULL,(rt_uint8_t*)&dbg_stack[0],sizeof(dbg_stack),THREAD_PRIORITY_DBG,5);
    if (result == RT_EOK)	rt_thread_startup(&dbg_thread);
#endif

}

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   RT-Thread Application program entry                                     */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/

int rt_application_init(void)
{
#ifdef RT_USING_FINSH
    /* initialize finsh */
    finsh_system_init();
    finsh_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
    tasks_new();
    return 0;
}
