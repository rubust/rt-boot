#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* RT-Thread Configuration */

/* RT-Thread Kernel */

#define RT_NAME_MAX 32
#define RT_ALIGN_SIZE 8
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 1000
//#define RT_DEBUG
#define RT_USING_OVERFLOW_CHECK
//#define RT_DEBUG_INIT 0
//#define RT_DEBUG_THREAD 0
//#define RT_USING_HOOK
#define IDLE_THREAD_STACK_SIZE 1024
#define RT_USING_CPU_FFS

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE
//#define RT_USING_SIGNALS

/* Memory Management */

#define RT_USING_MEMPOOL
#define RT_USING_MEMHEAP
#define RT_USING_SMALL_MEM
#define RT_USING_HEAP

/* Software timers Configuration */
#define RT_USING_TIMER_SOFT         1
#if RT_USING_TIMER_SOFT == 0
#undef RT_USING_TIMER_SOFT
#endif
#define RT_TIMER_THREAD_PRIO		4
#define RT_TIMER_THREAD_STACK_SIZE	512
#define RT_TIMER_TICK_PER_SECOND	100

/* Kernel Device Object */

#define RT_USING_DEVICE
#define RT_USING_INTERRUPT_INFO
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 128
#define RT_CONSOLE_DEVICE_NAME "uart-dev"

/* RT-Thread Components */

//#define RT_USING_COMPONENTS_INIT
//#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE 2048

/* C++ features */


/* Command shell */

#define RT_USING_FINSH
#define FINSH_USING_MSH
#define FINSH_USING_MSH_ONLY
#define FINSH_THREAD_PRIORITY       5
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES	        5
#define FINSH_THREAD_STACK_SIZE 	0x1000
#define FINSH_USING_DESCRIPTION
#define FINSH_USING_SYMTAB

/* Device virtual file system */

#define RT_USING_DFS
#define DFS_USING_WORKDIR
#define DFS_FILESYSTEMS_MAX 3
#define DFS_FILESYSTEM_TYPES_MAX 3
#define DFS_FD_MAX 6
//#define RT_USING_DFS_ELMFAT

/* elm-chan's FatFs, Generic FAT Filesystem Module */

//#define RT_DFS_ELM_CODE_PAGE 936
//#define RT_DFS_ELM_WORD_ACCESS
//#define RT_DFS_ELM_USE_LFN_0
//#define RT_DFS_ELM_USE_LFN 0
//#define RT_DFS_ELM_MAX_LFN 64
//#define RT_DFS_ELM_DRIVES 2
//#define RT_DFS_ELM_MAX_SECTOR_SIZE 512
//#define RT_DFS_ELM_REENTRANT

/* Device Drivers */

#define RT_USING_DEVICE_IPC
//#define RT_USING_SERIAL
//#define RT_USING_CAN
//#define RT_CAN_USING_HDR
//#define RT_USING_I2C
//#define RT_USING_I2C_BITOPS
//#define RT_USING_PIN
#define RT_USING_SPI
#define RT_USING_SFUD
#define RT_SFUD_USING_SFDP
//#define RT_SHOW_SFUD_INFO
#define RT_SFUD_USING_FLASH_INFO_TABLE
//#define RT_USING_SPI_MSD

/* Using USB */


/* POSIX layer and C standard library */

//#define RT_USING_LIBC
//#define RT_USING_PTHREADS

/* Network stack */

/* light weight TCP/IP stack */

#define RT_USING_LWIP
#define RT_USING_LWIP210
//#define RT_LWIP_IGMP
#define RT_LWIP_ICMP
//#define RT_LWIP_RAW
//#define RT_LWIP_DNS
//#define RT_LWIP_DHCP
#define LWIP_IP_ACCEPT_UDP_PORT(port) ((port) == 67)
#define IP_SOF_BROADCAST 1
#define IP_SOF_BROADCAST_RECV 1

/* Static IPv4 Address */

#define RT_LWIP_IPADDR "192.168.1.1"
#define RT_LWIP_GWADDR "192.168.1.1"
#define RT_LWIP_MSKADDR "255.255.255.0"
#define RT_LWIP_UDP
#define RT_LWIP_TCP
#define RT_MEMP_NUM_NETCONN 8
#define RT_LWIP_PBUF_NUM 4
#define RT_LWIP_RAW_PCB_NUM 4
#define RT_LWIP_UDP_PCB_NUM 4
#define RT_LWIP_TCP_PCB_NUM 3
#define RT_LWIP_TCP_SEG_NUM 40
#define RT_LWIP_TCP_SND_BUF 4096
#define RT_LWIP_TCP_WND 2048
#define RT_LWIP_TCPTHREAD_PRIORITY 12
#define RT_LWIP_TCPTHREAD_MBOX_SIZE 8
#define RT_LWIP_TCPTHREAD_STACKSIZE 4096
#define RT_LWIP_ETHTHREAD_PRIORITY 14
#define RT_LWIP_ETHTHREAD_STACKSIZE 4096
#define RT_LWIP_ETHTHREAD_MBOX_SIZE 8
#define LWIP_NETIF_STATUS_CALLBACK 1
#define SO_REUSE 1
#define LWIP_SO_RCVTIMEO 1
#define LWIP_SO_SNDTIMEO 1
#define LWIP_SO_RCVBUF 1
#define LWIP_NETIF_LOOPBACK 0

/* Modbus master and slave stack */


/* VBUS(Virtual Software BUS) */


/* Utilities */


/* RT-Thread online packages */

/* system packages */

/* RT-Thread GUI Engine */


/* IoT - internet of things */


/* Wi-Fi */

/* Marvell WiFi */


/* Wiced WiFi */


/* security packages */


/* language packages */


/* multimedia packages */


/* tools packages */


/* miscellaneous packages */

#define TINY_CRYPT_MD5
#define TINY_CRYPT_AES
#define TINY_CRYPT_SHA1
#define TINY_CRYPT_SHA256
#define TINY_CRYPT_BASE64

#endif
