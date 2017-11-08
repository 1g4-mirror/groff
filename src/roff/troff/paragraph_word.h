// -*- C++ -*-

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

#ifndef PARAGRAPH_WORD_H
#define PARAGRAPH_WORD_H

class paragraph_word {
public:
  virtual ~paragraph_word() {};
  virtual unsigned int get_width() = 0;
  virtual int get_next_glue_values(unsigned int *width,
                                   unsigned int *stretchability,
                                   unsigned int *shrinkability) = 0;
  virtual void write() = 0;
  // for debug only
  virtual int snprint(char *str, size_t size) { return 0; };
};

#endif
