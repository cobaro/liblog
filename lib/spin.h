// -*- mode: c -*-
#ifndef COBARO_LOG0_SPIN_H
#define COBARO_LOG0_SPIN_H

// COPYRIGHT_BEGIN
// Copyright (C) 2015, cobaro.org
// All rights reserved.
// COPYRIGHT_END

// This code is a very lightly modified version of that posted by
// Majek Majkowski at https://idea.popcount.org/
// 2012-09-12-reinventing-spinlocks/ and also available at GitHub
// https://github.com/majek/dump/blob/master/msqueue/
// pthread_spin_lock_shim.h

#include <errno.h>

#define UNUSED(x) (void)x;

typedef int pthread_spinlock_t;


#ifndef PTHREAD_PROCESS_SHARED
# define PTHREAD_PROCESS_SHARED (1)
#endif

#ifndef PTHREAD_PROCESS_PRIVATE
# define PTHREAD_PROCESS_PRIVATE (2)
#endif


static inline int
pthread_spin_init(
    pthread_spinlock_t *lock,
    int pshared)
{
    UNUSED(pshared);
	__asm__ __volatile__ ("" ::: "memory");
	*lock = 0;
	return 0;
}


static inline int
pthread_spin_destroy(
    pthread_spinlock_t *lock)
{
    UNUSED(lock);
	return 0;
}


static inline int
pthread_spin_lock(
    pthread_spinlock_t *lock)
{
	while (1) {
		for (int i = 0; i < 10000; i++) {
			if (__sync_bool_compare_and_swap(lock, 0, 1)) {
				return 0;
			}
		}
		sched_yield();
	}
}


static inline int
pthread_spin_trylock(
    pthread_spinlock_t *lock)
{
	if (__sync_bool_compare_and_swap(lock, 0, 1)) {
		return 0;
	}
	return EBUSY;
}


static inline int
pthread_spin_unlock(
    pthread_spinlock_t *lock)
{
	__asm__ __volatile__ ("" ::: "memory");
	*lock = 0;
	return 0;
}


#endif // COBARO_LOG0_SPIN_H
