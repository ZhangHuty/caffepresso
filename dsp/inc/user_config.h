/*
 * user_config.h
 *
 */

#ifndef _USER_CONFIG_H
#define _USER_CONFIG_H

// TODO: Get these directly from TI platform specific header files.
#ifdef DEVICE_K2H

#define NO_CORES 				8

#define L1_CACHE_LINE_SIZE		64

#elif defined(__KEYSTONE1)
#error "Not supported. Core is tied to Keystone II currently"
#define NUM_CORES 				8
#endif

// This core is responsible for all init
#define MASTER_CORE_ID			0
// HW semaphores for shared data coherency and synchronization
#define INIT_DONE_SEM	 		2
// semaphore for data synchronization. Same above semaphore can be used. Just to have simplicity we use different one.
#define SHARED_MEM_SEM			3

#endif /*_USER_CONFIG_H */