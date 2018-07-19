#ifndef _BITS_PTHREADTYPES_ARCH_H
#define _BITS_PTHREADTYPES_ARCH_H	1

/* Determine the wordsize from the preprocessor defines.  */

#if defined __x86_64__ && !defined __ILP32__
# define __WORDSIZE     64
#else
# define __WORDSIZE     32
#define __WORDSIZE32_SIZE_ULONG         0
#define __WORDSIZE32_PTRDIFF_LONG       0
#endif

#ifdef __x86_64__
# define __WORDSIZE_TIME64_COMPAT32     1
/* Both x86-64 and x32 use the 64-bit system call interface.  */
# define __SYSCALL_WORDSIZE             64
#else
# define __WORDSIZE_TIME64_COMPAT32     0
#endif


#ifdef __x86_64__
# if __WORDSIZE == 64
#  define __SIZEOF_PTHREAD_MUTEX_T 40
#  define __SIZEOF_PTHREAD_ATTR_T 56
#  define __SIZEOF_PTHREAD_MUTEX_T 40
#  define __SIZEOF_PTHREAD_RWLOCK_T 56
#  define __SIZEOF_PTHREAD_BARRIER_T 32
# else
#  define __SIZEOF_PTHREAD_MUTEX_T 32
#  define __SIZEOF_PTHREAD_ATTR_T 32
#  define __SIZEOF_PTHREAD_MUTEX_T 32
#  define __SIZEOF_PTHREAD_RWLOCK_T 44
#  define __SIZEOF_PTHREAD_BARRIER_T 20
# endif
#else
# define __SIZEOF_PTHREAD_MUTEX_T 24
# define __SIZEOF_PTHREAD_ATTR_T 36
# define __SIZEOF_PTHREAD_MUTEX_T 24
# define __SIZEOF_PTHREAD_RWLOCK_T 32
# define __SIZEOF_PTHREAD_BARRIER_T 20
#endif
#define __SIZEOF_PTHREAD_MUTEXATTR_T 4
#define __SIZEOF_PTHREAD_COND_T 48
#define __SIZEOF_PTHREAD_CONDATTR_T 4
#define __SIZEOF_PTHREAD_RWLOCKATTR_T 8
#define __SIZEOF_PTHREAD_BARRIERATTR_T 4

/* Definitions for internal mutex struct.  */
#define __PTHREAD_COMPAT_PADDING_MID
#define __PTHREAD_COMPAT_PADDING_END
#define __PTHREAD_MUTEX_LOCK_ELISION    1
#ifdef __x86_64__
# define __PTHREAD_MUTEX_NUSERS_AFTER_KIND  0
# define __PTHREAD_MUTEX_USE_UNION          0
#else
# define __PTHREAD_MUTEX_NUSERS_AFTER_KIND  1
# define __PTHREAD_MUTEX_USE_UNION          1
#endif

#define __LOCK_ALIGNMENT
#define __ONCE_ALIGNMENT

struct __pthread_rwlock_arch_t
{
  unsigned int __readers;
  unsigned int __writers;
  unsigned int __wrphase_futex;
  unsigned int __writers_futex;
  unsigned int __pad3;
  unsigned int __pad4;
#ifdef __x86_64__
  int __cur_writer;
  int __shared;
  signed char __rwelision;
# ifdef  __ILP32__
  unsigned char __pad1[3];
#  define __PTHREAD_RWLOCK_ELISION_EXTRA 0, { 0, 0, 0 }
# else
  unsigned char __pad1[7];
#  define __PTHREAD_RWLOCK_ELISION_EXTRA 0, { 0, 0, 0, 0, 0, 0, 0 }
# endif
  unsigned long int __pad2;
  /* FLAGS must stay at this position in the structure to maintain
     binary compatibility.  */
  unsigned int __flags;
# define __PTHREAD_RWLOCK_INT_FLAGS_SHARED	1
#else
  /* FLAGS must stay at this position in the structure to maintain
     binary compatibility.  */
  unsigned char __flags;
  unsigned char __shared;
  signed char __rwelision;
# define __PTHREAD_RWLOCK_ELISION_EXTRA 0
  unsigned char __pad2;
  int __cur_writer;
#endif
};

#ifndef __x86_64__
/* Extra attributes for the cleanup functions.  */
# define __cleanup_fct_attribute __attribute__ ((__regparm__ (1)))
#endif

#endif	/* bits/pthreadtypes.h */





#ifndef _THREAD_SHARED_TYPES_H
#define _THREAD_SHARED_TYPES_H 1

/* Common definition of pthread_mutex_t. */

#if !__PTHREAD_MUTEX_USE_UNION
typedef struct __pthread_internal_list
{
  struct __pthread_internal_list *__prev;
  struct __pthread_internal_list *__next;
} __pthread_list_t;
#else
typedef struct __pthread_internal_slist
{
  struct __pthread_internal_slist *__next;
} __pthread_slist_t;
#endif

/* Lock elision support.  */
#if __PTHREAD_MUTEX_LOCK_ELISION
# if !__PTHREAD_MUTEX_USE_UNION
#  define __PTHREAD_SPINS_DATA	\
  short __spins;		\
  short __elision
#  define __PTHREAD_SPINS             0, 0
# else
#  define __PTHREAD_SPINS_DATA	\
  struct			\
  {				\
    short __espins;		\
    short __eelision;		\
  } __elision_data
#  define __PTHREAD_SPINS         { 0, 0 }
#  define __spins __elision_data.__espins
#  define __elision __elision_data.__eelision
# endif
#else
# define __PTHREAD_SPINS_DATA int __spins
/* Mutex __spins initializer used by PTHREAD_MUTEX_INITIALIZER.  */
# define __PTHREAD_SPINS 0
#endif

struct __pthread_mutex_s
{
  int __lock __LOCK_ALIGNMENT;
  unsigned int __count;
  int __owner;
#if !__PTHREAD_MUTEX_NUSERS_AFTER_KIND
  unsigned int __nusers;
#endif
  /* KIND must stay at this position in the structure to maintain
     binary compatibility with static initializers.  */
  int __kind;
  __PTHREAD_COMPAT_PADDING_MID
#if __PTHREAD_MUTEX_NUSERS_AFTER_KIND
  unsigned int __nusers;
#endif
#if !__PTHREAD_MUTEX_USE_UNION
  __PTHREAD_SPINS_DATA;
  __pthread_list_t __list;
# define __PTHREAD_MUTEX_HAVE_PREV      1
#else
  __extension__ union
  {
    __PTHREAD_SPINS_DATA;
    __pthread_slist_t __list;
  };
# define __PTHREAD_MUTEX_HAVE_PREV      0
#endif
  __PTHREAD_COMPAT_PADDING_END
};


/* Common definition of pthread_cond_t. */

struct __pthread_cond_s
{
  __extension__ union
  {
    __extension__ unsigned long long int __wseq;
    struct
    {
      unsigned int __low;
      unsigned int __high;
    } __wseq32;
  };
  __extension__ union
  {
    __extension__ unsigned long long int __g1_start;
    struct
    {
      unsigned int __low;
      unsigned int __high;
    } __g1_start32;
  };
  unsigned int __g_refs[2] __LOCK_ALIGNMENT;
  unsigned int __g_size[2];
  unsigned int __g1_orig_size;
  unsigned int __wrefs;
  unsigned int __g_signals[2];
};

#endif /* _THREAD_SHARED_TYPES_H  */





#ifndef _BITS_PTHREADTYPES_COMMON_H
# define _BITS_PTHREADTYPES_COMMON_H	1

/* Thread identifiers.  The structure of the attribute type is not
   exposed on purpose.  */
typedef unsigned long int pthread_t;


/* Data structures for mutex handling.  The structure of the attribute
   type is not exposed on purpose.  */
typedef union
{
  char __size[__SIZEOF_PTHREAD_MUTEXATTR_T];
  int __align;
} pthread_mutexattr_t;


/* Data structure for condition variable handling.  The structure of
   the attribute type is not exposed on purpose.  */
typedef union
{
  char __size[__SIZEOF_PTHREAD_CONDATTR_T];
  int __align;
} pthread_condattr_t;


/* Keys for thread-specific data */
typedef unsigned int pthread_key_t;


/* Once-only execution */
typedef int __ONCE_ALIGNMENT pthread_once_t;


union pthread_attr_t
{
  char __size[__SIZEOF_PTHREAD_ATTR_T];
  long int __align;
};
#ifndef __have_pthread_attr_t
typedef union pthread_attr_t pthread_attr_t;
# define __have_pthread_attr_t 1
#endif


typedef union
{
  struct __pthread_mutex_s __data;
  char __size[__SIZEOF_PTHREAD_MUTEX_T];
  long int __align;
} pthread_mutex_t;


typedef union
{
  struct __pthread_cond_s __data;
  char __size[__SIZEOF_PTHREAD_COND_T];
  __extension__ long long int __align;
} pthread_cond_t;


#if defined __USE_UNIX98 || defined __USE_XOPEN2K
/* Data structure for reader-writer lock variable handling.  The
   structure of the attribute type is deliberately not exposed.  */
typedef union
{
  struct __pthread_rwlock_arch_t __data;
  char __size[__SIZEOF_PTHREAD_RWLOCK_T];
  long int __align;
} pthread_rwlock_t;

typedef union
{
  char __size[__SIZEOF_PTHREAD_RWLOCKATTR_T];
  long int __align;
} pthread_rwlockattr_t;
#endif


#ifdef __USE_XOPEN2K
/* POSIX spinlock data type.  */
typedef volatile int pthread_spinlock_t;


/* POSIX barriers data type.  The structure of the type is
   deliberately not exposed.  */
typedef union
{
  char __size[__SIZEOF_PTHREAD_BARRIER_T];
  long int __align;
} pthread_barrier_t;

typedef union
{
  char __size[__SIZEOF_PTHREAD_BARRIERATTR_T];
  int __align;
} pthread_barrierattr_t;
#endif

#endif







#ifndef _PTHREAD_H_
#define _PTHREAD_H_

typedef int pthread_once_t;
typedef unsigned int pthread_key_t;
typedef unsigned long int pthread_t;

#endif // _PTHREAD_H_
