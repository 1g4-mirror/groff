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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

#include "paragraph_l.h"
#include "paragraph.h"

#define TRACE_ALIGN_LENGTH 40
#include "trace.h"
trace_define_category(item, LEVEL_ERROR);
trace_define_category(breakpoint, LEVEL_ERROR);
trace_define_category(paragraph, LEVEL_ERROR);

#define TEST_FAIL 1
#define TEST_SUCCESS 0
#define test_assert(expression, fmt, args...) \
  (((expression) == 0) ? \
   printf("   * FAIL [%s:%d]: " fmt "\n", \
          __FUNCTION__, __LINE__, ##args ), TEST_FAIL : TEST_SUCCESS)

/* A simple implementation of paragraph_word, for ascii only. */
class ascii_paragraph_word : public paragraph_word {
private:
  /* Length of each letter as set in Knuth's original example in his
   * article "Breaking Paragraph Into Lines" */
  static unsigned int char_len_[256];
  static void init_char_len();
  static bool is_class_initialized_;
  char *sz_word_;
  unsigned int width_;
public:
  // width_ is set by the strlen of sz_word
  ascii_paragraph_word(const char *sz_word);
  ~ascii_paragraph_word();
  // overwrite the width calculated by the constructor, this is useful
  // to create a box for indentation (which are empty but with a width
  // > 0
  void set_width(unsigned int width);
  unsigned int get_width();
  int get_next_glue_values(unsigned int *width,
                           unsigned int *stretchability,
                           unsigned int *shrinkability);
  int snprint(char *str, size_t size);
  void write();
};

unsigned int ascii_paragraph_word::char_len_[256];
bool ascii_paragraph_word::is_class_initialized_;

void
ascii_paragraph_word::init_char_len()
{
  int k;
  // Unknown characters width is set to 10
  for (k = 0; k < 256; k++)
    char_len_[k] = 10;
  char_len_[' '] = 0; // we set the space width to 0 here to be able to quickly
                     // compute the length of a line without having to deal with
                     // the last space
  char_len_['a'] = 9;
  char_len_['b'] = 10;
  char_len_['c'] = 8;
  char_len_['d'] = 10;
  char_len_['e'] = 8;
  char_len_['f'] = 6;
  char_len_['g'] = 9;
  char_len_['h'] = 10;
  char_len_['i'] = 5;
  char_len_['j'] = 6;
  char_len_['k'] = 10;
  char_len_['l'] = 5;
  char_len_['m'] = 15;
  char_len_['n'] = 10;
  char_len_['o'] = 9;
  char_len_['p'] = 10;
  char_len_['q'] = 10;
  char_len_['r'] = 7;
  char_len_['s'] = 7;
  char_len_['t'] = 7;
  char_len_['u'] = 10;
  char_len_['v'] = 9;
  char_len_['w'] = 13;
  char_len_['x'] = 10;
  char_len_['y'] = 10;
  char_len_['z'] = 8;
  char_len_['C'] = 13;
  char_len_['I'] = 6;

  char_len_['-'] = 6;
  char_len_[','] = 5;
  char_len_[';'] = 5;
  char_len_['.'] = 5;
  char_len_['\''] = 5;

  is_class_initialized_ = true ;
}

ascii_paragraph_word::ascii_paragraph_word(const char *sz_word)
{
  if (is_class_initialized_ == false)
    init_char_len();

  width_ = 0;
  if (sz_word) {
    size_t len;
    int k = 0;
    len = strlen(sz_word);
    sz_word_ = (char *)malloc(len + 1);
    strcpy(sz_word_, sz_word);
    while (sz_word_[k] != '\0') {
      if (sz_word_[k] >= 0)
        width_ += char_len_[(unsigned char) sz_word_[k]];
      k++;
    }
  }
}

ascii_paragraph_word::~ascii_paragraph_word()
{
  if (sz_word_)
    free(sz_word_);
}

void
ascii_paragraph_word::set_width(unsigned int width)
{
  width_ = width;
}
  
unsigned int
ascii_paragraph_word::get_width()
{
  return width_;
}

int
ascii_paragraph_word::get_next_glue_values(unsigned int *width,
                                           unsigned int *stretchability,
                                           unsigned int *shrinkability)
{
  size_t len;
  char last_character;
  
  if (sz_word_) {
    len = strlen(sz_word_);
    last_character = sz_word_[len - 1];
  } else {
    last_character = 0;
  }
  switch (last_character) {
  case ',':
    *width = 6;
    *stretchability = 4;
    *shrinkability = 2;
    break;
  case ';':
    *width = 6;
    *stretchability = 4;
    *shrinkability = 1;
    break;
  case '.':
    *width = 8;
    *stretchability = 6;
    *shrinkability = 1;
    break;
  default:
    *width = 6;
    *stretchability = 3;
    *shrinkability = 2;
    break;
  }
}

int
ascii_paragraph_word::snprint(char *str, size_t size)
{
  return snprintf(str, size, "%s", sz_word_);
}

void
ascii_paragraph_word::write()
{
  printf("%s", sz_word_);
}

/* Return -1 if the word cannot be hyphenate.  If it can, return the position in
   the word were the hyphenation can be done.  We just take all the words in the
   example paragraph that can be hyphenated. */
typedef enum {
  NO_HYPHEN,
  EXPLICIT_HYPHEN,
  OPTIONAL_HYPHEN
} hyphen_type_t;

class text_loader {
  char *text_; // Complete paragraph text in one string
  hyphen_type_t simulate_hyphenate (const char *word,
                                    unsigned int *first_part_len);
public:
  text_loader(char *text, const char *path = NULL);
  ~text_loader();
  void process_text(paragraph *par, bool with_indentation);
};

text_loader::text_loader(char *text, const char *path)
{
  if (text) {
    text_ = (char *)calloc(strlen(text) +1, sizeof (char));
    strcpy(text_, text);
  } else if (path) {
    FILE *fp;
    char *c;
    size_t text_size;
    struct stat buf;
    
    fp = fopen (path, "r");
    if (fp == NULL) {
      printf("Error:%s\n", strerror(errno));
      text_ = NULL;
    } else {
      stat(path, &buf);
      text_ = (char *) calloc((buf.st_size) + 1, sizeof(char));
      text_size = fread(text_, 1, buf.st_size, fp);
      for (c = text_; c < text_ + text_size; c++) {
        if (*c == '\n')
          *c = ' ';
      }
      fclose(fp);
    }
  }
}

text_loader::~text_loader()
{
  if (text_)
    free(text_);
}

hyphen_type_t
text_loader::simulate_hyphenate (const char *word, unsigned int *first_part_len)
{
  hyphen_type_t ret = OPTIONAL_HYPHEN;
  
  if (word == NULL || first_part_len == NULL)
    goto end;
  
  if (strncmp(word, "lime-tree", 9) == 0) {
    *first_part_len = 5;
    ret = EXPLICIT_HYPHEN;
    goto end;
  }
  
  if (strncmp(word, "wishing", 7) == 0)
    *first_part_len = 4;
  else if (strncmp(word, "daughters", 9) == 0)
    *first_part_len = 5;
  /* for the sake of simplicity we don't consider the break after beauti */
  else if (strncmp(word, "beautiful", 9) == 0)
    *first_part_len = 4;
  else if (strncmp(word, "youngest", 8) == 0)
    *first_part_len = 5;
  else if (strncmp(word, "itself", 6) == 0)
    *first_part_len = 2;
  else if (strncmp(word, "astonished", 10) == 0)
    *first_part_len = 5;
  else if (strncmp(word, "whenever", 8) == 0)
    *first_part_len = 4;
  else if (strncmp(word, "forest", 6) == 0)
    *first_part_len = 3;
  else if (strncmp(word, "under", 5) == 0)
    *first_part_len = 2;
  else if (strncmp(word, "lime-tree", 9) == 0)
    *first_part_len = 5;
  else if (strncmp(word, "fountain", 8) == 0)
    *first_part_len = 4;
  else if (strncmp(word, "favorite", 8) == 0)
    *first_part_len = 5;
  else if (strncmp(word, "plaything", 9) == 0)
    *first_part_len = 4;
  else if (strncmp(word, "hyphenationtest", 15) == 0)
    *first_part_len = 11;
  else
    ret = NO_HYPHEN;

end:
  return ret;
}

void
text_loader::process_text(paragraph *par, bool with_indentation)
{
  char *cur, *tmp;
  ascii_paragraph_word *cur_word, *indentation;
  unsigned int width;
  hyphen_type_t type;
  unsigned int hyphenate = 0;
  char previous_char;
  char *tmp_text;

  /* Add indentation width 18 */
  if (with_indentation) {
    indentation = new ascii_paragraph_word("   ");
    indentation->set_width(18);
    par->add_box(indentation);
  }
  
  /* Build the paragraph: for each word of 'text' we check if there is an
   * explicit hyphen (here only the word "lime-tree"), otherwise we add an
   * optional hyphen, and add the corresponding items. For example 'whenever'
   * has an optional hyphen (when-ever), so we:
   * - add a box of width 4 ('when').
   * - add an optional hyphen, which is a penalty item (penalty added only if
       the hyphen is chosen).
   * - add a box of width 4 ('ever').
   */
  tmp_text = (char *) calloc(strlen(text_) + 1, sizeof(char));
  strcpy(tmp_text, text_);
  cur = strtok(tmp_text, " ");
  while (cur != NULL) {
    type = simulate_hyphenate(cur, &hyphenate);
    if (type != NO_HYPHEN) {
      tmp = (char *) malloc(hyphenate + 1);
      strncpy(tmp, cur, hyphenate);
      tmp[hyphenate] = '\0';
      cur_word = new ascii_paragraph_word(tmp);
      par->add_box(cur_word);
      free(tmp);
      if (type == EXPLICIT_HYPHEN) {
        par->add_explicit_hyphen();
      } else {
        ascii_paragraph_word *hyphen_sign = new ascii_paragraph_word("-");
        par->add_optional_hyphen(hyphen_sign);
      }
      cur_word = new ascii_paragraph_word(cur + hyphenate);
      par->add_box(cur_word);
      par->add_glue();
    } else {
      cur_word = new ascii_paragraph_word(cur);
      par->add_box(cur_word);
      par->add_glue();
    }
    cur = strtok(NULL, " ");
  }
  /* Add final glue */
  par->finish();
  free(tmp_text);
}

struct expected_break_info {
  const char word[64];
  unsigned int demerit;
  unsigned int total_demerits;
};


#define PRINT_RESULT(n_failures)                \
  do {                                          \
    if (n_failures == 0)                                \
      printf("-- Test %s PASSED\n\n", __FUNCTION__);    \
    else                                                                \
      fprintf(stderr, "** Test %s FAILED, %d failures\n", __FUNCTION__, n_failures); \
  } while(0)

class test_paragraph {
  
private:
  text_loader *text_loader_;
  int check_all_breakpoint(struct expected_break_info tab_expected[],
                            paragraph *par);
  int check_best_breakpoint(struct expected_break_info tab_expected[],
                            paragraph *par);
  int check_best_breakpoint(const char*tab_expected[], paragraph *par);
public:
  test_paragraph();
  ~test_paragraph();
  void suite1_init();
  int test11_original_example();
  int test12_example_with_default_demerits_formula();
  int test13_example_with_larger_tolerance();
  
  void suite2_init();
  int test21_hyphen_flagged_penalty();

  void suite3_init();
  int test31_fitness_class();
};

test_paragraph::test_paragraph()
{
  text_loader_ = NULL;
}

/* Check all breakpoints list, we start at passive_breaks_list_head_.next
 * because the first breakpoint is the initial breakpoint */
int
test_paragraph::check_all_breakpoint(struct expected_break_info tab_expected[],
                                      paragraph *par)
{
  int res = 0;
  breakpoint *pos;
  unsigned int total_demerits;
  char word[256];
  int k = 0;
  item *i;

  list_for_each_entry(pos,
                      par->passive_breaks_list_head_.next,
                      list_,
                      breakpoint) {
    total_demerits = pos->get_total_demerits();
    res += test_assert(tab_expected[k].total_demerits == total_demerits,
                       "total demerits: expected: %u, actual: %u",
                       tab_expected[k].total_demerits,
                       total_demerits);
    i = pos->get_previous_box();
    if (i != NULL) {
      paragraph_word *pw = i->get_word();
      pw->snprint(word, 256);
      i = pos->get_item();
      res += test_assert(strcmp(tab_expected[k].word, word) == 0,
                         "expected: '%s' actual: '%s'",
                         tab_expected[k].word,
                         word);
    }
    k++;
  }

  return res;
}

/* Check best breakpoints list, the first breakpoint of best_breaks_list_head_
   is the breakpoint at the end of the 1st line, not the initial breakpoint
   Return the number of assert that failed or 0 if OK */
int
test_paragraph::check_best_breakpoint(struct expected_break_info tab_expected[],
                                      paragraph *par)
{
  int res = 0;
  unsigned int total_demerits;
  char word[256];
  int k = 0;
  item *i;

  for (k = 0; k < par->get_number_of_lines(); k++) {
    total_demerits = par->array_best_breaks_[k]->get_total_demerits();
    res += test_assert(tab_expected[k].total_demerits == total_demerits,
                       "total demerits: expected: %u, actual: %u",
                       tab_expected[k].total_demerits,
                       total_demerits);
    i = par->array_best_breaks_[k]->get_previous_box();
    if (i == NULL) {
      res += test_assert(false, "cannot find breakpoint's previous box");
    } else {
      paragraph_word *pw = i->get_word();
      pw->snprint(word, 256);
      i = par->array_best_breaks_[k]->get_item();
      res += test_assert(strcmp(tab_expected[k].word, word) == 0,
                         "expected: '%s' actual: '%s'",
                         tab_expected[k].word,
                         word);
    }
  }

  return res;
}

int
test_paragraph::check_best_breakpoint(const char*tab_expected[], paragraph *par)
{
  int res = 0;
  char word[256];
  int k = 0;
  item *i;

  for (k = 0; k < par->get_number_of_lines(); k++) {
    i = par->array_best_breaks_[k]->get_previous_box();
    if (i == NULL) {
      res += test_assert(false, "cannot find breakpoint's previous box");
    } else {
      paragraph_word *pw;
      pw = i->get_word();
      pw->snprint(word, 256);
      i = par->array_best_breaks_[k]->get_item();
      res += test_assert(strcmp(tab_expected[k], word) == 0,
                         "expected: '%s' actual: '%s'",
                         tab_expected[k],
                         word);
    }
  }

  return res; 
}

test_paragraph::~test_paragraph()
{
  if (text_loader_)
    delete text_loader_;
}

/* Load the text and add to par the corresponding box, glue, penalty */

/**
 * Test 1
 *
 * Check each line ratio.  FIXME understand why the last one should be 0.004 and
 * not 0.000.
 */
void
test_paragraph::suite1_init()
{  
  text_loader *tl;
  char text[] =
    "In olden times when wishing still helped one, there lived a "
    "king whose daughters were all beautiful; and the youngest was "
    "so beautiful that the sun itself, which has seen so much, was "
    "astonished whenever it shone in her face. Close by the king's "
    "castle lay a great dark forest, and under an old lime-tree in the "
    "forest was a well, and when the day was very warm, the king's "
    "child went out into the forest and sat down by the side of the "
    "cool fountain; and when she was bored she took a golden ball, "
    "and threw it up on high and caught it; and this ball was her "
    "favorite plaything.";

  printf("-- Suite 1 Initialisation\n");
  tl = new text_loader(text);
  text_loader_ = tl;
}

int
test_paragraph::test11_original_example()
{
  paragraph *par = new paragraph;
  paragraph_printer *printer;
  int res = 0;
  float expected_line_ratio[10] = {0.774, 0.179, 0.629, 0.545, 0.000,
                                   0.079, 0.282, 0.294, 0.575, 0.000};
  
  int n_lines;
  int k;
  float ratio;
  struct expected_break_info all_expected[] = {
    { "a", 2209, 2209 },
    { "king", 1521, 1521 },
    { "was", 4, 2213 },
    { "so", 3136, 4657 },
    { "was", 676, 2889 },
    { "aston", 3600, 8257 },
    { "king's", 289, 3178 },
    { "castle", 9, 8266 },
    { "lay", 4489, 12746 },
    { "in", 5929, 9107 },
    { "the", 1, 3179 },
    { "for", 3481, 11747 },
    { "est", 1, 8267 },
    { "was", 4, 12750 },
    { "a", 2209, 14955 },
    { "the", 676, 9783 },
    { "king's", 1, 3180 },
    { "child", 4, 8271 },
    { "went", 1, 12751 },
    { "out", 1369, 16324 },
    { "side", 16, 9799 },
    { "of", 49, 9832 },
    { "the", 9, 3189 },
    { "cool", 121, 8392 },
    { "foun", 3249, 16000 },
    { "tain;", 400, 13151 },
    { "and", 1444, 17768 },
    { "golden", 1, 9800 },
    { "ball,", 16, 3205 },
    { "and", 25, 8417 },
    { "threw", 4, 16004 },
    { "it", 289, 13440 },
    { "up", 4, 13155 },
    { "on", 1, 17769 },
    { "was", 25, 9825 },
    { "her", 400, 3605 },
    { "favor", 2601, 11018 },
    { "ite", 16, 8433 },
    { "play", 3364, 16804 },
    { "thing.", 1, 3606 }
  };

  struct expected_break_info best_expected[] = {
    { "a", 2209, 2209 },
    { "was", 4, 2213 },
    { "was", 676, 2889 },
    { "king's", 289, 3178 },
    { "the", 1, 3179 },
    { "king's", 1, 3180 },
    { "the", 9, 3189 },
    { "ball,", 16, 3205 },
    { "her", 400, 3605 },
    { "thing.", 1, 3606 }
  };

  printf("-- Test11...\n");

  par->config_use_old_demerits_formula();
  par->config_no_fitness_class();
  text_loader_->process_text(par, true);
  par->format_knuth_plass();

  /* There should be 10 lines */
  printf("   Checking the number of lines\n");
  n_lines = par->get_number_of_lines();
  if (test_assert(n_lines == 10,
                  "actual number of lines:%d expected: 10",
                  n_lines) == TEST_FAIL) {
    res++;
  }

  /* test ratio */
  printf("   Checking the lines adjustement ratio\n");
  for (k = 0; k < 10; k++) {
    ratio = par->get_adjust_ratio(k + 1);
    if (test_assert(fabs(ratio - expected_line_ratio[k]) < 0.001,
                    "line number %d expected: %.3f actual ratio: %.3f",
                    k + 1, expected_line_ratio[k], ratio) == TEST_FAIL) {
      res++;
    }
  }

  printf("   Checking all breakpoints demerits\n");
  res += check_all_breakpoint(all_expected, par);
  
  printf("   Checking the best breakpoints array\n");
  res += check_best_breakpoint(best_expected, par);
  printer = new paragraph_printer(par);
  printer->print();
  delete printer;
  PRINT_RESULT(res);
  delete par;
  
  return res;
}

int
test_paragraph::test12_example_with_default_demerits_formula()
{
  int res = 0;
  paragraph *par = new paragraph;
  fitness_class_t fitness_class;
  struct expected_break_info best_expected[] = {
    { "a", 2209, 2209 },
    { "was", 4, 2213 },
    { "was", 676, 2889 },
    { "king's", 289, 3178 },
    { "the", 1, 3179 },
    { "king's", 1, 3180 },
    { "the", 9, 3189 },
    { "ball,", 16, 3205 },
    { "her", 400, 3605 },
    { "thing.", 1, 3606 }
  };
  fitness_class_t expected_fitness_class[10] = {
    FITNESS_CLASS_LOOSE,
    FITNESS_CLASS_NORMAL,
    FITNESS_CLASS_LOOSE,
    FITNESS_CLASS_LOOSE,
    FITNESS_CLASS_NORMAL,
    FITNESS_CLASS_NORMAL,
    FITNESS_CLASS_NORMAL,
    FITNESS_CLASS_NORMAL,
    FITNESS_CLASS_LOOSE,
    FITNESS_CLASS_NORMAL
  };
    
  int k = 0;
  
  printf("-- Test12...\n");
  text_loader_->process_text(par, true);
  par->format_knuth_plass();
  
  printf("   Checking the best breakpoints array\n");
  res += check_best_breakpoint(best_expected, par);
  
  printf("   Checking the lines fitness class\n");
  for (k = 0; k < 10; k++) {
    fitness_class = par->get_fitness_class(k + 1);
    if (test_assert(
          (fitness_class == expected_fitness_class[k]),
          "line number: %d expected: %d actual fitness class: %d",
          k + 1,
          expected_fitness_class[k],
          fitness_class)  == TEST_FAIL)
      res++;
  }

  PRINT_RESULT(res);
  delete par;

  return res;
}

int
test_paragraph::test13_example_with_larger_tolerance()
{
  int res = 0;
  paragraph *par = new paragraph;
  struct expected_break_info best_expected[] = {
    { "a", 2209, 2209 },
    { "was", 4, 2213 },
    { "was", 676, 2889 },
    { "king's", 289, 3178 },
    { "the", 1, 3179 },
    { "king's", 1, 3180 },
    { "the", 9, 3189 },
    { "ball,", 16, 3205 },
    { "her", 400, 3605 },
    { "thing.", 1, 3606 }
  };
  
  printf("-- Test13...\n");
  text_loader_->process_text(par, true);
  par->format_knuth_plass(10);
  
  printf("   Checking the best breakpoints array\n");
  res += check_best_breakpoint(best_expected, par);
  
  PRINT_RESULT(res);
  delete par;

  return res;
}

/* In this test the word 'hyphenationtest' can be hyphenated after
 * 'hyphenation'. Letters 'A', 'B' and 'D' have a width of 10; 'C' has a width
 * of 13, which means that the algorithm would naturally try to cut like this:

    AAAAAAAAAA AAAAAAAAAA AAAAAAAAAA AAAAAAA hyphenation-
    test BBBBBBBBBB BBBBBBBBBB BBBBBBBBBB jlC hyphenation-
    test DDDDDDDDDD DDDDDDDDDD";

   However the additional penalty for two hyphenation in a row should make the
   algorithm chose the break before the first DDDDDDDDDD.
 */
void
test_paragraph::suite2_init()
{  
  text_loader *tl;
  char text[] =
    "AAAAAAAAAA AAAAAAAAAA AAAAAAAAAA AAAAAAA hyphenationtest "
    "BBBBBBBBBB BBBBBBBBBB BBBBBBBBBB jlC hyphenationtest "
    "DDDDDDDDDD DDDDDDDDDD";

  printf("-- Suite 2 Initialisation\n");
  tl = new text_loader(text);
  text_loader_ = tl;
}

int
test_paragraph::test21_hyphen_flagged_penalty()
{
  int res = 0;
  paragraph *par = new paragraph;
  paragraph_printer *printer;
  const char *best_expected[] = {
    "hyphenation",
    "test", //FIXME actually is should be hyphenationtest
    "DDDDDDDDDD"
  };
  
  printf("-- Test21...\n");
  text_loader_->process_text(par, false);
  par->format_knuth_plass(2);

  printf("   Checking the best breakpoints array\n");
  res += check_best_breakpoint(best_expected, par);

  printer = new paragraph_printer(par);
  printer->print();
  delete printer;
  PRINT_RESULT(res);
  delete par;

  return res;
}

/* There is only 1 feasible breakpoint at the first line, and the line is of
class 0.  At the second line, there are two feasible breaks, a class  and a
class 2; the class 2 is better but the class 1 should be chosen because of the
first line. */
void
test_paragraph::suite3_init()
{  
  text_loader *tl;
  char text[] =
    "The first line's best break makes it very veryyyyyy tiiiiiiiiiiiiiight, "
    "the second line's best break is of class two but another break will "
    "have to be preferred; it will give another line of class 0.";

  printf("-- Suite 3 Initialisation\n");
  tl = new text_loader(text);
  text_loader_ = tl;
}

int
test_paragraph::test31_fitness_class()
{
  int res = 0;
  paragraph *par = new paragraph;
  paragraph_printer *printer;
  int k = 0;
  item *i;

  const char *best_expected[] = {
    "tiiiiiiiiiiiiiight,",
    "will",
    "0."
  };
  char word[256];
  
  printf("-- Test31...\n");
  text_loader_->process_text(par, false);
  par->format_knuth_plass(2);

  printf("   Checking the best breakpoints array\n");
  for (k = 0; k < par->get_number_of_lines(); k++) {
    i = par->array_best_breaks_[k]->get_previous_box();
    if (i == NULL) {
      res += test_assert(false, "cannot find breakpoint's previous box");
    } else {
      i->sprint_word(word);
      i = par->array_best_breaks_[k]->get_item();
      res += test_assert(strcmp(best_expected[k], word) == 0,
                         "expected: '%s' actual: '%s'",
                         best_expected[k],
                         word);
    }
  }

  printer = new paragraph_printer(par);
  printer->print();
  delete printer;
  PRINT_RESULT(res);
  delete par;

  return res;
}


int main (int argc, char **argv)
{
  int res = 0;
  int c;
  test_paragraph *tp;
  char *file_path = NULL;
  unsigned int line_length = 500;
  float tolerance = 1;
  long int suite_to_launch = -1;
  long int test_to_launch = -1;

  while ((c = getopt (argc, argv, "f:l:s:t:T:")) != -1) {
      switch (c)
      {
      case 'f':
        file_path = optarg;
        break;
      case 'l':
        line_length = strtol(optarg, NULL, 10);
        break;
      case 's':
        suite_to_launch = strtol(optarg, NULL, 10);
        break;
      case 't':
        test_to_launch = strtol(optarg, NULL, 10);
        break;
      case 'T':
        tolerance = strtof(optarg, NULL);
        break;
      case '?':
        if (optopt == 'f' || optopt == 's' || optopt == 't')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint(optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort();
      }
  }

  if (suite_to_launch == 0 && test_to_launch > 0) {
    fprintf (stderr,
             "Passing test number without test suite, please use option -s");
    res = -1;
    goto end;
  }

  if (file_path != NULL) {
    text_loader *tl = new text_loader(NULL, file_path);
    paragraph *par = new paragraph();
    paragraph_printer *printer;
    printf("Processing content of %s with tolerance:%.3f line length:%u\n\n",
           file_path, tolerance, line_length);
    tl->process_text(par, true);
    par->format_knuth_plass(tolerance, line_length);
    printer = new paragraph_printer(par);
    printer->print();
    delete printer;
    delete par;
    delete tl;
  } else {
    if (suite_to_launch == -1 || suite_to_launch == 1) {
      tp = new test_paragraph();
      tp->suite1_init();
      if (test_to_launch == -1 || test_to_launch == 1)
        res += tp->test11_original_example();
      if (test_to_launch == -1 || test_to_launch == 2)
        res += tp->test12_example_with_default_demerits_formula();
      if (test_to_launch == -1 || test_to_launch == 3)
        res += tp->test13_example_with_larger_tolerance();
      delete tp;
    }

    if (suite_to_launch == -1 || suite_to_launch == 2) {
      tp = new test_paragraph();
      tp->suite2_init();
      if (test_to_launch == -1 || test_to_launch == 1)
        res += tp->test21_hyphen_flagged_penalty();
      delete tp;
    }

    if (suite_to_launch == -1 || suite_to_launch == 3) {
      tp = new test_paragraph();
      tp->suite3_init();
      if (test_to_launch == -1 || test_to_launch == 1)
        res += tp->test31_fitness_class();
      delete tp;
    }

    if (res == 0)
      printf("All tests passed\n");
    else
      fprintf(stderr, "%d tests failed\n", res);
  }

end:
  return res;
}
