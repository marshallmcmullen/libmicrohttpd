/*
  This file is part of libmicrohttpd
  Copyright (C) 2016 Karlson2k (Evgeny Grin)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

/**
 * @file microhttpd/mhd_locks.h
 * @brief  Header for platform-independent locks abstraction
 * @author Karlson2k (Evgeny Grin)
 * @author Christian Grothoff
 *
 * Provides basic abstraction for locks/mutex and semaphores.
 * Any functions can be implemented as macro on some platforms
 * unless explicitly marked otherwise.
 * Any function argument can be skipped in macro, so avoid
 * variable modification in function parameters.
 *
 * @warning Unlike pthread functions, most of functions return
 *          nonzero on success.
 */

#ifndef MHD_LOCKS_H
#define MHD_LOCKS_H 1

#include "mhd_options.h"

#if defined(MHD_USE_W32_THREADS)
#  define MHD_W32_MUTEX_ 1
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN 1
#  endif /* !WIN32_LEAN_AND_MEAN */
#  include <windows.h>
#elif defined(HAVE_PTHREAD_H) && defined(MHD_USE_POSIX_THREADS)
#  define MHD_PTHREAD_MUTEX_ 1
#  undef HAVE_CONFIG_H
#  include <pthread.h>
#  define HAVE_CONFIG_H 1
#else
#  error No base mutex API is available.
#endif

#if defined(MHD_PTHREAD_MUTEX_)
  typedef pthread_mutex_t MHD_mutex_;
#elif defined(MHD_W32_MUTEX_)
  typedef CRITICAL_SECTION MHD_mutex_;
#endif

#if defined(MHD_PTHREAD_MUTEX_)
/**
 * Initialise new mutex.
 * @param pmutex pointer to the mutex
 * @return nonzero on success, zero otherwise
 */
#define MHD_mutex_init_(pmutex) (!(pthread_mutex_init((pmutex), NULL)))
#elif defined(MHD_W32_MUTEX_)
/**
 * Initialise new mutex.
 * @param pmutex pointer to mutex
 * @return nonzero on success, zero otherwise
 */
#define MHD_mutex_init_(pmutex) (InitializeCriticalSectionAndSpinCount((pmutex),16))
#endif

#if defined(MHD_PTHREAD_MUTEX_)
/**
 * Destroy previously initialised mutex.
 * @param pmutex pointer to mutex
 */
#define MHD_mutex_destroy_(pmutex) do { \
  if ( (0 != pthread_mutex_destroy((pmutex))) && \
       (EAGAIN != errno) && \
       (EINPROGRESS != errno) )  \
    MHD_PANIC (_("Failed to destroy mutex\n")); \
  } while (0)
#elif defined(MHD_W32_MUTEX_)
/**
 * Destroy previously initialised mutex.
 * @param pmutex pointer to mutex
 */
#define MHD_mutex_destroy_(pmutex) DeleteCriticalSection((pmutex))
#endif

#if defined(MHD_PTHREAD_MUTEX_)
/**
 * Acquire lock on previously initialised mutex.
 * If mutex was already locked by other thread, function
 * blocks until mutex becomes available.
 * @param pmutex pointer to mutex
 */
#define MHD_mutex_lock_(pmutex) do { \
  if (0 != pthread_mutex_lock((pmutex)))   \
    MHD_PANIC (_("Failed to lock mutex\n")); \
  } while (0)
#elif defined(MHD_W32_MUTEX_)
/**
 * Acquire lock on previously initialised mutex.
 * If mutex was already locked by other thread, function
 * blocks until mutex becomes available.
 * @param pmutex pointer to mutex
 */
#define MHD_mutex_lock_(pmutex) EnterCriticalSection((pmutex))
#endif

#if defined(MHD_PTHREAD_MUTEX_)
/**
 * Unlock previously initialised and locked mutex.
 * @param pmutex pointer to mutex
 * @return nonzero on success, zero otherwise
 */
#define MHD_mutex_unlock_(pmutex) do { \
    if (0 != pthread_mutex_unlock((pmutex)))   \
    MHD_PANIC (_("Failed to unlock mutex\n")); \
  } while (0)
#elif defined(MHD_W32_MUTEX_)
/**
 * Unlock previously initialised and locked mutex.
 * @param pmutex pointer to mutex
 * @return Always nonzero
 */
#define MHD_mutex_unlock_(pmutex) LeaveCriticalSection((pmutex))
#endif


/**
 * A semaphore.
 */
struct MHD_Semaphore;


/**
 * Create a semaphore with an initial counter of @a init
 *
 * @param init initial counter
 * @return the semaphore, NULL on error
 */
struct MHD_Semaphore *
MHD_semaphore_create (unsigned int init);


/**
 * Count down the semaphore, block if necessary.
 *
 * @param sem semaphore to count down.
 */
void
MHD_semaphore_down (struct MHD_Semaphore *sem);


/**
 * Increment the semaphore.
 *
 * @param sem semaphore to increment.
 */
void
MHD_semaphore_up (struct MHD_Semaphore *sem);


/**
 * Destroys the semaphore.
 *
 * @param sem semaphore to destroy.
 */
void
MHD_semaphore_destroy (struct MHD_Semaphore *sem);


#endif /* ! MHD_LOCKS_H */
