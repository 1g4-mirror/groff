// -*- C++ -*-

/* Copyright (C) 2016-  Free Software Foundation, Inc.
   Written by Bertrand Garrigues (bertrand.garrigues@laposte.net)

This file is part of groff.

groff is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of othe License, or
(at your option) any later version.

groff is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>. */

#include <stdbool.h>
#include <limits.h>
#include "list.h"
#include "paragraph_word.h"

#define PARAGRAPH_DEFAULT_TOLERANCE 1
#define PARAGRAPH_DEFAULT_HYPHENATION_PENALTY 50
#define PARAGRAPH_DEFAULT_LINE_WIDTH 500
#define PARAGRAPH_DEFAULT_NON_ADJACENT_FITNESS_DEMERITS 10000

/* Classes local to paragraph.cpp  */

/**
 * Abstract Class item
 * An item is either:
 *  - a box_item (contains a word).
 *  - a glue item (space between words).
 *  - a penalty item (used to put extra charges on less desirable way to break
 *    the paragraph).
 */
class item {
  
protected:
  paragraph_word *word_;
  unsigned int width_;
  unsigned int stretchability_;
  unsigned int shrinkability_;
  int penalty_;
  bool flagged_penalty_;
  bool is_breakpoint_;
   
public:
  struct list_head list_;
  item();
  virtual ~item();
  unsigned int get_width();
  unsigned int get_stretchability();
  unsigned int get_shrinkability();
  int get_penalty();
  item *get_previous_item();
  paragraph_word *get_word();
  int sprint_word(char *str);
  virtual bool is_box() = 0;
  virtual bool is_glue() = 0;
  virtual bool is_legal_breakpoint() = 0;
  virtual bool is_forced_break() = 0;
  virtual bool is_flagged_penalty() = 0;
  virtual int sprint_info(char *str) = 0;
};

/**
 * Class box_item
 *
 * A box contains a word and is not a legal breakpoint.
 */
class box_item : public item {
  
public:
  box_item(paragraph_word *word);
  ~box_item();
  bool is_box();
  bool is_glue();
  bool is_legal_breakpoint();
  bool is_forced_break();
  bool is_flagged_penalty();
  int sprint_info(char *str);
};

/**
 * Class glue_item
 *
 * The glue is the space between words.  It is a legal breakpoint if the
 * previous item is a box.
 */
class glue_item : public item {
public:
  glue_item (unsigned int width, unsigned int stretchability, unsigned int shrinkability);
  ~glue_item();
  bool is_box();
  bool is_glue();
  bool is_legal_breakpoint();
  bool is_forced_break();
  bool is_flagged_penalty();
  int sprint_info(char *str);
};

/**
 * Class penalty_item
 *
 * Penalties can have special values:
 * - minus infinity: it is a legaal and forced break point.
 * - plus infinity: it is a disabled (forbidden) break point.
 * Other values constitute a legal break point.
 */
class penalty_item : public item {
  paragraph_word *optional_word_;
public:
  penalty_item(int penalty,
               bool flagged_penalty,
               paragraph_word *optional_word);
  ~penalty_item();
  unsigned int get_width();
  bool is_box();
  bool is_glue();
  bool is_legal_breakpoint();
  bool is_forced_break();
  bool is_flagged_penalty();
  int sprint_info(char *str);
};

/**
  * Class breakpoint
  * 
  * A breakpoint:
  *  - points to an item (the place where to break)
  *  - points to the previous best breakpoint.
  *  - Stores the the total width, stretch, shrink from the start of the
  *    paragraph.
  */

/* Fitness class is used to avoid having, for example, a tight line
 * following a loose line.*/
typedef enum {
  FITNESS_CLASS_TIGHT,
  FITNESS_CLASS_NORMAL,
  FITNESS_CLASS_LOOSE,
  FITNESS_CLASS_VERY_LOOSE,
  FITNESS_CLASS_MAX
} fitness_class_t;

class breakpoint {
  int line_number_;
  float adjust_ratio_;
  fitness_class_t fitness_class_;
  unsigned int total_width_;
  unsigned int total_stretch_;
  unsigned int total_shrink_;
  unsigned int total_demerits_;
  item *break_item_;
  breakpoint *previous_best_;
  char *sz_print_;
  
protected:
  /* simple getters */
  unsigned int get_total_width();
  unsigned int get_total_stretch();
  unsigned int get_total_shrink();
  /* add the glue to the total */
  unsigned int get_total_width_after();
  unsigned int get_total_stretch_after();
  unsigned int get_total_shrink_after();

public:
  struct list_head list_;
  breakpoint(item *break_item,
             unsigned int total_width,
             unsigned int total_stretch,
             unsigned int total_shrink);
  ~breakpoint();
  /* simple getters and setters */
  item           *get_item();
  item           *get_previous_box();
  int             get_line_number();
  float           get_adjust_ratio();
  void            set_adjust_ratio(float adjust_ratio);
  unsigned int    get_total_demerits();
  void            set_total_demerits(unsigned int total_demerits);
  fitness_class_t get_fitness_class();
  void            set_fitness_class(fitness_class_t fitness_class);
  breakpoint     *get_previous_best();
  void            set_previous_best(breakpoint *previous);
  
  /* compute various values */
  float           compute_adjust_ratio(int desired_line_length,
                                       unsigned int total_width,
                                       unsigned int total_stretch,
                                       unsigned int total_shrink,
                                       item *candidate);
  float           compute_badness(float adjust_ratio);
  unsigned int    compute_demerits(float badness,
                                   item *candidate,
                                   bool use_old_demerits_formula = false);
  unsigned int    compute_adj_extra_demerits(fitness_class_t candidate_fitness);
  fitness_class_t compute_fitness_class(float adjust_ratio);

  int             sprint(char *str);
  const char     *print_breakpoint_info();
};
