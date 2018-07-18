/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _SYS_TIME_H
#define _SYS_TIME_H	1

typedef long __suseconds_t;

#ifndef __time_t_defined
#define __time_t_defined 1

/* Returned by `time'.  */
typedef __time_t time_t;

#endif

#ifndef __timeval_defined
#define __timeval_defined 1

/* A time value that is accurate to the nearest
   microsecond but also has a range of years.  */
struct timeval
{
  __time_t tv_sec;		/* Seconds.  */
  __suseconds_t tv_usec;	/* Microseconds.  */
};
#endif


#ifndef __suseconds_t_defined
typedef __suseconds_t suseconds_t;
# define __suseconds_t_defined
#endif

#endif /* sys/time.h */
