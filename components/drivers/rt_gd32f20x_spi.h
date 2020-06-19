#ifndef STM32_SPI_H_INCLUDED
#define STM32_SPI_H_INCLUDED

#include <rtdevice.h>
#include "gd32f20x.h"
#include "gd32f20x_spi.h"
#include "rtconfig.h"
#include <fal.h>
#include <dfs_fs.h>


/* 定义要使用的分区名字 */
#define FS_PARTITION_NAME              "filesystem"

#ifdef RT_USING_SPI2
#define USING_SPI2
#endif

#ifdef RT_USING_SPI_DMA
#define SPI_USE_DMA
#endif

struct stm32_spi
{
    struct rt_spi_bus parent;
    uint32_t SPI;
};

/* public function list */
rt_err_t stm32_spi_register(uint32_t SPI,
                            struct stm32_spi * stm32_spi,
                            const char * spi_bus_name);

rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name);
int rt_hw_spi_flash_with_sfud_init(void);
void initFileSystem(void);
#endif // STM32_SPI_H_INCLUDED
