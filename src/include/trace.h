/* Copyright (C) 2016-  Free Software Foundation, Inc.
   Written by Bertrand Garrigues (bertrand.garrigues@laposte.net)

This file is part of groff.

groff is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or
(at your option) any later version.

groff is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>. */

#ifndef TRACE_H
#define TRACE_H

#define LEVEL_DEBUG 3
#define LEVEL_INFO  2
#define LEVEL_ERROR 1
#define LEVEL_QUIET 0

#include <stdio.h>

#ifndef TRACE_ALIGN_LENGTH
#define TRACE_ALIGN_LENGTH -1
#endif

// __FILE__ gives the full path of the file, we just want the file name.
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#ifndef DEACTIVATE_TRACES

#define trace_define_category(module_name, default_level) \
  int trace_category_level_##module_name = default_level

#define trace_use_category(module_name) \
  extern int trace_category_level_##module_name

#define trace_set_level(module_name, level) \
  do { \
    trace_category_level_##module_name = level; \
  } while(0)

#define trace_print(module_name, min_level, fmt, args...)              \
  do {                                                                  \
    if ((trace_category_level_##module_name) >= (min_level)) {          \
      int res##__LINE__;                                                \
      int length_align##__LINE__ = TRACE_ALIGN_LENGTH;                  \
      res##__LINE__ = fprintf(stderr, "[%s:%d:%s]",  __FILENAME__, __LINE__, __FUNCTION__); \
      if (length_align##__LINE__ == -1)                                 \
        fprintf(stderr, fmt "\n", ##args);                               \
      else                                                              \
        fprintf(stderr, "%*s" fmt "\n", TRACE_ALIGN_LENGTH - res##__LINE__, #module_name": ", ##args); \
      fflush(stderr);                                                   \
    }                                                           \
  } while(0)

#define trace_debug(module_name, fmt, args...) \
  trace_print(module_name, LEVEL_DEBUG, fmt, ##args)

#define trace_info(module_name, fmt, args...) \
  trace_print(module_name, LEVEL_INFO, fmt, ##args)

#define trace_error(module_name, fmt, args...)       \
  trace_print(module_name, LEVEL_ERROR, fmt, ##args)

#else /* DEACTIVATE_TRACES */
#define trace_debug(args...)
#define trace_info(args...)
#define trace_error(args...)
#define trace_define_category(args...)
#define trace_set_level(args...)
#endif /* DEACTIVATE_TRACES */

#endif
