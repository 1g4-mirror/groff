/* Copyright (C) 1989-2024 Free Software Foundation, Inc.
     Written by James Clark (jjc@jclark.com)

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
along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <vector>
#include <utility>

extern bool using_character_classes;	// if `.class` is invoked
extern void get_flags();

class macro;

class charinfo : glyph {
  static int next_index;
  charinfo *translation;
  macro *mac;
  unsigned char special_translation;
  unsigned char hyphenation_code;
  unsigned int flags;
  unsigned char ascii_code;
  unsigned char asciify_code;
  bool is_not_found;
  bool is_transparently_translatable;
  bool translatable_as_input; // asciify_code is active for .asciify
  char_mode mode;
  // Unicode character classes
  std::vector<std::pair<int, int> > ranges;
  std::vector<charinfo *> nested_classes;
public:
  enum {		// Values for the flags bitmask.  See groff
			// manual, description of the '.cflags' request.
    ENDS_SENTENCE = 0x01,
    BREAK_BEFORE = 0x02,
    BREAK_AFTER = 0x04,
    OVERLAPS_HORIZONTALLY = 0x08,
    OVERLAPS_VERTICALLY = 0x10,
    TRANSPARENT = 0x20,
    IGNORE_HCODES = 0x40,
    DONT_BREAK_BEFORE = 0x80,
    DONT_BREAK_AFTER = 0x100,
    INTER_CHAR_SPACE = 0x200
  };
  enum {
    TRANSLATE_NONE,
    TRANSLATE_SPACE,
    TRANSLATE_DUMMY,
    TRANSLATE_STRETCHABLE_SPACE,
    TRANSLATE_HYPHEN_INDICATOR
  };
  symbol nm;
  charinfo(symbol);
  glyph *as_glyph();
  int ends_sentence();
  int overlaps_vertically();
  int overlaps_horizontally();
  int can_break_before();
  int can_break_after();
  int transparent();
  int ignore_hcodes();
  int prohibit_break_before();
  int prohibit_break_after();
  int inter_char_space();
  unsigned char get_hyphenation_code();
  unsigned char get_ascii_code();
  unsigned char get_asciify_code();
  int get_unicode_code();
  void set_hyphenation_code(unsigned char);
  void set_ascii_code(unsigned char);
  void set_asciify_code(unsigned char);
  void make_translatable_as_input();
  bool is_translatable_as_input();
  charinfo *get_translation(bool = false);
  void set_translation(charinfo *, int, int);
  void get_flags();
  void set_flags(unsigned int);
  void set_special_translation(int, int);
  int get_special_translation(bool = false);
  macro *set_macro(macro *);
  macro *setx_macro(macro *, char_mode);
  macro *get_macro();
  int first_time_not_found();
  void set_number(int);
  int get_number();
  int numbered();
  int is_normal();
  int is_fallback();
  int is_special();
  symbol *get_symbol();
  void add_to_class(int);
  void add_to_class(int, int);
  void add_to_class(charinfo *);
  bool is_class();
  bool contains(int, bool = false);
  bool contains(symbol, bool = false);
  bool contains(charinfo *, bool = false);
};

charinfo *get_charinfo(symbol);
extern charinfo *charset_table[];
charinfo *get_charinfo_by_number(int);

inline int charinfo::overlaps_horizontally()
{
  if (using_character_classes)
    ::get_flags();
  return flags & OVERLAPS_HORIZONTALLY;
}

inline int charinfo::overlaps_vertically()
{
  if (using_character_classes)
    ::get_flags();
  return flags & OVERLAPS_VERTICALLY;
}

inline int charinfo::can_break_before()
{
  if (using_character_classes)
    ::get_flags();
  return flags & BREAK_BEFORE;
}

inline int charinfo::can_break_after()
{
  if (using_character_classes)
    ::get_flags();
  return flags & BREAK_AFTER;
}

inline int charinfo::ends_sentence()
{
  if (using_character_classes)
    ::get_flags();
  return flags & ENDS_SENTENCE;
}

inline int charinfo::transparent()
{
  if (using_character_classes)
    ::get_flags();
  return flags & TRANSPARENT;
}

inline int charinfo::ignore_hcodes()
{
  if (using_character_classes)
    ::get_flags();
  return flags & IGNORE_HCODES;
}

inline int charinfo::prohibit_break_before()
{
  if (using_character_classes)
    ::get_flags();
  return flags & DONT_BREAK_BEFORE;
}

inline int charinfo::prohibit_break_after()
{
  if (using_character_classes)
    ::get_flags();
  return flags & DONT_BREAK_AFTER;
}

inline int charinfo::inter_char_space()
{
  if (using_character_classes)
    ::get_flags();
  return flags & INTER_CHAR_SPACE;
}

inline int charinfo::numbered()
{
  return number >= 0;
}

inline int charinfo::is_normal()
{
  return mode == CHAR_NORMAL;
}

inline int charinfo::is_fallback()
{
  return mode == CHAR_FALLBACK;
}

inline int charinfo::is_special()
{
  return mode == CHAR_SPECIAL;
}

inline charinfo *charinfo::get_translation(bool for_transparent_throughput)
{
  return ((for_transparent_throughput && !is_transparently_translatable)
	  ? 0
	  : translation);
}

inline unsigned char charinfo::get_hyphenation_code()
{
  return hyphenation_code;
}

inline unsigned char charinfo::get_ascii_code()
{
  return ascii_code;
}

inline unsigned char charinfo::get_asciify_code()
{
  return (translatable_as_input ? asciify_code : 0);
}

inline void charinfo::set_flags(unsigned int c)
{
  flags = c;
}

inline glyph *charinfo::as_glyph()
{
  return this;
}

inline void charinfo::make_translatable_as_input()
{
  translatable_as_input = true;
}

inline bool charinfo::is_translatable_as_input()
{
  return translatable_as_input;
}

inline int charinfo::get_special_translation(bool for_transparent_throughput)
{
  return (for_transparent_throughput && !is_transparently_translatable
	  ? int(TRANSLATE_NONE)
	  : special_translation);
}

inline macro *charinfo::get_macro()
{
  return mac;
}

inline int charinfo::first_time_not_found()
{
  if (is_not_found)
    return false;
  else {
    is_not_found = true;
    return true;
  }
}

inline symbol *charinfo::get_symbol()
{
  return &nm;
}

inline void charinfo::add_to_class(int c)
{
  using_character_classes = true;
  // TODO ranges cumbersome for single characters?
  ranges.push_back(std::pair<int, int>(c, c));
}

inline void charinfo::add_to_class(int lo,
				   int hi)
{
  using_character_classes = true;
  ranges.push_back(std::pair<int, int>(lo, hi));
}

inline void charinfo::add_to_class(charinfo *ci)
{
  using_character_classes = true;
  nested_classes.push_back(ci);
}

inline bool charinfo::is_class()
{
  return (!ranges.empty() || !nested_classes.empty());
}

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:
