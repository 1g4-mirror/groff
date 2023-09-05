/* Copyright (C) 1989-2021 Free Software Foundation, Inc.
     Written by James Clark (jjc@jclark.com)
     OSC 8 support by G. Branden Robinson

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

#include "driver.h"
#include "device.h"
#include "ptable.h"

#include <stringclass.h>

#include <curses.h>
#include <term.h>

typedef signed char schar;

declare_ptable(schar)
implement_ptable(schar)

extern "C" const char *Version_string;

#ifndef SHRT_MIN
#define SHRT_MIN (-32768)
#endif

#ifndef SHRT_MAX
#define SHRT_MAX 32767
#endif

static int tab_width = 8;

// A character of the output device fits in a 32-bit word.
typedef unsigned int output_character;

static bool want_horizontal_tabs = false;
static bool want_form_feeds = false;
static bool want_emboldening_by_overstriking = true;
static bool do_bold;
static bool want_italics_by_underlining = true;
static bool do_underline;
static bool accept_glyph_composition_by_overstriking = true;
static bool do_glyph_composition_by_overstriking;
static bool allow_drawing_commands = true;
static bool want_real_italics = false;
static bool want_reverse_video_for_italics = false;
static bool use_overstriking_drawing_scheme = false;

static void update_options();
static void usage(FILE *stream);

// TODO(humm): We can get proper line drawing characters from terminfo
// using acsc, smacs, rmacs; we could do that for -Tascii + terminfo.
// There are other characters there we could theoreticlly use as well,
// like degree and pi.
static int hline_char = '-';
static int vline_char = '|';

typedef unsigned char glyph_mode;
enum {
  UNDERLINE_MODE = 0x01,
  BOLD_MODE = 0x02,
  VDRAW_MODE = 0x04,
  HDRAW_MODE = 0x08,
  CU_MODE = 0x10,
  URI_MODE = 0x20,
  START_LINE = 0x40,
  END_LINE = 0x80
};

// Mode to use for bold-underlining.
static glyph_mode bold_underline_mode_option = BOLD_MODE|UNDERLINE_MODE;
static glyph_mode bold_underline_mode;

#ifndef IS_EBCDIC_HOST
#define OSC8 "\033]8"
#define ST "\033\\"
#else
#define OSC8 "\047]8"
#define ST "\047\\"
#endif

static char *enter_italics_or_the_like_mode;

#define DEFAULT_COLOR_IDX -1

class tty_font : public font {
  tty_font(const char *);
  glyph_mode mode;
public:
  ~tty_font();
  glyph_mode get_mode() { return mode; }
#if 0
  void handle_x_command(int argc, const char **argv);
#endif
  static tty_font *load_tty_font(const char *);
};

tty_font *tty_font::load_tty_font(const char *s)
{
  tty_font *f = new tty_font(s);
  if (!f->load()) {
    delete f;
    return 0;
  }
  const char *num = f->get_internal_name();
  long n;
  if (num != 0 && (n = strtol(num, 0, 0)) != 0)
    f->mode = (glyph_mode)(n & (BOLD_MODE|UNDERLINE_MODE));
  if (!do_underline)
    f->mode &= ~UNDERLINE_MODE;
  if (!do_bold)
    f->mode &= ~BOLD_MODE;
  if ((f->mode & (BOLD_MODE|UNDERLINE_MODE)) == (BOLD_MODE|UNDERLINE_MODE))
    f->mode = ((f->mode & ~(BOLD_MODE|UNDERLINE_MODE))
	       | bold_underline_mode);
  return f;
}

tty_font::tty_font(const char *nm)
: font(nm), mode(0)
{
}

tty_font::~tty_font()
{
}

#if 0
void tty_font::handle_x_command(int argc, const char **argv)
{
  if (argc >= 1 && strcmp(argv[0], "bold") == 0)
    mode |= BOLD_MODE;
  else if (argc >= 1 && strcmp(argv[0], "underline") == 0)
    mode |= UNDERLINE_MODE;
}
#endif

class tty_glyph {
public:
  tty_glyph *next;
  int w;
  int hpos;
  unsigned int code;
  glyph_mode mode;
  schar back_color_idx;
  schar fore_color_idx;
  string osc8_string;
  inline glyph_mode draw_mode() { return mode & (VDRAW_MODE|HDRAW_MODE); }
  inline int order() {
    return mode & (VDRAW_MODE|HDRAW_MODE|CU_MODE|URI_MODE); }
};


class tty_printer : public printer {
  tty_glyph **lines_; // clashes with Curses lines
  int nlines;
  int cached_v;
  int cached_vpos;
  schar curr_fore_idx;
  schar curr_back_idx;
  bool is_underlining;
  bool is_boldfacing;
  bool is_continuously_underlining;
  PTABLE(schar) tty_colors;
  void overstrike(bool, bool, output_character, int);
  void update_attributes(bool, bool, schar, schar);
  schar color_to_idx(color *);
  void add_char(output_character, int, int, int, color *, color *,
		const string &, glyph_mode);
  char *make_rgb_string(unsigned int, unsigned int, unsigned int);
  bool tty_color(unsigned int, unsigned int, unsigned int, schar *,
		 schar = DEFAULT_COLOR_IDX);
  void line(int, int, int, int, color *, color *);
  void draw_line(int *, int, const environment *);
  void draw_polygon(int *, int, const environment *);
  void special_link(const char *, const environment *);
public:
  tty_printer();
  ~tty_printer();
  void set_char(glyph *, font *, const environment *, int, const char *);
  void draw(int, int *, int, const environment *);
  void special(char *, const environment *, char);
  void put_char(output_character);
  void begin_page(int) { }
  void end_page(int);
  font *make_font(const char *);
};

char *tty_printer::make_rgb_string(unsigned int r,
				   unsigned int g,
				   unsigned int b)
{
  char *s = new char[8];
  s[0] = char(r >> 8);
  s[1] = char(r & 0xff);
  s[2] = char(g >> 8);
  s[3] = char(g & 0xff);
  s[4] = char(b >> 8);
  s[5] = char(b & 0xff);
  s[6] = char(0x80);
  s[7] = 0;
  // avoid null-bytes in string
  for (int i = 0; i < 6; i++)
    if (!s[i]) {
      s[i] = 1;
      s[6] |= 1 << i;
    }
  return s;
}

bool tty_printer::tty_color(unsigned int r,
			    unsigned int g,
			    unsigned int b, schar *idx, schar value)
{
  bool is_known_color = true;
  char *s = make_rgb_string(r, g, b);
  schar *i = tty_colors.lookup(s);
  if (!i) {
    is_known_color = false;
    i = new schar[1];
    *i = value;
    tty_colors.define(s, i);
  }
  *idx = *i;
  delete[] s;
  return is_known_color;
}

tty_printer::tty_printer() : cached_v(0)
{
  if (font::is_unicode) {
    hline_char = 0x2500;
    vline_char = 0x2502;
  }
  schar dummy;
  // black, white
  (void)tty_color(0, 0, 0, &dummy, 0);
  (void)tty_color(color::MAX_COLOR_VAL,
		  color::MAX_COLOR_VAL,
		  color::MAX_COLOR_VAL, &dummy, 7);
  // red, green, blue
  (void)tty_color(color::MAX_COLOR_VAL, 0, 0, &dummy, 1);
  (void)tty_color(0, color::MAX_COLOR_VAL, 0, &dummy, 2);
  (void)tty_color(0, 0, color::MAX_COLOR_VAL, &dummy, 4);
  // yellow, magenta, cyan
  (void)tty_color(color::MAX_COLOR_VAL, color::MAX_COLOR_VAL, 0, &dummy, 3);
  (void)tty_color(color::MAX_COLOR_VAL, 0, color::MAX_COLOR_VAL, &dummy, 5);
  (void)tty_color(0, color::MAX_COLOR_VAL, color::MAX_COLOR_VAL, &dummy, 6);
  nlines = 66;
  lines_ = new tty_glyph *[nlines];
  for (int i = 0; i < nlines; i++)
    lines_[i] = 0;
  is_continuously_underlining = false;
}

tty_printer::~tty_printer()
{
  delete[] lines_;
}

// The function overstrike applies text attributes for the next glyph
// rendered using the overstriking scheme.  The function
// update_attributes uses terminfo capabilities to apply text attributes
// for all following glyphs, until it's called again with different
// arguments.  If update_attributes is called with the same arguments as
// the last time, it doesn't print anything.
//
// Thus, call overstrike for every single glyph you print and call
// update_attributes either whenever the attributes change or for every
// glyph, whatever is more convenient.  Do call both.

void tty_printer::overstrike(bool underline, bool bold,
			     output_character c, int w)
{
  if (!use_overstriking_drawing_scheme)
    return;

  if (underline) {
    if (!w)
      warning("can't underline zero-width character");
    else {
      putchar('_');
      putchar('\b');
    }
  }
  if (bold) {
    if (!w)
      warning("can't print zero-width character in bold");
    else {
      put_char(c);
      putchar('\b');
    }
  }
}

void tty_printer::update_attributes(bool underline, bool bold,
				    schar fore_idx, schar back_idx)
{
  if (use_overstriking_drawing_scheme)
    return;

  if (is_underlining && !underline
      || is_boldfacing && !bold
      || curr_fore_idx != DEFAULT_COLOR_IDX && fore_idx == DEFAULT_COLOR_IDX
      || curr_back_idx != DEFAULT_COLOR_IDX && back_idx == DEFAULT_COLOR_IDX) {
    putp(exit_attribute_mode);
    is_underlining = is_boldfacing = false;
    curr_fore_idx = curr_back_idx = DEFAULT_COLOR_IDX;
  }

  if (!is_underlining && underline) {
    putp(enter_italics_or_the_like_mode);
    is_underlining = true;
  }

  if (!is_boldfacing && bold) {
    putp(enter_bold_mode);
    is_boldfacing = true;
  }

  if (curr_fore_idx != fore_idx) {
    putp(tparm(set_a_foreground, fore_idx, 0, 0, 0, 0, 0, 0, 0, 0));
    curr_fore_idx = fore_idx;
  }

  if (curr_back_idx != back_idx) {
    putp(tparm(set_a_background, back_idx, 0, 0, 0, 0, 0, 0, 0, 0));
    curr_back_idx = back_idx;
  }
}

schar tty_printer::color_to_idx(color *col)
{
  if (col->is_default())
    return DEFAULT_COLOR_IDX;
  unsigned int r, g, b;
  col->get_rgb(&r, &g, &b);
  schar idx;
  if (!tty_color(r, g, b, &idx)) {
    char *s = col->print_color();
    error("unrecognized color '%1' mapped to default", s);
    delete[] s;
  }
  return idx;
}

void tty_printer::set_char(glyph *g, font *f, const environment *env,
			   int w, const char *)
{
  if (w % font::hor != 0)
    fatal("glyph width is not a multiple of horizontal motion quantum");
  add_char(f->get_code(g), w,
	   env->hpos, env->vpos,
	   env->col, env->fill,
	   string(),
	   ((tty_font *)f)->get_mode());
}

void tty_printer::add_char(output_character c, int w,
			   int h, int v,
			   color *fore, color *back,
			   const string &osc8_string,
			   glyph_mode mode)
{
#if 0
  // This is too expensive.
  if (h % font::hor != 0)
    fatal("horizontal position not a multiple of horizontal motion quantum");
#endif
  int hpos = h / font::hor;
  if (hpos < SHRT_MIN || hpos > SHRT_MAX) {
    error("character with ridiculous horizontal position discarded");
    return;
  }
  int vpos;
  if (v == cached_v && cached_v != 0)
    vpos = cached_vpos;
  else {
    if (v % font::vert != 0)
      fatal("vertical position not a multiple of vertical motion"
	    " quantum");
    vpos = v / font::vert;
    if (vpos > nlines) {
      tty_glyph **old_lines = lines_;
      lines_ = new tty_glyph *[vpos + 1];
      memcpy(lines_, old_lines, nlines * sizeof(tty_glyph *));
      for (int i = nlines; i <= vpos; i++)
	lines_[i] = 0;
      delete[] old_lines;
      nlines = vpos + 1;
    }
    // Note that the first output line corresponds to groff
    // position font::vert.
    if (vpos <= 0) {
      error("output above first line discarded");
      return;
    }
    cached_v = v;
    cached_vpos = vpos;
  }
  tty_glyph *g = new tty_glyph;
  g->w = w;
  g->hpos = hpos;
  g->code = c;
  g->fore_color_idx = color_to_idx(fore);
  g->back_color_idx = color_to_idx(back);
  g->osc8_string = osc8_string;
  g->mode = mode;

  // The list will be reversed later.  After reversal, it must be in
  // increasing order of hpos, with CU and URI specials before HDRAW
  // characters before VDRAW characters before normal characters at
  // each hpos, and otherwise in order of occurrence.

  tty_glyph **pp;
  for (pp = lines_ + (vpos - 1); *pp; pp = &(*pp)->next)
    if ((*pp)->hpos < hpos
	|| ((*pp)->hpos == hpos && (*pp)->order() >= g->order()))
      break;
  g->next = *pp;
  *pp = g;
}

void tty_printer::special(char *arg, const environment *env, char type)
{
  if (type == 'u') {
    add_char(*arg - '0', 0, env->hpos, env->vpos, env->col, env->fill,
	     string(), CU_MODE);
    return;
  }
  if (type != 'p')
    return;
  char *p;
  for (p = arg; *p == ' ' || *p == '\n'; p++)
    ;
  char *tag = p;
  for (; *p != '\0' && *p != ':' && *p != ' ' && *p != '\n'; p++)
    ;
  if (*p == '\0' || strncmp(tag, "tty", p - tag) != 0) {
    error("X command without 'tty:' tag ignored");
    return;
  }
  p++;
  for (; *p == ' ' || *p == '\n'; p++)
    ;
  char *command = p;
  for (; *p != '\0' && *p != ' ' && *p != '\n'; p++)
    ;
  if (*command == '\0') {
    error("empty X command ignored");
    return;
  }
  if (strncmp(command, "link", p - command) == 0)
    special_link(p, env);
  else
    warning("unrecognized X command '%1' ignored", command);
}

// Produce an OSC 8 hyperlink.  Given ditroff output of the form:
//   x X tty: link [URI[ KEY=VALUE] ...]
// produce "OSC 8 ;[KEY=VALUE];[URI] ST".  KEY/VALUE pairs can be
// repeated arbitrarily and are separated by colons.  Omission of the
// URI ends the hyperlink that was begun by specifying it.  See
// <https://gist.github.com/egmontkob/eb114294efbcd5adb1944c9f3cb5feda>.
//
// TODO: Discard URIs with prohibited bytes.  Only 08-0D and 20-7E are
// allowed between OSC and ST.
void tty_printer::special_link(const char *arg, const environment *env)
{
  static bool is_link_active = false;
  if (use_overstriking_drawing_scheme)
    return;
  char c = *arg;
  if ('\0' == c || '\n' == c) {
    if (!is_link_active)
      warning("ending hyperlink when none was started");
    else
      add_char(0, 0, env->hpos, env->vpos, env->col, env->fill,
	       string(), URI_MODE);
    is_link_active = false;
  }
  else {
    // Our caller ensures that we see whitespace after 'link'.
    assert(c == ' ' || c == '\t');
    string s = ";";
    if (is_link_active)
      warning("new hyperlink started without ending previous one");
    is_link_active = true;
    do
      c = *arg++;
    while (c == ' ' || c == '\t');
    arg--;
    // The first argument is the URI.
    const char *uri = arg;
    do
      c = *arg++;
    while (c != '\0' && c != ' ' && c != '\t');
    arg--;
    ptrdiff_t uri_len = arg - uri;
    // Any remaining arguments are "key=value" pairs.
    const char *pair = 0;
    bool done = false;
    do {
      if (pair != 0)
	s += ':';
      pair = arg;
      bool in_pair = true;
      do {
	c = *arg++;
	if ('\0' == c)
	  done = true;
	else if (' ' == c || '\t' == c)
	  in_pair = false;
	else
	  s += c;
      } while (!done && in_pair);
    } while (!done);
    s += ';';
    s.append(uri, uri_len);

    add_char(0, 0, env->hpos, env->vpos, env->col, env->fill, s, URI_MODE);
  }
}

void tty_printer::draw(int code, int *p, int np, const environment *env)
{
  if (!allow_drawing_commands)
    return;
  if (code == 'l')
    draw_line(p, np, env);
  else if (code == 'p')
    draw_polygon(p, np, env);
  else
    warning("ignoring unsupported drawing command '%1'", char(code));
}

void tty_printer::draw_polygon(int *p, int np, const environment *env)
{
  if (np & 1) {
    error("even number of arguments required for polygon");
    return;
  }
  if (np == 0) {
    error("no arguments for polygon");
    return;
  }
  // We only draw polygons which consist entirely of horizontal and
  // vertical lines.
  int hpos = 0;
  int vpos = 0;
  for (int i = 0; i < np; i += 2) {
    if (!(p[i] == 0 || p[i + 1] == 0))
      return;
    hpos += p[i];
    vpos += p[i + 1];
  }
  if (!(hpos == 0 || vpos == 0))
    return;
  int start_hpos = env->hpos;
  int start_vpos = env->vpos;
  hpos = start_hpos;
  vpos = start_vpos;
  for (int i = 0; i < np; i += 2) {
    line(hpos, vpos, p[i], p[i + 1], env->col, env->fill);
    hpos += p[i];
    vpos += p[i + 1];
  }
  line(hpos, vpos, start_hpos - hpos, start_vpos - vpos,
       env->col, env->fill);
}

void tty_printer::draw_line(int *p, int np, const environment *env)
{
  if (np != 2) {
    error("2 arguments required for line");
    return;
  }
  line(env->hpos, env->vpos, p[0], p[1], env->col, env->fill);
}

void tty_printer::line(int hpos, int vpos, int dx, int dy,
		       color *col, color *fill)
{
  // XXX: zero-length lines get drawn as '+' crossings in nroff, even
  // when there is no crossing, but they nevertheless occur frequently
  // in input.  Does tbl produce them?
#if 0
  if (0 == dx)
    fatal("cannot draw zero-length horizontal line");
  if (0 == dy)
    fatal("cannot draw zero-length vertical line");
#endif
  if ((dx != 0) && (dy != 0))
    warning("cannot draw diagonal line");
  if (dx % font::hor != 0)
    fatal("length of horizontal line %1 is not a multiple of horizontal"
	" motion quantum %2", dx, font::hor);
  if (dy % font::vert != 0)
    fatal("length of vertical line %1 is not a multiple of vertical"
	" motion quantum %2", dy, font::vert);
  if (dx == 0) {
    // vertical line
    int v = vpos;
    int len = dy;
    if (len < 0) {
      v += len;
      len = -len;
    }
    if (len == 0)
      add_char(vline_char, font::hor, hpos, v, col, fill, string(),
	       VDRAW_MODE|START_LINE|END_LINE);
    else {
      add_char(vline_char, font::hor, hpos, v, col, fill, string(),
	       VDRAW_MODE|START_LINE);
      len -= font::vert;
      v += font::vert;
      while (len > 0) {
	add_char(vline_char, font::hor, hpos, v, col, fill, string(),
		 VDRAW_MODE|START_LINE|END_LINE);
	len -= font::vert;
	v += font::vert;
      }
      add_char(vline_char, font::hor, hpos, v, col, fill, string(),
	       VDRAW_MODE|END_LINE);
    }
  }
  if (dy == 0) {
    // horizontal line
    int h = hpos;
    int len = dx;
    if (len < 0) {
      h += len;
      len = -len;
    }
    if (len == 0)
      add_char(hline_char, font::hor, h, vpos, col, fill, string(),
	       HDRAW_MODE|START_LINE|END_LINE);
    else {
      add_char(hline_char, font::hor, h, vpos, col, fill, string(),
	       HDRAW_MODE|START_LINE);
      len -= font::hor;
      h += font::hor;
      while (len > 0) {
	add_char(hline_char, font::hor, h, vpos, col, fill, string(),
		 HDRAW_MODE|START_LINE|END_LINE);
	len -= font::hor;
	h += font::hor;
      }
      add_char(hline_char, font::hor, h, vpos, col, fill, string(),
	       HDRAW_MODE|END_LINE);
    }
  }
}

void tty_printer::put_char(output_character wc)
{
  if (font::is_unicode && wc >= 0x80) {
    char buf[6 + 1];
    int count;
    char *p = buf;
    if (wc < 0x800)
      count = 1, *p = (unsigned char)((wc >> 6) | 0xc0);
    else if (wc < 0x10000)
      count = 2, *p = (unsigned char)((wc >> 12) | 0xe0);
    else if (wc < 0x200000)
      count = 3, *p = (unsigned char)((wc >> 18) | 0xf0);
    else if (wc < 0x4000000)
      count = 4, *p = (unsigned char)((wc >> 24) | 0xf8);
    else if (wc <= 0x7fffffff)
      count = 5, *p = (unsigned char)((wc >> 30) | 0xfC);
    else
      return;
    do *++p = (unsigned char)(((wc >> (6 * --count)) & 0x3f) | 0x80);
      while (count > 0);
    *++p = '\0';
    fputs(buf, stdout);
  }
  else
    putchar(wc);
}

// The possible Unicode combinations for crossing characters.
//
// '  ' = 0, ' -' = 4, '- ' = 8, '--' = 12,
//
// '  ' = 0, ' ' = 1, '|' = 2, '|' = 3
//            |                 |

static output_character crossings[4*4] = {
  0x0000, 0x2577, 0x2575, 0x2502,
  0x2576, 0x250C, 0x2514, 0x251C,
  0x2574, 0x2510, 0x2518, 0x2524,
  0x2500, 0x252C, 0x2534, 0x253C
};

void tty_printer::end_page(int page_length)
{
  if (page_length % font::vert != 0)
    error("vertical position at end of page not multiple of vertical"
	  " motion quantum");
  int lines_per_page = page_length / font::vert;
  int last_line;
  for (last_line = nlines; last_line > 0; last_line--)
    if (lines_[last_line - 1])
      break;
#if 0
  if (last_line > lines_per_page) {
    error("characters past last line discarded");
    do {
      --last_line;
      while (lines[last_line]) {
	tty_glyph *tem = lines[last_line];
	lines[last_line] = tem->next;
	delete tem;
      }
    } while (last_line > lines_per_page);
  }
#endif
  for (int i = 0; i < last_line; i++) {
    tty_glyph *p = lines_[i];
    lines_[i] = 0;
    tty_glyph *g = 0;
    while (p) {
      tty_glyph *tem = p->next;
      p->next = g;
      g = p;
      p = tem;
    }
    int hpos = 0;
    tty_glyph *nextp;
    curr_fore_idx = DEFAULT_COLOR_IDX;
    curr_back_idx = DEFAULT_COLOR_IDX;
    is_underlining = false;
    is_boldfacing = false;
    for (p = g; p; delete p, p = nextp) {
      nextp = p->next;
      if (p->mode & CU_MODE) {
	is_continuously_underlining = (p->code != 0);
	continue;
      }
      if (nextp && p->hpos == nextp->hpos
	  && !(p->mode & URI_MODE) && !(nextp->mode & URI_MODE)) {
	if (p->draw_mode() == HDRAW_MODE &&
	    nextp->draw_mode() == VDRAW_MODE) {
	  if (font::is_unicode)
	    nextp->code =
	      crossings[((p->mode & (START_LINE|END_LINE)) >> 4)
			+ ((nextp->mode & (START_LINE|END_LINE)) >> 6)];
	  else
	    nextp->code = '+';
	  continue;
	}
	if (p->draw_mode() != 0 && p->draw_mode() == nextp->draw_mode()) {
	  nextp->code = p->code;
	  continue;
	}
	if (!do_glyph_composition_by_overstriking)
	  continue;
      }
      // TODO(humm): Is this branch ever taken?  See the loop at the end
      // of add_char.
      if (hpos > p->hpos) {
	do {
	  putchar('\b');
	  hpos--;
	} while (hpos > p->hpos);
      }
      else if (p->hpos > hpos) {
	update_attributes(is_continuously_underlining, is_boldfacing,
			  curr_fore_idx, curr_back_idx);
	if (want_horizontal_tabs) {
	  for (;;) {
	    int next_tab_pos = ((hpos + tab_width) / tab_width) * tab_width;
	    if (next_tab_pos > p->hpos)
	      break;
	    // TODO(humm): Should we not take the width of the actual
	    // tab?  The current status works, albeit not with
	    // typewriters: _\b\t is rendered as multiple underlined
	    // cells by less.  We could pass overstrike the width
	    // the actual tab will have and let it emit multiple
	    // underlines.
	    overstrike(is_continuously_underlining, false, '\t', p->w);
	    putchar('\t');
	    hpos = next_tab_pos;
	  }
	}
	for (; hpos < p->hpos; hpos++) {
	  overstrike(is_continuously_underlining, false, ' ', p->w);
	  putchar(' ');
	}
      }
      assert(hpos == p->hpos);
      if (p->mode & URI_MODE) {
	if (nextp && (nextp->mode & URI_MODE))
	  continue;
	fputs(OSC8, stdout);
	if (p->osc8_string.empty())
	  fputs(";;", stdout);
	else
	  fwrite(p->osc8_string.contents(), 1, p->osc8_string.length(), stdout);
	fputs(ST, stdout);
	continue;
      }
      overstrike(p->mode & UNDERLINE_MODE, p->mode & BOLD_MODE, p->code, p->w);
      update_attributes(p->mode & UNDERLINE_MODE, p->mode & BOLD_MODE,
			p->fore_color_idx, p->back_color_idx);
      put_char(p->code);
      hpos += p->w / font::hor;
    }
    update_attributes(false, false, DEFAULT_COLOR_IDX, DEFAULT_COLOR_IDX);
    putchar('\n');
  }
  if (want_form_feeds) {
    if (last_line < lines_per_page)
      putchar('\f');
  }
  else {
    for (; last_line < lines_per_page; last_line++)
      putchar('\n');
  }
}

font *tty_printer::make_font(const char *nm)
{
  return tty_font::load_tty_font(nm);
}

printer *make_printer()
{
  return new tty_printer();
}

static void update_options()
{
  if (use_overstriking_drawing_scheme) {
    bold_underline_mode = bold_underline_mode_option;
    do_bold = want_emboldening_by_overstriking;
    do_underline = want_italics_by_underlining;
    do_glyph_composition_by_overstriking =
      accept_glyph_composition_by_overstriking;
    return;
  }

  bold_underline_mode = BOLD_MODE|UNDERLINE_MODE;
  do_bold = true;
  do_underline = true;

  int err;
  if (setupterm(NULL, 1, &err) == ERR)
    switch (err) {
      // It's a little sad that we can't use Curses's own error
      // strings, just to be able to handle hardcopy terminals,
      // only because ncurses behaves in a non-standard manner when
      // stumbling upon the hc capability.
    case -1:
      fatal("terminfo database not found");
    case 0:
      fatal("terminal description not found");
    case 1: // hardcopy terminal (non standard) / success (standard)
      // We check for over_strike anyway.  Ncurses is nice enough
      // to load the capabilities anyway.
      ;
    }

  if (want_real_italics)
    enter_italics_or_the_like_mode = enter_italics_mode;
  else if (want_reverse_video_for_italics)
    enter_italics_or_the_like_mode = enter_reverse_mode;
  else
    enter_italics_or_the_like_mode = enter_underline_mode;

  if (tab_width == -2)
    fatal("bad it (init_tabs) capability");

  tab_width = init_tabs != -1 ? init_tabs : 8;

  if (over_strike == -1)
    fatal("bad os (over_strike) capability");

  do_glyph_composition_by_overstriking =
    accept_glyph_composition_by_overstriking && over_strike;

  static const struct {
    char *cap;
    char const *nullmsg;
    char const *negonemsg;
  } string_caps[] = {
    // all the string capabilities we use in the program
    {enter_bold_mode, "can't make text bold", "bad bold capability"},
    {enter_italics_or_the_like_mode, "can't make text italic (or the like)",
      "bad italics (or the like) capability"},
    {exit_attribute_mode, "can't disable text attributes",
      "bad sgr0 (exit attributes) capability"},
    {set_a_foreground, "can't colorize text",
      "bad setaf (foreground color) capability"},
    {set_a_background, "can't colorize text",
      "bad setab (background color) capability"},
  };

  for (int i = 0; i < sizeof string_caps / sizeof *string_caps; ++i) {
    if (string_caps[i].cap == NULL) {
      if (over_strike && !want_real_italics && !want_reverse_video_for_italics) {
	use_overstriking_drawing_scheme = true;
	update_options();
	return;
      }
      else
	fatal(string_caps[i].nullmsg);
    }
    else if (string_caps[i].cap == (char *)-1)
      fatal(string_caps[i].negonemsg);
  }
}

int main(int argc, char **argv)
{
  program_name = argv[0];
  static char stderr_buf[BUFSIZ];
  if (getenv("GROFF_NO_SGR"))
    use_overstriking_drawing_scheme = true;
  setbuf(stderr, stderr_buf);
  setlocale(LC_CTYPE, "");
  int c;
  static const struct option long_options[] = {
    { "help", no_argument, 0, CHAR_MAX + 1 },
    { "version", no_argument, 0, 'v' },
    { NULL, 0, 0, 0 }
  };
  while ((c = getopt_long(argc, argv, "bBcdfF:hiI:oruUv", long_options, NULL))
	 != EOF)
    switch(c) {
    case 'v':
      printf("GNU grotty (groff) version %s\n", Version_string);
      exit(EXIT_SUCCESS);
      break;
    case 'i':
      // Use italic font instead of underlining.
      want_real_italics = true;
      break;
    case 'I':
      // ignore include search path
      break;
    case 'b':
      // Do not embolden by overstriking.
      want_emboldening_by_overstriking = false;
      break;
    case 'c':
      // Use old scheme for emboldening and underline.
      use_overstriking_drawing_scheme = true;
      break;
    case 'u':
      // Do not underline.
      want_italics_by_underlining = false;
      break;
    case 'o':
      // Do not overstrike (other than emboldening and underlining).
      accept_glyph_composition_by_overstriking = false;
      break;
    case 'r':
      // Use reverse mode instead of underlining.
      want_reverse_video_for_italics = true;
      break;
    case 'B':
      // Do bold-underlining as bold.
      bold_underline_mode_option = BOLD_MODE;
      break;
    case 'U':
      // Do bold-underlining as underlining.
      bold_underline_mode_option = UNDERLINE_MODE;
      break;
    case 'h':
      // Use horizontal tabs.
      want_horizontal_tabs = true;
      break;
    case 'f':
      want_form_feeds = true;
      break;
    case 'F':
      font::command_line_font_dir(optarg);
      break;
    case 'd':
      // Ignore \D commands.
      allow_drawing_commands = false;
      break;
    case CHAR_MAX + 1: // --help
      usage(stdout);
      break;
    case '?':
      usage(stderr);
      exit(EXIT_FAILURE);
      break;
    default:
      assert(0 == "unhandled getopt_long return value");
    }
  update_options();
  if (optind >= argc)
    do_file("-");
  else {
    for (int i = optind; i < argc; i++)
      do_file(argv[i]);
  }
  return 0;
}

static void usage(FILE *stream)
{
  fprintf(stream,
"usage: %s [-bBcdfhioruU] [-F font-directory] [file ...]\n"
"usage: %s {-v | --version}\n"
"usage: %s --help\n",
	  program_name, program_name, program_name);
  if (stdout == stream) {
    fputs(
"\n"
"Translate the output of troff(1) into a form suitable for\n"
"typewriter‐like devices, including terminal emulators.  See the\n"
"grotty(1) manual page.\n",
	  stream);
    exit(EXIT_SUCCESS);
  }
}

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:
