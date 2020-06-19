/*
 * File      : usart.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006-2013, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2010-03-29     Bernard      remove interrupt Tx and DMA Rx mode
 * 2013-05-13     aozima       update for kehong-lingtai.
 * 2015-01-31     armink       make sure the serial transmit complete in putc()
 * 2016-05-13     armink       add DMA Rx mode
 * 2017-01-19     aubr.cool    add interrupt Tx mode
 */

#include "gd32f20x.h"
#include "usart.h"
#include <rtdevice.h>

/* USART1 */
#define UART1_GPIO_TX        GPIO_PIN_9
#define UART1_GPIO_RX        GPIO_PIN_10
#define UART1_GPIO           GPIOA

/* STM32 uart driver */
struct stm32_uart
{
	uint32_t uart_device;
	IRQn_Type irq;
};

static rt_err_t stm32_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct stm32_uart* uart;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    uart = (struct stm32_uart *)serial->parent.user_data;
    usart_baudrate_set(uart->uart_device, cfg->baud_rate);

    if (cfg->data_bits == DATA_BITS_8){
        usart_word_length_set(uart->uart_device, USART_WL_8BIT);
    } else if (cfg->data_bits == DATA_BITS_9) {
        usart_word_length_set(uart->uart_device, USART_WL_9BIT);
    }

    if (cfg->stop_bits == STOP_BITS_1){
        usart_stop_bit_set(uart->uart_device,USART_STB_1BIT);
    } else if (cfg->stop_bits == STOP_BITS_2){
        usart_stop_bit_set(uart->uart_device,USART_STB_2BIT);
    }

    if (cfg->parity == PARITY_NONE){
        usart_parity_config(uart->uart_device, USART_PM_NONE);
    } else if (cfg->parity == PARITY_ODD) {
        usart_parity_config(uart->uart_device, USART_PM_ODD);
    } else if (cfg->parity == PARITY_EVEN) {
        usart_parity_config(uart->uart_device, USART_PM_EVEN);
    }

    usart_hardware_flow_cts_config(uart->uart_device, USART_CTS_DISABLE);
    usart_hardware_flow_rts_config(uart->uart_device, USART_RTS_DISABLE);
    usart_transmit_config(uart->uart_device, USART_TRANSMIT_ENABLE);
    usart_receive_config(uart->uart_device, USART_RECEIVE_ENABLE);

    /* Enable USART */
    usart_enable(uart->uart_device);

    return RT_EOK;
}

static rt_err_t stm32_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct stm32_uart* uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct stm32_uart *)serial->parent.user_data;

    switch (cmd)
    {
    /* disable interrupt */
    case RT_DEVICE_CTRL_CLR_INT:
        /* disable rx irq */
        UART_DISABLE_IRQ(uart->irq);
        /* disable interrupt */
        usart_interrupt_disable(uart->uart_device, USART_INT_RBNE);
        break;
        /* enable interrupt */
    case RT_DEVICE_CTRL_SET_INT:
        /* enable rx irq */
        UART_ENABLE_IRQ(uart->irq);
        /* enable interrupt */
        usart_interrupt_enable(uart->uart_device, USART_INT_RBNE);
        break;
        /* USART config */
    case RT_DEVICE_CTRL_CONFIG :
        break;
    }
    return RT_EOK;
}

static int stm32_putc(struct rt_serial_device *serial, char c)
{
    struct stm32_uart* uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct stm32_uart *)serial->parent.user_data;

    if(serial->parent.open_flag & RT_DEVICE_FLAG_INT_TX)
    {
        if(!(usart_flag_get(uart->uart_device, USART_FLAG_TBE)))
        {
            usart_interrupt_enable(uart->uart_device, USART_INT_TC);
            return -1;
        }
        usart_data_transmit(uart->uart_device,(uint16_t)c);
        usart_interrupt_enable(uart->uart_device, USART_INT_TC);
    }
    else
    {
        usart_data_transmit(uart->uart_device,(uint16_t)c);
        while (!(usart_flag_get(uart->uart_device, USART_FLAG_TC)));
    }

    return 1;
}

static int stm32_getc(struct rt_serial_device *serial)
{
    int ch;
    struct stm32_uart* uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct stm32_uart *)serial->parent.user_data;

    ch = -1;
    if (usart_flag_get(uart->uart_device, USART_FLAG_RBNE))
    {
        
        ch = usart_data_receive(uart->uart_device) & 0xff;
    }

    return ch;
}

/**
 * Uart common interrupt process. This need add to uart ISR.
 *
 * @param serial serial device
 */
static void uart_isr(struct rt_serial_device *serial) {
    struct stm32_uart *uart = (struct stm32_uart *) serial->parent.user_data;

    RT_ASSERT(uart != RT_NULL);
    if(usart_interrupt_flag_get(uart->uart_device, USART_INT_FLAG_RBNE) != RESET)
    {
        rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_IND);
        /* clear interrupt */
        usart_interrupt_flag_clear(uart->uart_device, USART_INT_FLAG_RBNE);
    }

    if (usart_interrupt_flag_get(uart->uart_device, USART_INT_FLAG_TC) != RESET)
    {
        /* clear interrupt */
        if(serial->parent.open_flag & RT_DEVICE_FLAG_INT_TX)
        {
            rt_hw_serial_isr(serial, RT_SERIAL_EVENT_TX_DONE);
        }
        usart_interrupt_disable(uart->uart_device, USART_INT_RBNE);
        usart_interrupt_flag_clear(uart->uart_device, USART_INT_TC);
    }

    if (usart_interrupt_flag_get(uart->uart_device, USART_INT_FLAG_RBNE_ORERR) == SET)
    {
        stm32_getc(serial);
    }
}

static const struct rt_uart_ops stm32_uart_ops =
{
    stm32_configure,
    stm32_control,
    stm32_putc,
    stm32_getc,
};

#if defined(RT_USING_UART0)
/* UART1 device driver structure */
struct stm32_uart uart0 =
{
    USART0,
    USART0_IRQn,
};
struct rt_serial_device serial1;

void USART0_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();
    uart_isr(&serial1);
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* RT_USING_UART0 */

static void RCC_Configuration(void)
{
#if defined(RT_USING_UART0)
    /* Enable UART GPIO clocks */
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_GPIOA);
    /* Enable UART clock */
    rcu_periph_clock_enable(RCU_USART0);
#endif /* RT_USING_UART0 */
}

static void GPIO_Configuration(void)
{
#if defined(RT_USING_UART0)
    /* Configure USART Rx/tx PIN */
    gpio_init(UART1_GPIO, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, UART1_GPIO_RX);
    gpio_init(UART1_GPIO, GPIO_MODE_AF_PP, GPIO_OSPEED_2MHZ, UART1_GPIO_TX);
#endif /* RT_USING_UART0 */
}

static void NVIC_Configuration(struct stm32_uart* uart)
{
    /* Enable the USART1 Interrupt */
    nvic_irq_enable(uart->irq, 0, 0);
}

void rt_hw_usart_init(void)
{
    struct stm32_uart* uart;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

    RCC_Configuration();
    GPIO_Configuration();

#if defined(RT_USING_UART0)
    uart = &uart0;
    config.baud_rate = BAUD_RATE_115200;

    serial1.ops    = &stm32_uart_ops;
    serial1.config = config;

    NVIC_Configuration(uart);

    /* register UART1 device */
    rt_hw_serial_register(&serial1, "uart0",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX |
                          RT_DEVICE_FLAG_INT_TX ,
                          uart);
#endif /* RT_USING_UART0 */
}
