/* RT-Thread config file */
#ifndef __RTTHREAD_CFG_H__
#define __RTTHREAD_CFG_H__

/* RT_NAME_MAX*/
#define RT_NAME_MAX	8

/* RT_ALIGN_SIZE*/
#define RT_ALIGN_SIZE	8

/* PRIORITY_MAX */
#define RT_THREAD_PRIORITY_MAX	32

/* Tick per Second */
#define RT_TICK_PER_SECOND	1000

#define RT_USING_OVERFLOW_CHECK

/* Using Hook */
#define RT_USING_HOOK

/* Using Software Timer */
/* #define RT_USING_TIMER_SOFT */
//#define RT_TIMER_THREAD_PRIO		4
//#define RT_TIMER_THREAD_STACK_SIZE	512

/* SECTION: IPC */
/* Using Semaphore*/
#define RT_USING_SEMAPHORE

/* Using Mutex */
#define RT_USING_MUTEX

/* Using Event */
//#define RT_USING_EVENT

/* Using MailBox */
#define RT_USING_MAILBOX

/* Using Message Queue */
//#define RT_USING_MESSAGEQUEUE

/* SECTION: Memory Management */
/* Using Memory Pool Management*/
//#define RT_USING_MEMPOOL

/* Using Dynamic Heap Management */
#define RT_USING_HEAP

/* Using Small MM */
#define RT_USING_SMALL_MEM
/* SECTION: Device System */
/* Using Device System */
#define RT_USING_DEVICE
// <bool name="RT_USING_DEVICE_IPC" description="Using device communication" default="true" />
//#define RT_USING_DEVICE_IPC
// <bool name="RT_USING_SERIAL" description="Using Serial" default="true" />
#define RT_USING_SERIAL
/* SECTION: Console options */
#define RT_USING_CONSOLE
/* the buffer size of console*/
#define RT_CONSOLEBUF_SIZE	128

// <string name="RT_CONSOLE_DEVICE_NAME" description="console device name" default="uart3" />
#define RT_CONSOLE_DEVICE_NAME	"uart0"

#define FINSH_THREAD_PRIORITY 16
// </section>

// <section name="RT_USING_COMPONENTS_INIT" description="Using components init" default="false" >
// #define RT_USING_COMPONENTS_INIT
// </section>

/* SECTION: finsh, a C-Express shell */
#define RT_USING_FINSH
/* Using symbol table */
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
//#define FINSH_USING_AUTH

#define RT_USING_LIBC
/* system packages */

#define PKG_USING_FAL
#define FAL_DEBUG_CONFIG
#define FAL_DEBUG 0
#define FAL_PART_HAS_TABLE_CFG
#define FAL_USING_SFUD_PORT
//#define FAL_USING_NOR_FLASH_DEV_NAME "W25Q64"
#define PKG_USING_FAL_LATEST_VERSION
#define PKG_FAL_VER_NUM 0x99999
#define PKG_USING_LITTLEFS
#define PKG_USING_LITTLEFS_V214
#define LFS_READ_SIZE 4096
#define LFS_PROG_SIZE 4096
#define LFS_BLOCK_SIZE 4096
#define LFS_CACHE_SIZE 4096
#define LFS_BLOCK_CYCLES 100
#define LFS_LOOKAHEAD 512

/* Device Drivers */
#define RT_USING_SPI
#define RT_USING_SPI2
//#define RT_USING_SPI_DMA

#define RT_USING_MTD_NOR
#define RT_USING_SFUD
#define RT_SFUD_USING_FLASH_INFO_TABLE
//#define RT_SFUD_USING_QSPI
#define RT_SFUD_SPI_MAX_HZ 50000000




#endif
