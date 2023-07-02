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

#include "list.h"
#include "paragraph_word.h"

class breakpoint;

/**
 * Class paragraph
 */
class paragraph_writer_interface {
public:
  virtual void write_word_cbk(paragraph_word *word) { word->write(); };
  virtual int write_space_cbk(float space_width) = 0;
  virtual int break_here_cbk(int line_number) = 0; //TODO : would be
                                                   //more useful to
                                                   //return the last
                                                   //word
};

class paragraph {
  friend class test_paragraph;
  friend class paragraph_printer;
  float tolerance_;
  unsigned int line_length_;
  // Only for test, using an original example from Knuth/Plass.
  bool use_old_demerits_formula_;
  bool use_fitness_class_;
  unsigned int hyphenation_penalty_;
  void deactivate_breakpoint(breakpoint *active);
  void record_feasible_break(breakpoint *active,
                             breakpoint *candidate);
  // Head of list of all the items of the paragraph
  struct list_head item_list_head_;
  // Head of lists of breakpoints
  struct list_head active_breaks_list_head_;
  struct list_head passive_breaks_list_head_;
  breakpoint **array_best_breaks_;
  int number_lines_;
  item *error_item_; // last item before exiting in error

public:
  paragraph();
  ~paragraph();
  // Configuration
  void config_use_old_demerits_formula();
  void config_no_fitness_class();
  void config_hyphenation_penalty(unsigned int value);

  // To build the paragraph
  // TODO: enable to add a custom item
  void add_box(paragraph_word *word);
  void add_glue();
  void add_optional_hyphen(paragraph_word *hyphen_sign);
  void add_explicit_hyphen();
  void finish();
  int
  format_knuth_plass(float tolerance = PARAGRAPH_DEFAULT_TOLERANCE,
                     unsigned int line_length = PARAGRAPH_DEFAULT_LINE_WIDTH);
  int write_text(paragraph_writer_interface *pwi, int *number_lines);
  // For stats
  int get_number_of_lines();
  float get_adjust_ratio(int line_number); // for stats
  unsigned int get_total_demerits(int line_number);
  fitness_class_t get_fitness_class(int line_number);
  void print_breakpoints(); // for debug only
};

/* A simple helper class to print a paragraph with the main information and
 * feasible breakpoints */
class paragraph_printer : public paragraph_writer_interface {
  paragraph *par_;
  char **tab_lines_;
  int n_lines_;
  char **tab_marks_;
  int size_to_print_;
  int max_line_length_;
  int current_index_;
  breakpoint *next_feasible_breakpoint_;
  void new_line(void);

public:
  paragraph_printer(paragraph *par);
  ~paragraph_printer();
  void write_word_cbk(paragraph_word *word);
  int write_space_cbk(float space_width);
  int break_here_cbk(int line_number);
  int print();
};
