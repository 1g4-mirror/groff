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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>

#include "paragraph_l.h"
#include "paragraph.h"

#include "trace.h"
trace_use_category(item);
trace_use_category(breakpoint);
trace_use_category(paragraph);

#define item_debug(args...)                     \
  trace_debug(item, ##args)
#define item_info(args...) \
  trace_info(item, ##args)
#define item_error(args...) \
  trace_error(item, item, ##args)
#define breakpoint_debug(args...)                       \
  trace_debug(breakpoint, ##args)
#define breakpoint_info(args...) \
  trace_info(breakpoint, ##args)
#define breakpoint_error(args...) \
  trace_error(breakpoint, ##args)
#define paragraph_debug(args...) \
  trace_debug(paragraph, ##args)
#define paragraph_info(args...) \
  trace_info(paragraph, ##args)
#define paragraph_error(args...) \
  trace_error(paragraph, ##args)

#define PLUS_INFINITY (INT_MAX / 4)
#define MINUS_INFINITY (INT_MIN / 4)

/* strings used for debug */
#define STRING_MAX_SIZE 256

/*
 * Abstract Class item
 */
item::item()
{
  INIT_LIST_HEAD(&list_, this);
  is_breakpoint_ = false;
}

item::~item()
{
  if (word_ != NULL)
    delete word_;
}

unsigned int
item::get_width()
{
  return width_;
}

unsigned int
item::get_stretchability()
{
  return stretchability_;
}

unsigned int
item::get_shrinkability()
{
  return shrinkability_;
}

int
item::get_penalty()
{
  return penalty_;
}

item *
item::get_previous_item()
{
  return list_entry(list_.prev, item);
}

paragraph_word *
item::get_word()
{
  return word_;
}

int
item::sprint_word(char *str)
{
  int res = 0;

  if (is_box() == true) {
    if (word_ != NULL)
      res = word_->snprint(str, STRING_MAX_SIZE);
  } else if (is_glue() == true) {
    res = sprintf(str, "glue: width %u strech %u shrink %u",
                  width_, stretchability_, shrinkability_);
  } else {
    res = sprintf(str, "penalty: width %u value %u flag %d",
                  width_, penalty_, flagged_penalty_);
  }
  item_debug("sprint_word:%s:", str);

  return res;
}

/*
 * Class box
 */
box_item::box_item(paragraph_word *word)
  : item()
{
  char sz[STRING_MAX_SIZE] = "";

  word_ = word;
  stretchability_ = 0;
  shrinkability_ = 0;
  penalty_ = 0;
  flagged_penalty_ = false;
  width_ = word->get_width();
  word_->snprint(sz, STRING_MAX_SIZE);
  item_debug("new box:%s: width %u", sz, width_);
}

box_item::~box_item()
{
}


bool
box_item::is_box()
{
  return true;
}

bool
box_item::is_glue()
{
  return false;
}

bool
box_item::is_legal_breakpoint()
{
  return false;
}

bool
box_item::is_forced_break()
{
  return false;
}

bool
box_item::is_flagged_penalty()
{
  return false;
}

int
box_item::sprint_info(char *str)
{
  char sz[STRING_MAX_SIZE] = "";
  word_->snprint(sz, STRING_MAX_SIZE);
  return sprintf(str, "box '%s' (width %u)", sz, width_);
}


/**
 * Class glue
 */
glue_item::glue_item (unsigned int width,
                      unsigned int stretchability,
                      unsigned int shrinkability)
  : item()
{
  width_ = width;
  stretchability_ = stretchability;
  shrinkability_ = shrinkability;
  penalty_ = 0;
  flagged_penalty_ = false;
  word_ = NULL;
}

glue_item::~glue_item()
{
}

bool
glue_item::is_box()
{
  return false;
}

bool
glue_item::is_glue()
{
  return true;
}

bool
glue_item::is_legal_breakpoint()
{
  bool ret;
  item *previous_item;

  previous_item = get_previous_item();
  if (previous_item->is_box())
    ret = true;
  else
    ret = false;

  return ret;
}

bool
glue_item::is_forced_break()
{
  return false;
}

bool
glue_item::is_flagged_penalty()
{
  return false;
}

int
glue_item::sprint_info(char *str)
{
  char tmp[32];

  if (stretchability_ >= PLUS_INFINITY)
    sprintf(tmp, "infinity");
  else
    sprintf(tmp, "%u", stretchability_);

  return sprintf(str, "glue width:%u strecth:%s shrink:%u",
                 width_, tmp, shrinkability_);
}

/**
 * Class penalty
 *
 */
penalty_item::penalty_item(int penalty,
                           bool flag,
                           paragraph_word *optional_word = NULL)
  : item()
{
  stretchability_ = 0;
  shrinkability_ = 0;
  penalty_ = penalty;
  flagged_penalty_ = flag;
  word_ = optional_word;
  if (word_)
     width_ = word_->get_width();
  else
     width_ = 0;
}

penalty_item::~penalty_item()
{
}

bool
penalty_item::is_box()
{
  return false;
}

bool
penalty_item::is_glue()
{
  return false;
}

bool
penalty_item::is_legal_breakpoint()
{
  bool ret;

  /* +infinity is a disallowed break */
  if (penalty_ < PLUS_INFINITY)
      ret = true;
    else
      ret = false;

  return ret;
}

bool
penalty_item::is_forced_break()
{
  bool ret =  false;

  if (penalty_ == MINUS_INFINITY)
    ret = true;

  return ret;
}

bool
penalty_item::is_flagged_penalty()
{
  return flagged_penalty_;
}

int
penalty_item::sprint_info(char *str)
{
  char tmp[32];

  if (penalty_ >= PLUS_INFINITY)
    sprintf(tmp, "infinity");
  else if (penalty_ <= MINUS_INFINITY)
    sprintf(tmp, "-infinity");
  else
    sprintf(tmp, "%d", penalty_);

  return sprintf(str,
                 "penalty width:%u value:%s flag:%d",
                 width_,
                 tmp,
                 flagged_penalty_);
}

/**
 * Class breakpoint
 */
breakpoint::breakpoint(item *break_item,
                       unsigned int total_width,
                       unsigned int total_stretch,
                       unsigned int total_shrink)
{
  breakpoint_debug("New breakpoint, total_width %u", total_width);
  break_item_ = break_item;
  line_number_ = 0;
  fitness_class_ = FITNESS_CLASS_MAX;
  total_width_ = total_width;
  total_stretch_ = total_stretch;
  total_shrink_ = total_shrink;
  total_demerits_ = 0;
  previous_best_ = NULL;
  sz_print_ = NULL;
  INIT_LIST_HEAD(&list_, this);
}

breakpoint::~breakpoint()
{
  if (sz_print_)
    free(sz_print_);
}

unsigned int
breakpoint::get_total_width()
{
  return total_width_;
}

unsigned int
breakpoint::get_total_stretch()
{
  return total_stretch_;
}

unsigned int
breakpoint::get_total_shrink()
{
  return total_shrink_;
}

unsigned int
breakpoint::get_total_width_after()
{
  unsigned int total_width_after = total_width_;

  /* We must not add the width if the item just after is, for example, an
   * optional penalty, otherwise the computation of next line length will be
   * incorrect. */
  if (break_item_ != NULL && break_item_->get_penalty() == 0)
    total_width_after += break_item_->get_width();

  return total_width_after;
}

unsigned int
breakpoint::get_total_stretch_after()
{
  unsigned int total_stretch_after = total_stretch_;

  if (break_item_ != NULL)
    total_stretch_after += break_item_->get_stretchability();

  return total_stretch_after;
}

unsigned int
breakpoint::get_total_shrink_after()
{
  unsigned int total_shrink_after = total_shrink_;

  if (break_item_ != NULL)
    total_shrink_after += break_item_->get_shrinkability();

  return total_shrink_after;
}

item *
breakpoint::get_item()
{
  return break_item_;
}

item *
breakpoint::get_previous_box()
{
  item *i = break_item_;

  while (i != NULL) {
    if (i->is_box())
      break;
    i = list_entry(i->list_.prev, item);
  }

  return i;
}

int
breakpoint::get_line_number()
{
  return line_number_;
}

float
breakpoint::get_adjust_ratio()
{
  return adjust_ratio_;
}

void
breakpoint::set_adjust_ratio(float adjust_ratio)
{
  adjust_ratio_ = adjust_ratio;
}

unsigned int
breakpoint::get_total_demerits()
{
  return total_demerits_;
}

void
breakpoint::set_total_demerits(unsigned int total_demerits)
{
  total_demerits_ = total_demerits;
}

fitness_class_t
breakpoint::get_fitness_class()
{
  return fitness_class_;
}

void
breakpoint::set_fitness_class(fitness_class_t fitness_class)
{
  fitness_class_ = fitness_class;
}

breakpoint *
breakpoint::get_previous_best()
{
  return previous_best_;
}

void
breakpoint::set_previous_best(breakpoint *previous)
{
  previous_best_ = previous;
  line_number_ = previous->get_line_number() + 1;
}


/**
 * Compute adjustement ratio between this breakpoint against the current item,
 * given the 3 values of total width, stretch and shrink. candidate is used
 * only to get the penalty.
 */
float
breakpoint::compute_adjust_ratio(int desired_line_length,
                                 unsigned int total_width,
                                 unsigned int total_stretch,
                                 unsigned int total_shrink,
                                 item *candidate)
{
  float ratio = 0;
  int line_length;
  unsigned int line_stretch;
  unsigned int line_shrink;

  if (candidate == NULL) {
    breakpoint_error("candidate item null");
    return -1;
  }

  line_length = total_width - get_total_width_after();
#if TRACE_BREAKPOINT >= LEVEL_DEBUG
  char tmp[256];
  char tmp1[256];
  sprint(tmp);
  //TODO: penalties should be printed directly
  candidate->get_previous_item()->sprint_word(tmp1);
  breakpoint_debug("From active breakpoint after '%s' to candidate after '%s'",
                   tmp, tmp1);
  breakpoint_debug("  previous total width: %u",
                   get_total_width_after());
  breakpoint_debug("  line_length: %d",
                   line_length);
#endif

/* If the candidate break is a penalty item, its width should be added (think of
 * an optional hyphen) */
  if (candidate->get_penalty() > 0) {
    line_length += candidate->get_width();
  }

  if (line_length < desired_line_length) {
    line_stretch = total_stretch - get_total_stretch_after();
    breakpoint_debug("  line_stretch %u", line_stretch);
    if (line_stretch > 0)
      ratio = ((float)desired_line_length
               - (float)line_length) / (float)line_stretch;
    else
      ratio = FLT_MAX;
  } else if (line_length > desired_line_length) {
    line_shrink = total_shrink - get_total_shrink_after();
    breakpoint_debug("  line_shrink %u", line_shrink);
    if (line_shrink > 0)
      ratio = ((float)desired_line_length
               - (float)line_length) / (float)line_shrink;
    else
      ratio = FLT_MIN;
  }

  if (ratio >= PLUS_INFINITY)
    breakpoint_debug("  ratio: infinity");
  else
    breakpoint_debug("  ratio: %.3f", ratio);

  return ratio;
}

float
breakpoint::compute_badness(float adjust_ratio)
{
  float badness;

  if (adjust_ratio < -1)
    badness = FLT_MAX;
  else
    badness = 100 * fabsf(pow(adjust_ratio, 3));

  breakpoint_debug("badness %.3f", badness);

  return badness;
}

unsigned int
breakpoint::compute_demerits(float badness,
                             item *candidate,
                             bool use_old_demerits_formula)
{
  unsigned int demerits;
  int penalty;
  int additional_penalty;

  if (break_item_
      && break_item_->is_flagged_penalty()
      && candidate->is_flagged_penalty())
    additional_penalty = PLUS_INFINITY; //TODO maybe other value;
  else
    additional_penalty = 0;

  penalty = candidate->get_penalty();

  /* Badness is always positive so we can simply convert float to int by adding
   * 0.5 */
  if (penalty >= 0) {
    if (use_old_demerits_formula) {
          demerits = pow(1 + (unsigned int)(badness + 0.5) + penalty, 2)
            + additional_penalty;
    }
    else {
      demerits = pow(1 + (unsigned int)(badness + 0.5), 2)
        + pow(penalty, 2)
        + additional_penalty;
    }
  } else if (penalty <= MINUS_INFINITY) {
    demerits = pow(1 + (unsigned int)(badness + 0.5), 2) + additional_penalty;
  } else {
    demerits =
      powf(1 + (unsigned int)(badness + 0.5), 2)
      - pow(penalty, 2)
      + additional_penalty;
  }

  breakpoint_debug("badness %.3f penalty %d demerits %u",
                   badness, penalty, demerits);
  return demerits;
}


unsigned int
breakpoint::compute_adj_extra_demerits(fitness_class_t candidate_fitness)
{
  unsigned int extra_demerits = 0;

  switch (candidate_fitness) {
  case FITNESS_CLASS_TIGHT:
    if (fitness_class_ >= FITNESS_CLASS_LOOSE)
      extra_demerits = PARAGRAPH_DEFAULT_NON_ADJACENT_FITNESS_DEMERITS;
    break;
  case FITNESS_CLASS_NORMAL:
    if (fitness_class_ >= FITNESS_CLASS_VERY_LOOSE)
        extra_demerits = PARAGRAPH_DEFAULT_NON_ADJACENT_FITNESS_DEMERITS;
    break;
  case FITNESS_CLASS_LOOSE:
    if (fitness_class_ == FITNESS_CLASS_TIGHT)
      extra_demerits = PARAGRAPH_DEFAULT_NON_ADJACENT_FITNESS_DEMERITS;
    break;
  case FITNESS_CLASS_VERY_LOOSE:
    if (fitness_class_ <= FITNESS_CLASS_NORMAL)
      extra_demerits = PARAGRAPH_DEFAULT_NON_ADJACENT_FITNESS_DEMERITS;
  default:
    break; //NOTE: intial breakpoint has a FITNESS_CLASS of MAX,
  }

  return extra_demerits;
}


fitness_class_t
breakpoint::compute_fitness_class(float adjust_ratio)
{
  fitness_class_t fitness_class;

  if (adjust_ratio < (float) -0.5)
    fitness_class = FITNESS_CLASS_TIGHT;
  else if (adjust_ratio <= (float) 0.5)
    fitness_class = FITNESS_CLASS_NORMAL;
  else if (adjust_ratio <= (float) 1)
    fitness_class = FITNESS_CLASS_LOOSE;
  else
    fitness_class = FITNESS_CLASS_VERY_LOOSE;

  return fitness_class;
}

int
breakpoint::sprint(char *str)
{
  item *box;
  int res;

  if (break_item_ != NULL)
  {
    box = get_previous_box();
    res = box->sprint_word(str);
    if (break_item_->is_glue() == false) {
      char tmp[32];
      sprintf(tmp, " (penalty: %d)", break_item_->get_penalty());
      strcat(str, tmp);
    }
  }  else {
    res = sprintf(str, "initial breakpoint");
  }

  return res;
}


const char *
breakpoint::print_breakpoint_info()
{
  item *box;
  item *previous_breakpoint_box;
  char tmp[256] = "";
  char tmp1[256] = "";

  if (sz_print_ == NULL) {
    sz_print_ = (char *)calloc(512, sizeof(char));

    if (break_item_ != NULL)
    {
      box = get_previous_box();
      box->sprint_word(tmp);
    }  else {
      sprintf(tmp, "initial breakpoint");
    }

    if (previous_best_ != NULL) {
      previous_breakpoint_box = previous_best_->get_previous_box();
      if (previous_breakpoint_box != NULL)
        previous_breakpoint_box->sprint_word(tmp1);
      else
        sprintf(tmp1, "initial breakpoint");
      snprintf(sz_print_, 511, "From '%s' to '%s'\n"
               "  line: %d\n"
               "  ratio: %.3f\n"
               "  total_demerits: %u\n"
               "  fitness class: %d\n",
               tmp1, tmp,
               line_number_,
               adjust_ratio_,
               total_demerits_,
               fitness_class_);
    } else {
      sprintf(sz_print_, "Initial breakpoint\n");
    }
  }

  return sz_print_;
}

/**
 * Class paragraph
 */
paragraph::paragraph()
{
  breakpoint *b;

  error_item_ = NULL;
  tolerance_ = PARAGRAPH_DEFAULT_TOLERANCE;
  line_length_ = PARAGRAPH_DEFAULT_LINE_WIDTH;
  use_old_demerits_formula_ = false;
  use_fitness_class_ = true;
  hyphenation_penalty_ = PARAGRAPH_DEFAULT_HYPHENATION_PENALTY;
  INIT_LIST_HEAD(&item_list_head_, NULL);
  INIT_LIST_HEAD(&active_breaks_list_head_, NULL);
  INIT_LIST_HEAD(&passive_breaks_list_head_, NULL);
  array_best_breaks_ = NULL;
  /* Add the 1st breakpoint */
  b = new breakpoint(NULL, 0, 0, 0);
  list_add_tail(&b->list_, &active_breaks_list_head_);
}

paragraph::~paragraph()
{
  item *pos, *n;
  breakpoint *break_pos, *break_n;
  /* Walk through the list and remove and delete 1 by 1 all items */
  list_for_each_entry_safe(pos, n, &item_list_head_, list_, item) {
    list_del_init(&pos->list_);
    delete pos;
  }

  list_for_each_entry_safe(
    break_pos, break_n, &passive_breaks_list_head_, list_, breakpoint) {
    list_del_init(&break_pos->list_);
    delete break_pos;
  }

  list_for_each_entry_safe(
    break_pos, break_n, &active_breaks_list_head_, list_, breakpoint) {
    list_del_init(&break_pos->list_);
    delete break_pos;
  }

  if (array_best_breaks_)
    free(array_best_breaks_);
}

void
paragraph::config_use_old_demerits_formula()
{
  use_old_demerits_formula_ = true;
}

void
paragraph::config_no_fitness_class()
{
  use_fitness_class_ = false;
}

void
paragraph::config_hyphenation_penalty(unsigned int value)
{
  hyphenation_penalty_ = value;
}

void
paragraph::add_box(paragraph_word *word)
{
  box_item *b;

  b = new box_item(word);
  list_add_tail(&b->list_, &item_list_head_);
}

void
paragraph::add_glue()
{
  item *previous;
  unsigned int width = 0;
  unsigned int stretchability = 0;
  unsigned int shrinkability = 0;
  glue_item *g;
  previous = list_entry(item_list_head_.prev, item);
  while (previous->is_box() == false && previous != list_entry(item_list_head_.next, item))
    previous = previous->get_previous_item();
  if (previous != NULL) {
    paragraph_word *word;
    word = previous->get_word();
    if (word != NULL)
      word->get_next_glue_values(&width, &stretchability, &shrinkability);
  }
  g = new glue_item(width, stretchability, shrinkability);
  list_add_tail(&g->list_, &item_list_head_);
}

void
paragraph::add_optional_hyphen(paragraph_word *hyphen_sign)
{
  //hyphen sign have a width of 6
  penalty_item *p = new penalty_item(hyphenation_penalty_, true, hyphen_sign);
  list_add_tail(&p->list_, &item_list_head_);
}

void
paragraph::add_explicit_hyphen()
{
  penalty_item *p = new penalty_item(hyphenation_penalty_, true);
  list_add_tail(&p->list_, &item_list_head_);
}

/*
 * If the last item is glue, remove it and add the finishing pattern:
 * - disallowed break: penalty(0, PLUS_INFINITY, 0)
 * - finishing glue: glue (0, PLUS_INFINITY, 0)
 * - forced break: penalty(0, MINUS_INFINITY, 0)
 */
void
paragraph::finish()
{
  glue_item *last_glue;
  penalty_item *disallowed_break_penalty = new penalty_item(PLUS_INFINITY, false);
  glue_item *finishing_glue = new glue_item(0, PLUS_INFINITY, 0);
  penalty_item *forced_break_penalty = new penalty_item(MINUS_INFINITY, false);

  /* FIXME we assume here that we always finish with glue so we always remove
   * it.  This should be more robust. */
  last_glue = list_entry(item_list_head_.prev, glue_item);
  list_del_init(item_list_head_.prev);
  delete(last_glue);
  list_add_tail(&disallowed_break_penalty->list_, &item_list_head_);
  list_add_tail(&finishing_glue->list_, &item_list_head_);
  list_add_tail(&forced_break_penalty->list_, &item_list_head_);
}


/**
 * Move a breakpoint from active list to passive list.
 */
void
paragraph::deactivate_breakpoint(breakpoint *active)
{
#if TRACE_PARAGRAPH >= LEVEL_DEBUG
  char str[256];
  active->sprint(str);
  paragraph_debug("  deactivating '%s'", str);
#endif

  list_del_init(&active->list_);
  list_add_tail(&active->list_, &passive_breaks_list_head_);
}

void
paragraph::record_feasible_break(breakpoint *active,
                                 breakpoint *candidate)
{
#if TRACE_PARAGRAPH >= LEVEL_DEBUG
  char str[256];
  active->sprint(str);
  paragraph_debug("   record feasible break '%s'", str);
#endif

  candidate->set_previous_best(active);
  if (list_empty(&candidate->list_))
    list_add_tail(&candidate->list_, &active_breaks_list_head_);
}

/*
 * Format the paragraph with Knuth-Plass algorithm. Algorithm general outline:

   for (all items 'b' of the paragraph) {
     if (item 'b' is a legal breakpoint) {
       for (all active breakpoint 'a') {
         compute the adjustment ratio 'r' from 'a' to 'b'
         if (r < -1) or (b is a forced break) {
             deactivate 'a' (a is now too far behind)
         }
         if ( -r <= r < tolerance threshold) {
             record feasible break from 'a' to 'b'
             comptude demerits and fitness class.
         }
       }
       if ('b' is a feasible breakpoint) {
         append the best such break as active node (chose the best active node)
       }
     }
   }
*/

// return 0 if OK
int
paragraph::format_knuth_plass(float tolerance,
                              unsigned int line_length)
{
  item *k_item;
  float adjust_ratio;
  breakpoint *active, *candidate, *n, *initial_breakpoint;
  unsigned int total_width = 0, total_stretch = 0, total_shrink = 0;
  float badness;
  unsigned int demerits;
  fitness_class_t fitness_class = FITNESS_CLASS_TIGHT;
  int k;
  breakpoint **tab_best_previous;
  unsigned int *tab_total_best_demerits;
  float *tab_ajust_ratio; // This is merely used for printing
  int n_fitness_class;
  unsigned int min_total_best_demerits;
#if TRACE_PARAGRAPH >= LEVEL_INFO
  char tmp[128] = "";
  char tmp1[128] = "";
#endif

  error_item_ = NULL;
  if (use_fitness_class_)
    n_fitness_class = FITNESS_CLASS_MAX;
  else
    n_fitness_class = 1;
  tab_best_previous = (breakpoint **)calloc(n_fitness_class,
                                             sizeof(breakpoint *));
  tab_total_best_demerits = (unsigned int *)calloc(n_fitness_class,
                                                   sizeof(unsigned int));
  tab_ajust_ratio = (float *)calloc(n_fitness_class, sizeof(float));
  tolerance_ = tolerance;
  line_length_ = line_length;

  /* Walk through all the items of the paragraph */
  list_for_each_entry(k_item, &item_list_head_, list_, item) {
#if TRACE_PARAGRAPH >= LEVEL_INFO
    k_item->sprint_info(tmp);
    paragraph_debug("Loop: total width %u total strecth %u total shrink %u",
                    total_width, total_stretch, total_shrink);
    paragraph_debug("  New item: %s", tmp);
#endif
    if (k_item->is_legal_breakpoint()) {
      min_total_best_demerits = PLUS_INFINITY;
      for (k = 0; k < n_fitness_class; k++) {
        tab_best_previous[k] = NULL;
        tab_total_best_demerits[k] = PLUS_INFINITY;
        tab_ajust_ratio[k] = PLUS_INFINITY;
      }

      /* Check the candidate breakpoint against each active breakpoint */
      list_for_each_entry_safe(
        active, n, &active_breaks_list_head_, list_, breakpoint) {
        adjust_ratio = active->compute_adjust_ratio(line_length_,
                                                    total_width,
                                                    total_stretch,
                                                    total_shrink,
                                                    k_item);
        if (k_item->is_forced_break() || adjust_ratio < -1) {
          deactivate_breakpoint(active);
        }
        if (adjust_ratio >= -1 && adjust_ratio < tolerance_) {
          /* there is a feasible break, remember of this breakpoint if the total
           * demerits from the active to the candidate is less than the previous
           * possible break. */
          badness = active->compute_badness(adjust_ratio);
          demerits = active->compute_demerits(badness,
                                              k_item,
                                              use_old_demerits_formula_);
          if (use_fitness_class_) {
            fitness_class = active->compute_fitness_class(adjust_ratio);
            demerits +=
              active->compute_adj_extra_demerits(fitness_class);
          }

#if TRACE_PARAGRAPH >= LEVEL_INFO
          paragraph_info("  From %s to %s:", tmp1, tmp);
          paragraph_info("    ratio          : %.3f", adjust_ratio);
          paragraph_info("    badness        : %.3f", badness);
          paragraph_info("    demerits       : %u", demerits);
          paragraph_info("    total_demerits : %u",
                         active->get_total_demerits() + demerits);
          paragraph_info("    fitness_class: %d", fitness_class);
#endif
          if (active->get_total_demerits() + demerits
              < tab_total_best_demerits[fitness_class]) {
            // possible break is best than previously
            tab_best_previous[fitness_class] = active;
            tab_total_best_demerits[fitness_class] =
              active->get_total_demerits() + demerits;
            tab_ajust_ratio[fitness_class] = adjust_ratio;
            if (tab_total_best_demerits[fitness_class] < min_total_best_demerits)
              min_total_best_demerits = tab_total_best_demerits[fitness_class];
          }
          paragraph_debug("  tab_best_previous[%d]:%p",
                          fitness_class,
                          tab_best_previous[fitness_class]);
        }
      }

      if (min_total_best_demerits < PLUS_INFINITY) {
        for (k = 0;
             k < n_fitness_class;
             k++) {
          paragraph_debug("  tab_total_best_demerits[%d]: %d min_total_best_demerits: %u",
                          k,
                          tab_total_best_demerits[k],
                          min_total_best_demerits);
          paragraph_debug("  tab_best_previous[%d]:%p",
                          k,
                          tab_best_previous[k]);

          if (tab_total_best_demerits[k] <= min_total_best_demerits
              + PARAGRAPH_DEFAULT_NON_ADJACENT_FITNESS_DEMERITS) {
            candidate = new breakpoint(k_item,
                                       total_width, total_stretch, total_shrink);
            candidate->set_total_demerits(tab_total_best_demerits[k]);
            candidate->set_adjust_ratio(tab_ajust_ratio[k]);
            candidate->set_fitness_class((fitness_class_t)k);
            record_feasible_break(tab_best_previous[k],
                                  candidate);
          }
        }
      }

      /* No more active breakpoint, leave with error */
      if (list_empty(&active_breaks_list_head_)) {
        paragraph_error("Could not format paragraph");
        error_item_ = k_item;
        break;
      }
    }
    /* compute total width */
    if (k_item->get_penalty() <= 0)
      total_width += k_item->get_width();
    total_stretch += k_item->get_stretchability();
    total_shrink += k_item->get_shrinkability();
  }

  /* There should be one breakpoint left, the final one; move it to the passive
     list */
  list_for_each_entry_safe(
    active, n, &active_breaks_list_head_, list_, breakpoint) {
    deactivate_breakpoint(active);
  }

  /* Create the list of best breakpoints by starting by the end of the passive
   * list */
  initial_breakpoint = list_entry(passive_breaks_list_head_.next, breakpoint);
  n = list_entry(passive_breaks_list_head_.prev, breakpoint);
  number_lines_ = n->get_line_number();
  if (number_lines_ == 0) {
     paragraph_error("Could not format paragraph");
     return -1;
  }
  array_best_breaks_ = (breakpoint **)calloc(number_lines_,
                                             sizeof(breakpoint *));
  k = 0;
  while (n != NULL && n != initial_breakpoint) {
    array_best_breaks_[number_lines_ - k - 1] = n;
    n = n->get_previous_best();
    k++;
  }

  if (tab_total_best_demerits)
    free(tab_total_best_demerits);
  if (tab_best_previous)
    free(tab_best_previous);
  if (tab_ajust_ratio)
    free(tab_ajust_ratio);

  if (error_item_ != NULL)
    return -1;
  else
    return 0;
}


int
paragraph::write_text(paragraph_writer_interface *pwi, int *number_lines)
{
  int res = 0;
  item *pos;
  breakpoint *next_feasible_breakpoint;
  breakpoint *next_best_breakpoint;
  int k, j_current_line;
  float ratio;
  float space_width;
  paragraph_word *word;

  if (pwi == NULL) {
    paragraph_error("Incorrect input");
    res = -1;
    if (number_lines)
      *number_lines = 0;
    goto end;
  }
  if (array_best_breaks_ == NULL) {
     res = -1;
     goto end;
  }
  k = 0;
  j_current_line = 1;
  next_best_breakpoint = array_best_breaks_[k];
  /* passive_breaks_list_head_.next is the initial breakpoint */
  next_feasible_breakpoint = list_entry(passive_breaks_list_head_.next->next,
                                        breakpoint);
  list_for_each_entry(pos, &item_list_head_, list_, item) {
    /* The item 'pos' correspond to a breakpoint */
    if (next_best_breakpoint && pos == next_best_breakpoint->get_item()) {
      word = pos->get_word();
      if (word != NULL) // case of a hyphen
        pwi->write_word_cbk(word);
      pwi->break_here_cbk(j_current_line);
      j_current_line++;
      if (k < number_lines_ - 1) {
        k++;
        next_best_breakpoint = array_best_breaks_[k];
      }
    } else if (pos == error_item_) {
      /* nothing else can be printed, exit */
      res = -1;
      goto end;
    } else {
      if (pos->is_box()) { // case of a word
        word = pos->get_word(); // FIXME maybe
        /*        if (strcmp(word, "") == 0) // a space box used for indentation
          pwi->write_space_cbk((float)pos->get_width());
          else*/
        pwi->write_word_cbk(word);
      } else if (pos->is_glue()) {
        /* Case of glue, that is, space: we must compute the space width */
        if (next_best_breakpoint != NULL) {
          ratio = next_best_breakpoint->get_adjust_ratio();
          if (ratio >=0)
            space_width = pos->get_width() + (pos->get_stretchability() * ratio);
          else
            space_width = pos->get_width() - (pos->get_shrinkability() * ratio);
          pwi->write_space_cbk(space_width);
        } else {
          /* case where the format_knuth function failed, we set the space width
           * to 1 so as this can still be used by the paragraph_printer class */
          pwi->write_space_cbk(1);
        }
      }
    }
  }

end:
  if (number_lines)
    *number_lines = k + 1;

  return res;
}

int
paragraph::get_number_of_lines()
{
  return number_lines_;
}

// return FLT_MAX if line not found
float
paragraph::get_adjust_ratio(int line_number)
{
  float ratio = FLT_MAX;

  if (line_number <= number_lines_ && line_number >= 0)
    ratio = array_best_breaks_[line_number - 1]->get_adjust_ratio();

  return ratio;
}

/* Returns fitness class of a line, or FITNESS_CLASS_MAX if incorrect line was
 * given as parameter */
fitness_class_t
paragraph::get_fitness_class(int line_number)
{
  fitness_class_t fitness_class = FITNESS_CLASS_MAX;

  if (line_number <= number_lines_ && line_number >= 0)
    fitness_class = array_best_breaks_[line_number - 1]->get_fitness_class();

  return fitness_class;
}


unsigned int
paragraph::get_total_demerits(int line_number)
{
  unsigned int total_demerits = PLUS_INFINITY;

  if (line_number <= number_lines_ && line_number >= 0)
    total_demerits = array_best_breaks_[line_number - 1]->get_total_demerits();

  return total_demerits;
}

void paragraph::print_breakpoints()
{
  breakpoint *pos;

  list_for_each_entry(pos, &passive_breaks_list_head_, list_, breakpoint) {
    printf("%s", pos->print_breakpoint_info());
  }
}

struct breakpoint_mark {
  struct list_head list_;
  int offset;
};

paragraph_printer::paragraph_printer(paragraph *par)
{
  par_ = par;
  current_index_ = 0;
  size_to_print_ = 0;
  max_line_length_ = 0;
  n_lines_ = par_->get_number_of_lines();
  /* We reserve for 1 more lines than the actual number of lines of the
   * paragraph: in case format_knuth failed to format the paragraph the last
   * lines (where the failure arose) would not be counted in the total number of
   * lines, but we still want to print it.*/
  tab_lines_ = (char **) calloc(n_lines_ + 1, sizeof(char *));
  tab_marks_ = (char **) calloc(n_lines_ + 1, sizeof(char *));
  tab_lines_[current_index_] = (char *) calloc(512, sizeof(char));
  tab_marks_[current_index_] = (char *) calloc(512, sizeof(char));
  /* passive_breaks_list_head_.next is the initial breakpoint */
  next_feasible_breakpoint_ =
    list_entry(par->passive_breaks_list_head_.next->next, breakpoint);
}

paragraph_printer::~paragraph_printer()
{
  int k;
  for (k = 0; k < par_->get_number_of_lines() + 1; k++) {
    free (tab_lines_[k]);
    free (tab_marks_[k]);
  }
  free(tab_lines_);
  free(tab_marks_);
}

void
paragraph_printer::new_line(void)
{
  if (current_index_ < par_->get_number_of_lines() - 1) {
    current_index_++;
    tab_lines_[current_index_] = (char *) calloc(512, sizeof(char));
    tab_marks_[current_index_] = (char *) calloc(512, sizeof(char));
    size_to_print_ = 0;
  }
}

void
paragraph_printer::write_word_cbk(paragraph_word *word)
{
  char sz_word[256] = "";
  int len;
  int k;

  word->snprint(sz_word, 256);
  len = strlen(sz_word);
  strcat(tab_lines_[current_index_], sz_word);
  for (k = size_to_print_; k < size_to_print_ + len; k++)
    tab_marks_[current_index_][k] = ' ';
  size_to_print_ += len;
  if (next_feasible_breakpoint_
      && word == next_feasible_breakpoint_->get_previous_box()->get_word()) {
    tab_marks_[current_index_][k - 1] = '^';
    next_feasible_breakpoint_ =
      list_entry(next_feasible_breakpoint_->list_.next, breakpoint);
  }
}

int
paragraph_printer::write_space_cbk(float space_width)
{
  paragraph_debug("space: %.3f", space_width);
  strcat(tab_lines_[current_index_], " ");
  tab_marks_[current_index_][size_to_print_] = ' ';
  size_to_print_++;

  return 0;
}

int
paragraph_printer::break_here_cbk(int line_number)
{
  if (size_to_print_ > max_line_length_)
    max_line_length_ = size_to_print_;
  new_line();

  return 0;
}

/* Works properly with ascii text only */
int
paragraph_printer::print()
{
  int first_column_width;
  int k, res;
  int column1, column2, column3;
  int number_lines = 0;

  res = par_->write_text(this, &number_lines);
  if (res != 0)
     return res;
  first_column_width = printf("Number of lines: %d",
                              par_->get_number_of_lines());
  first_column_width += printf("%*s",
                               max_line_length_ + 5 - first_column_width, "|");
  column1 = printf(" adj. ratio |");
  column2 = printf(" total demerits |");
  column3 = printf(" fitness class |");
  printf("\n\n");

  for (k = 0; k < number_lines; k++) {
    printf("%-*s", first_column_width, tab_lines_[k]);
    printf("%*.3f  ",
           column1 - 2,
           par_->array_best_breaks_[k]->get_adjust_ratio());
    printf("%*u  ",
           column2 - 2,
           par_->array_best_breaks_[k]->get_total_demerits());
    printf("%*u  \n",
           column3 - 2,
           par_->array_best_breaks_[k]->get_fitness_class());
    printf("%s\n", tab_marks_[k]);
  }

  /* if write_text failed we must print the very last line that is not part of
   * the lines of formatted the paragraph */
  if (res != 0) {
    printf("\nCould not finish the formatting after line:\n\n");
    if (tab_lines_[number_lines] != NULL)
       printf("%s\n", tab_lines_[number_lines]);
  }
  return 0;
}
