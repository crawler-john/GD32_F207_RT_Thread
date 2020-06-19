#include "rt_gd32f20x_spi.h"
#include "stdio.h"
#include "spi_flash_sfud.h"

static rt_err_t configure(struct rt_spi_device* device, struct rt_spi_configuration* configuration);
static rt_uint32_t xfer(struct rt_spi_device* device, struct rt_spi_message* message);

static struct rt_spi_ops stm32_spi_ops =
{
    configure,
    xfer
};

//------------------ DMA ------------------
#ifdef SPI_USE_DMA
static uint8_t dummy = 0xFF;
#endif

#ifdef SPI_USE_DMA
static void DMA_Configuration(const void * send_addr, void * recv_addr, rt_size_t size)
{
    dma_parameter_struct DMA_InitStructure;

    dma_flag_clear(DMA1,DMA_CH0,DMA_FLAG_FTF);
    dma_flag_clear(DMA1,DMA_CH0,DMA_FLAG_ERR);
    dma_flag_clear(DMA1,DMA_CH1,DMA_FLAG_FTF);
    dma_flag_clear(DMA1,DMA_CH1,DMA_FLAG_ERR);

    /* RX channel configuration */
    dma_channel_disable(DMA1,DMA_CH0);
    DMA_InitStructure.periph_addr = (unsigned int)(SPI_DATA(SPI2));
    DMA_InitStructure.direction = DMA_PERIPHERAL_TO_MEMORY;
    DMA_InitStructure.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    DMA_InitStructure.periph_width = 0;
    DMA_InitStructure.memory_width = 0;
    DMA_InitStructure.priority = DMA_PRIORITY_ULTRA_HIGH;
    DMA_InitStructure.number = size;

    if(recv_addr != RT_NULL)
    {
        DMA_InitStructure.memory_addr = (unsigned int) recv_addr;
        DMA_InitStructure.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    }
    else
    {
        DMA_InitStructure.memory_addr = (unsigned int) (&dummy);
        DMA_InitStructure.memory_inc = DMA_MEMORY_INCREASE_DISABLE;
    }

    dma_init(DMA1,DMA_CH0, &DMA_InitStructure);

    dma_channel_enable(DMA1,DMA_CH0);

    /* TX channel configuration */
    dma_channel_disable(DMA1,DMA_CH1);
    DMA_InitStructure.periph_addr = (unsigned int)(SPI_DATA(SPI2));
    DMA_InitStructure.direction = DMA_MEMORY_TO_PERIPHERAL;
    DMA_InitStructure.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    DMA_InitStructure.periph_width = 0;
    DMA_InitStructure.memory_width = 0;
    DMA_InitStructure.priority = DMA_PRIORITY_MEDIUM;
    DMA_InitStructure.number  = size;

    if(send_addr != RT_NULL)
    {
        DMA_InitStructure.memory_addr = (unsigned int)send_addr;
        DMA_InitStructure.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    }
    else
    {
        DMA_InitStructure.memory_addr = (unsigned int)(&dummy);
        DMA_InitStructure.memory_inc = DMA_MEMORY_INCREASE_DISABLE;
    }

    dma_init(DMA1,DMA_CH1, &DMA_InitStructure);
    dma_channel_enable(DMA1,DMA_CH1);
}
#endif

rt_inline uint16_t get_spi_BaudRatePrescaler(rt_uint32_t max_hz)
{
    uint16_t SPI_BaudRatePrescaler;

    /* STM32F10x SPI MAX 18Mhz */
    if(max_hz >= SystemCoreClock/2 && SystemCoreClock/2 <= 18000000)
    {
        SPI_BaudRatePrescaler = SPI_PSC_2;
    }
    else if(max_hz >= SystemCoreClock/4)
    {
        SPI_BaudRatePrescaler = SPI_PSC_4;
    }
    else if(max_hz >= SystemCoreClock/8)
    {
        SPI_BaudRatePrescaler = SPI_PSC_8;
    }
    else if(max_hz >= SystemCoreClock/16)
    {
        SPI_BaudRatePrescaler = SPI_PSC_16;
    }
    else if(max_hz >= SystemCoreClock/32)
    {
        SPI_BaudRatePrescaler = SPI_PSC_32;
    }
    else if(max_hz >= SystemCoreClock/64)
    {
        SPI_BaudRatePrescaler = SPI_PSC_64;
    }
    else if(max_hz >= SystemCoreClock/128)
    {
        SPI_BaudRatePrescaler = SPI_PSC_128;
    }
    else
    {
        /* min prescaler 256 */
        SPI_BaudRatePrescaler = SPI_PSC_256;
    }
    return SPI_BaudRatePrescaler;
}

static rt_err_t configure(struct rt_spi_device* device, struct rt_spi_configuration* configuration)
{
//    struct stm32_spi * stm32_spi = (struct stm32_spi *)device->bus;
    spi_parameter_struct SPI_InitStructure;

    spi_struct_para_init(&SPI_InitStructure);

    /* data_width */
    if(configuration->data_width <= 8)
    {
        SPI_InitStructure.frame_size = SPI_FRAMESIZE_8BIT;
    }
    else if(configuration->data_width <= 16)
    {
        SPI_InitStructure.frame_size = SPI_FRAMESIZE_16BIT;
    }
    else
    {
        return RT_EIO;
    }
    /* baudrate */
    SPI_InitStructure.prescale = get_spi_BaudRatePrescaler(configuration->max_hz);
    /* CPOL */
    if(configuration->mode & RT_SPI_CPOL)
    {
        /* CPHA */
        if(configuration->mode & RT_SPI_CPHA)
        {
            SPI_InitStructure.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
        }
        else
        {
            SPI_InitStructure.clock_polarity_phase = SPI_CK_PL_HIGH_PH_1EDGE;
        }
    }
    else
    {
            /* CPHA */
        if(configuration->mode & RT_SPI_CPHA)
        {
            SPI_InitStructure.clock_polarity_phase = SPI_CK_PL_LOW_PH_2EDGE;
        }
        else
        {
            SPI_InitStructure.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
        }
    }

    /* MSB or LSB */
    if(configuration->mode & RT_SPI_MSB)
    {
        SPI_InitStructure.endian= SPI_ENDIAN_MSB;
    }
    else
    {
        SPI_InitStructure.endian = SPI_ENDIAN_LSB;
    }
    SPI_InitStructure.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    SPI_InitStructure.device_mode = SPI_MASTER;
    SPI_InitStructure.nss  = SPI_NSS_SOFT;
    /* init SPI */
    spi_i2s_deinit(SPI2);
    spi_init(SPI2, &SPI_InitStructure);
    /* Enable SPI_MASTER */
    spi_enable(SPI2);
    spi_crc_off(SPI2);

    return RT_EOK;
};

static rt_uint32_t xfer(struct rt_spi_device* device, struct rt_spi_message* message)
{
    struct rt_spi_configuration * config = &device->config;
    rt_uint32_t size = message->length;
    /* take CS */
    if(message->cs_take)
    {
        gpio_bit_reset(GPIOA,GPIO_PIN_15);
    }

#ifdef SPI_USE_DMA
    if(message->length > 32)
    {
        if(config->data_width <= 8)
        {
            DMA_Configuration(message->send_buf, message->recv_buf, message->length);
            spi_dma_enable(SPI2,SPI_DMA_TRANSMIT|SPI_DMA_RECEIVE);
            while (dma_flag_get(DMA1,DMA_CH0,DMA_FLAG_FTF) == RESET
                   || dma_flag_get(DMA1,DMA_CH1,DMA_FLAG_FTF) == RESET);
            spi_dma_disable(SPI2,SPI_DMA_TRANSMIT|SPI_DMA_RECEIVE);
        }
    }
    else
#endif
    {
        if(config->data_width <= 8)
        {
            const rt_uint8_t * send_ptr = message->send_buf;
            rt_uint8_t * recv_ptr = message->recv_buf;

            while(size--)
            {
                rt_uint8_t data = 0xFF;

                if(send_ptr != RT_NULL)
                {
                    data = *send_ptr++;
                }

                //Wait until the transmit buffer is empty
                while (spi_i2s_flag_get(SPI2, SPI_FLAG_TBE) == RESET);
                // Send the byte
                spi_i2s_data_transmit(SPI2, data);

                //Wait until a data is received
                while (spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE) == RESET);
                // Get the received data
                data = spi_i2s_data_receive(SPI2);

                if(recv_ptr != RT_NULL)
                {
                    *recv_ptr++ = data;
                }
            }
        }
        else if(config->data_width <= 16)
        {
            const rt_uint16_t * send_ptr = message->send_buf;
            rt_uint16_t * recv_ptr = message->recv_buf;

            while(size--)
            {
                rt_uint16_t data = 0xFF;

                if(send_ptr != RT_NULL)
                {
                    data = *send_ptr++;
                }

                //Wait until the transmit buffer is empty
                while (spi_i2s_flag_get(SPI2, SPI_FLAG_TBE) == RESET);
                // Send the byte
                spi_i2s_data_transmit(SPI2, data);

                //Wait until a data is received
                while (spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE) == RESET);
                // Get the received data
                data = spi_i2s_data_receive(SPI2);

                if(recv_ptr != RT_NULL)
                {
                    *recv_ptr++ = data;
                }
            }
        }
    }

    /* release CS */
    if(message->cs_release)
    {
        gpio_bit_set(GPIOA, GPIO_PIN_15);
    }

    return message->length;
};

/** \brief init and register stm32 spi bus.
 *
 * \param SPI: STM32 SPI, e.g: SPI1,SPI2,SPI3.
 * \param stm32_spi: stm32 spi bus struct.
 * \param spi_bus_name: spi bus name, e.g: "spi1"
 * \return
 *
 */
rt_err_t stm32_spi_register(uint32_t SPI,
                            struct stm32_spi * stm32_spi,
                            const char * spi_bus_name)
{
    rcu_periph_clock_enable(RCU_AF);

    if(SPI == SPI2)
    {
        stm32_spi->SPI = SPI2;
        rcu_periph_clock_enable(RCU_SPI2);
    }
    else
    {
        return RT_ENOSYS;
    }

    return rt_spi_bus_register(&stm32_spi->parent, spi_bus_name, &stm32_spi_ops);
}

rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name)
{
    RT_ASSERT(bus_name != RT_NULL);
    RT_ASSERT(device_name != RT_NULL);

    rt_err_t result;
    struct rt_spi_device *spi_device;

    /* initialize the cs pin && select the slave*/
    rcu_periph_clock_enable(RCU_GPIOA);
    /* spi21: PG10 */
    
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_15);
    gpio_bit_set(GPIOA, GPIO_PIN_15);

    /* attach the device to spi bus*/
    spi_device = (struct rt_spi_device *)rt_malloc(sizeof(struct rt_spi_device));
    RT_ASSERT(spi_device != RT_NULL);
    result = rt_spi_bus_attach_device(spi_device, device_name, bus_name, NULL);

    if (result != RT_EOK)
    {
        //LOG_E("%s attach to %s faild, %d\n", device_name, bus_name, result);
    }

    RT_ASSERT(result == RT_EOK);
    //LOG_D("%s attach to %s done", device_name, bus_name);
    return result;
}


static void rt_hw_spi_init(void)
{
#ifdef RT_USING_SPI2
    static struct stm32_spi stm32_spi;
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_pin_remap_config(GPIO_SWJ_NONJTRST_REMAP,ENABLE);
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP,ENABLE) ;

    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_15);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
    stm32_spi_register(SPI2, &stm32_spi, "spi2");
    rt_hw_spi_device_attach("spi2", "spi30");
#endif /* RT_USING_SPI3 */
}


int rt_hw_spi_flash_with_sfud_init(void)
{
    rt_hw_spi_init();
    /* init w25q128 */
    if (RT_NULL == rt_sfud_flash_probe("norflash0", "spi30"))
    {
        return -RT_ERROR;
    }
    return RT_EOK;
}

void initFileSystem(void)
{
    struct rt_device *mtd_dev = RT_NULL;

    rt_hw_spi_flash_with_sfud_init();
    /* 初始化 fal */
    fal_init();    
    /* 生成 mtd 设备 */
    mtd_dev = fal_mtd_nor_device_create(FS_PARTITION_NAME);
    if (mtd_dev)
    {
        /* initialize the device file system */
        dfs_init();
        extern int dfs_lfs_init(void);
        dfs_lfs_init();
        /* 挂载 littlefs */
        if (dfs_mount(FS_PARTITION_NAME, "/", "lfs", 0, 0) != 0)
        {
            /* 格式化文件系统 */
            dfs_mkfs("lfs", FS_PARTITION_NAME);
            /* 挂载 littlefs */
            dfs_mount("filesystem", "/", "lfs", 0, 0);
        }
    }
}

