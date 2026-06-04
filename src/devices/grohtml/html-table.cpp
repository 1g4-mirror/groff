/* Copyright 2002-2007 Free Software Foundation, Inc.
 *
 *  Gaius Mulley (gaius@glam.ac.uk) wrote html-table.cpp
 *
 *  html-table.h
 *
 *  provides the methods necessary to handle indentation and tab
 *  positions using html tables.
 */

/*
This file is part of groff, the GNU roff typesetting system.

groff is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your
option) any later version.

groff is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h> // atoi()

#include <new> // std::bad_alloc

// libgroff
#include "symbol.h" // prerequisite of color.h
#include "color.h"
#include "cset.h" // csspace()
#include "errarg.h" // prerequisite of error.h
#include "error.h" // fatal()
#include "stringclass.h"

// grohtml
#include "html.h"
#include "html-table.h"
#include "html-text.h"

extern html_dialect dialect;


tabs::tabs ()
  : tab(0 /* nullptr */)
{
}

tabs::~tabs ()
{
  delete_list();
}

/*
 *  delete_list - frees the tab list and sets tab to a null pointer.
 */

void tabs::delete_list (void)
{
  tab_position *p = tab;
  tab_position *q;

  while (p != 0 /* nullptr */) {
    q = p;
    p = p->next;
    delete q;
  }
  tab = 0 /* nullptr */;
}

void tabs::clear (void)
{
  delete_list();
}

/*
 *  compatible - returns true if the tab stops in, s, do
 *               not conflict with the current tab stops.
 *               The new tab stops are _not_ placed into
 *               this class.
 */

int tabs::compatible (const char *s)
{
  char align;
  int  total=0;
  tab_position *last = tab;

  if (last == 0 /* nullptr */)
    return false;  // no tab stops defined

  // move over tag name
  while ((*s != '\0') && !csspace(*s))
    s++;

  while (*s != '\0' && last != 0 /* nullptr */) {
    // move over whitespace
    while ((*s != '\0') && csspace(*s))
      s++;
    // collect alignment
    align = *s;
    // move over alignment
    s++;
    // move over whitespace
    while ((*s != '\0') && csspace(*s))
      s++;
    // collect tab position
    total = atoi(s);
    // move over tab position
    while ((*s != '\0') && !csspace(*s))
      s++;
    if (last->alignment != align || last->position != total)
      return false;

    last = last->next;
  }
  return true;
}

/*
 *  init - scans the string, s, and initializes the tab stops.
 */

void tabs::init (const char *s)
{
  char align;
  int  total=0;
  tab_position *last = 0 /* nullptr */;

  clear(); // remove any tab stops

  // move over tag name
  while ((*s != '\0') && !csspace(*s))
    s++;

  while (*s != '\0') {
    // move over whitespace
    while ((*s != '\0') && csspace(*s))
      s++;
    // collect alignment
    align = *s;
    // move over alignment
    s++;
    // move over whitespace
    while ((*s != '\0') && csspace(*s))
      s++;
    // collect tab position
    total = atoi(s);
    // move over tab position
    while ((*s != '\0') && !csspace(*s))
      s++;
    tab_position *newtab = 0 /* nullptr */;
    try {
      newtab = new tab_position;
    }
    catch (const std::bad_alloc &exc) {
      fatal("cannot allocate storage for tab_position object");
    }
    if (last == 0 /* nullptr */) {
      tab = newtab;
      last = tab;
    } else {
      last->next = newtab;
      last = last->next;
    }
    last->alignment = align;
    last->position = total;
    last->next = 0 /* nullptr */;
  }
}

/*
 *  check_init - define tab stops using, s, providing none already exist.
 */

void tabs::check_init (const char *s)
{
  if (tab == 0 /* nullptr */)
    init(s);
}

/*
 *  find_tab - returns the tab number corresponding to the position, pos.
 */

int tabs::find_tab (int pos)
{
  tab_position *p;
  int i=0;

  for (p = tab; p != 0 /* nullptr */; p = p->next) {
    i++;
    if (p->position == pos)
      return i;
  }
  return 0;
}

/*
 *  get_tab_pos - returns the, nth, tab position
 */

int tabs::get_tab_pos (int n)
{
  tab_position *p;

  n--;
  for (p = tab; (p != 0 /* nullptr */) && (n>0); p = p->next) {
    n--;
    if (n == 0)
      return p->position;
  }
  return 0;
}

char tabs::get_tab_align (int n)
{
  tab_position *p;

  n--;
  for (p = tab; (p != 0 /* nullptr */) && (n>0); p = p->next) {
    n--;
    if (n == 0)
      return p->alignment;
  }
  return 'L';
}

/*
 *  dump_tab - display tab positions
 */

void tabs::dump_tabs (void)
{
  int i=1;
  tab_position *p;

  for (p = tab; p != 0 /* nullptr */; p = p->next) {
    printf("tab %d is %d\n", i, p->position);
    i++;
  }
}

/*
 *  html_table - methods
 */

html_table::html_table (simple_output *op, int linelen)
  : out(op), columns(0 /* nullptr */), linelength(linelen),
    last_col(0 /* nullptr */), start_space(false)
{
  try {
    tab_stops = new tabs();
  }
  catch (const std::bad_alloc &exc) {
    fatal("cannot allocate storage for tab_stops object");
  }
}

html_table::~html_table ()
{
  cols *c;
  if (tab_stops != 0 /* nullptr */)
    delete tab_stops;

  c = columns;
  while (columns != 0 /* nullptr */) {
    columns = columns->next;
    delete c;
    c = columns;
  }
}

/*
 *  remove_cols - remove a list of columns as defined by, c.
 */

void html_table::remove_cols (cols *c)
{
  cols *p;

  while (c != 0 /* nullptr */) {
    p = c;
    c = c->next;
    delete p;
  }
}

/*
 *  set_linelength - sets the line length value in this table.
 *                   It also adds an extra blank column to the
 *                   table should linelen exceed the last column.
 */

void html_table::set_linelength (int linelen)
{
  cols *p = 0 /* nullptr */;
  cols *c;
  linelength = linelen;

  for (c = columns; c != 0 /* nullptr */; c = c->next) {
    if (c->right > linelength) {
      c->right = linelength;
      remove_cols(c->next);
      c->next = 0 /* nullptr */;
      return;
    }
    p = c;
  }
  if (p != 0 /* nullptr */ && p->right > 0 && linelength > p->right)
    add_column(p->no+1, p->right, linelength, 'L');
}

/*
 *  get_effective_linelength -
 */

int html_table::get_effective_linelength (void)
{
  if (columns != 0 /* nullptr */)
    return linelength - columns->left;
  else
    return linelength;
}

/*
 *  add_indent - adds the indent to a table.
 */

void html_table::add_indent (int indent)
{
  if (columns != 0 /* nullptr */ && columns->left > indent)
    add_column(0, indent, columns->left, 'L');
}

/*
 *  emit_table_header - emits the html header for this table.
 */

void html_table::emit_table_header (int space)
{
  if (columns == 0 /* nullptr */)
    return;

  // dump_table();

  last_col = 0 /* nullptr */;
  if (linelength > 0) {
    out->nl();
    out->nl();

    out->put_string("<table width=\"100%\"")
      .put_string(" border=\"0\" rules=\"none\" frame=\"void\"\n")
      .put_string("       cellspacing=\"0\" cellpadding=\"0\"");
    out->put_string(">")
      .nl();
    if (dialect == xhtml)
      emit_colspan();
    out->put_string("<tr valign=\"top\" align=\"left\"");
    if (space) {
      out->put_string(" style=\"margin-top: ");
      out->put_string(STYLE_VERTICAL_SPACE);
      out->put_string("\"");
    }
    out->put_string(">").nl();
  }
}

/*
 *  get_right - returns the right most position of this column.
 */

int html_table::get_right (cols *c)
{
  if (c != 0 /* nullptr */ && c->right > 0)
    return c->right;
  if (c->next != 0 /* nullptr */)
    return c->left;
  return linelength;
}

/*
 *  set_space - assigns start_space. Used to determine the
 *              vertical alignment when generating the next table row.
 */

void html_table::set_space (int space)
{
  start_space = space;
}

/*
 *  emit_colspan - emits a series of colspan entries defining the
 *                 table columns.
 */

void html_table::emit_colspan (void)
{
  cols *b = columns;
  cols *c = columns;
  int   width = 0;

  out->put_string("<colgroup>");
  while (c != 0 /* nullptr */) {
    if (b != 0 /* nullptr */ && b != c && is_gap(b))
      /*
       *   blank column for gap
       */
      out->put_string("<col width=\"")
	.put_number(is_gap(b))
	.put_string("%\" class=\"center\"></col>")
	.nl();

    width = (get_right(c)*100 + get_effective_linelength()/2)
	      / get_effective_linelength()
             - (c->left*100 + get_effective_linelength()/2)
		/get_effective_linelength();
    switch (c->alignment) {
    case 'C':
      out->put_string("<col width=\"")
	  .put_number(width)
	  .put_string("%\" class=\"center\"></col>")
	  .nl();
      break;
    case 'R':
      out->put_string("<col width=\"")
	  .put_number(width)
	  .put_string("%\" class=\"right\"></col>")
	  .nl();
      break;
    default:
      out->put_string("<col width=\"")
	  .put_number(width)
	  .put_string("%\"></col>")
	  .nl();
    }
    b = c;
    c = c->next;
  }
  out->put_string("</colgroup>").nl();
}

/*
 *  emit_td - writes out a <td> tag with a corresponding width
 *            if the dialect is html4.
 */

void html_table::emit_td (int percentage, const char *s)
{
  if (percentage) {
    if (dialect == html4) {
      out->put_string("<td width=\"")
	.put_number(percentage)
	.put_string("%\"");
      if (s != 0 /* nullptr */)
	out->put_string(s);
      out->nl();
    }
    else {
      out->put_string("<td");
      if (s != 0 /* nullptr */)
	out->put_string(s);
      out->nl();
    }
  }
}

/*
 *  emit_col - moves onto column, n.
 */

void html_table::emit_col (int n)
{
  cols *c = columns;
  cols *b = columns;
  int   width = 0;

  // must be a different row
  if (last_col != 0 /* nullptr */ && n <= last_col->no)
    emit_new_row();

  while (c != 0 /* nullptr */ && c->no < n)
    c = c->next;

  // can we find column, n?
  if (c != 0 /* nullptr */ && c->no == n) {
    // shutdown previous column
    if (last_col != 0 /* nullptr */)
      out->put_string("</td>").nl();

    // find previous column
    if (last_col == 0 /* nullptr */)
      b = columns;
    else
      b = last_col;

    // have we a gap?
    if (last_col != 0 /* nullptr */) {
      emit_td(is_gap(b), "></td>");
      b = b->next;
    }

    // move across to column n
    while (b != c) {
      // we compute the difference after converting positions
      // to avoid rounding errors
      width = (get_right(b)*100 + get_effective_linelength()/2)
		/ get_effective_linelength()
	      - (b->left*100 + get_effective_linelength()/2)
		  /get_effective_linelength();
      emit_td(width, "></td>");
      // have we a gap?
      emit_td(is_gap(b), "></td>");
      b = b->next;
    }
    width = (get_right(b)*100 + get_effective_linelength()/2)
	      / get_effective_linelength()
	    - (b->left*100 + get_effective_linelength()/2)
		/get_effective_linelength();
    switch (b->alignment) {
    case 'C':
      emit_td(width, " align=center>");
      break;
    case 'R':
      emit_td(width, " align=right>");
      break;
    default:
      emit_td(width);
    }
    // remember column, b
    last_col = b;
  }
}

/*
 *  finish_row -
 */

void html_table::finish_row (void)
{
  int n = 0;
  cols *c;

  if (last_col != 0 /* nullptr */) {
    for (c = last_col->next; c != 0 /* nullptr */; c = c->next)
      n = c->no;

    if (n > 0)
      emit_col(n);
#if 1
    if (last_col != 0 /* nullptr */) {
      out->put_string("</td>");
      last_col = 0 /* nullptr */;
    }
#endif
    out->put_string("</tr>").nl();
  }
}

/*
 *  emit_new_row - move to the next row.
 */

void html_table::emit_new_row (void)
{
  finish_row();

  out->put_string("<tr valign=\"top\" align=\"left\"");
  if (start_space) {
    out->put_string(" style=\"margin-top: ");
    out->put_string(STYLE_VERTICAL_SPACE);
    out->put_string("\"");
  }
  out->put_string(">").nl();
  start_space = false;
  last_col = 0 /* nullptr */;
}

void html_table::emit_finish_table (void)
{
  finish_row();
  out->put_string("</table>");
}

/*
 *  add_column - adds a column. It returns false if hstart..hend
 *               crosses into a different columns.
 */

int html_table::add_column (int coln, int hstart, int hend, char align)
{
  cols *c = get_column(coln);

  if (c == 0 /* nullptr */)
    return insert_column(coln, hstart, hend, align);
  else
    return modify_column(c, hstart, hend, align);
}

/*
 *  get_column - returns the column, coln.
 */

cols *html_table::get_column (int coln)
{
  cols *c = columns;

  while (c != 0 /* nullptr */ && coln != c->no)
    c = c->next;

  if (c != 0 /* nullptr */ && coln == c->no)
    return c;
  else
    return 0 /* nullptr */;
}

/*
 *  insert_column - inserts a column, coln.
 *                  It returns true if it does not bump into
 *                  another column.
 */

int html_table::insert_column (int coln, int hstart, int hend, char align)
{
  cols *c = columns;
  cols *l = columns;
  cols *n = 0 /* nullptr */;

  while (c != 0 /* nullptr */ && c->no < coln) {
    l = c;
    c = c->next;
  }
  if (l != 0 /* nullptr */ && l->no>coln && hend > l->left)
    return false;	// new column bumps into previous one

  l = 0 /* nullptr */;
  c = columns;
  while (c != 0 /* nullptr */ && c->no < coln) {
    l = c;
    c = c->next;
  }

  if ((l != 0 /* nullptr */) && (hstart < l->right))
    return false;	// new column bumps into previous one

  if ((l != 0 /* nullptr */) && (l->next != 0 /* nullptr */) &&
      (l->next->left < hend))
    return false;  // new column bumps into next one

  try {
    n = new cols;
  }
  catch (const std::bad_alloc &exc) {
    fatal("cannot allocate storage for cols object");
  }
  if (l == 0 /* nullptr */) {
    n->next = columns;
    columns = n;
  } else {
    n->next = l->next;
    l->next = n;
  }
  n->left = hstart;
  n->right = hend;
  n->no = coln;
  n->alignment = align;
  return true;
}

/*
 *  modify_column - given a column, c, modify the width to
 *                  contain hstart..hend.
 *                  It returns true if it does not clash with
 *                  the next or previous column.
 */

int html_table::modify_column (cols *c, int hstart, int hend, char align)
{
  cols *l = columns;

  while (l != 0 /* nullptr */ && l->next != c)
    l = l->next;

  if ((l != 0 /* nullptr */) && (hstart < l->right))
    return false;	// new column bumps into previous one

  if ((c->next != 0 /* nullptr */) && (c->next->left < hend))
    return false;  // new column bumps into next one

  if (c->left > hstart)
    c->left = hstart;

  if (c->right < hend)
    c->right = hend;

  c->alignment = align;

  return true;
}

/*
 *  find_tab_column - finds the column number for position, pos.
 *                    It searches through the list tab stops.
 */

int html_table::find_tab_column (int pos)
{
  // remember the first column is reserved for untabbed glyphs
  return tab_stops->find_tab(pos)+1;
}

/*
 *  find_column - find the column number for position, pos.
 *                It searches through the list of columns.
 */

int html_table::find_column (int pos)
{
  int   p=0;
  cols *c;

  for (c = columns; c != 0 /* nullptr */; c = c->next) {
    if (c->left > pos)
      return p;
    p = c->no;
  }
  return p;
}

/*
 *  no_columns - returns the number of table columns (rather than tabs)
 */

int html_table::no_columns (void)
{
  int n=0;
  cols *c;

  for (c = columns; c != 0 /* nullptr */; c = c->next)
    n++;
  return n;
}

/*
 *  is_gap - returns the gap between column, c, and the next column.
 */

int html_table::is_gap (cols *c)
{
  if (c == 0 /* nullptr */
      || c->right <= 0
      || c->next == 0 /* nullptr */)
    return 0;
  else
    // we compute the difference after converting positions
    // to avoid rounding errors
    return (c->next->left*100 + get_effective_linelength()/2)
	     / get_effective_linelength()
	   - (c->right*100 + get_effective_linelength()/2)
	       / get_effective_linelength();
}

/*
 *  no_gaps - returns the number of table gaps between the columns
 */

int html_table::no_gaps (void)
{
  int n=0;
  cols *c;

  for (c = columns; c != 0 /* nullptr */; c = c->next)
    if (is_gap(c))
      n++;
  return n;
}

/*
 *  get_tab_pos - returns the, nth, tab position
 */

int html_table::get_tab_pos (int n)
{
  return tab_stops->get_tab_pos(n);
}

char html_table::get_tab_align (int n)
{
  return tab_stops->get_tab_align(n);
}


void html_table::dump_table (void)
{
  if (columns != 0 /* nullptr */) {
    cols *c;
    for (c = columns; c != 0 /* nullptr */; c = c->next) {
      printf("column %d  %d..%d  %c\n", c->no, c->left, c->right, c->alignment);
    }
  } else
    tab_stops->dump_tabs();
}

/*
 *  html_indent - creates an indent with indentation, ind, given
 *                a line length of linelength.
 */

html_indent::html_indent (simple_output *op, int ind, int pageoffset, int linelength)
{
  try {
    table = new html_table(op, linelength);
  }
  catch (const std::bad_alloc &exc) {
    fatal("cannot allocate storage for html_table object");
  }

  table->add_column(1, ind+pageoffset, linelength, 'L');
  table->add_indent(pageoffset);
  in = ind;
  pg = pageoffset;
  ll = linelength;
}

html_indent::~html_indent (void)
{
  end();
  delete table;
}

void html_indent::begin (int space)
{
  if (in + pg == 0) {
    if (space) {
      table->out->put_string(" style=\"margin-top: ");
      table->out->put_string(STYLE_VERTICAL_SPACE);
      table->out->put_string("\"");
    }
  }
  else {
    //
    // we use exactly the same mechanism for calculating
    // indentation as html_table::emit_col
    //
    table->out->put_string(" style=\"margin-left:")
      .put_number(((in + pg) * 100 + ll/2) / ll -
		  (ll/2)/ll)
      .put_string("%;");

    if (space) {
      table->out->put_string(" margin-top: ");
      table->out->put_string(STYLE_VERTICAL_SPACE);
    }
    table->out->put_string("\"");
  }
}

void html_indent::end (void)
{
}

/*
 *  get_reg - collects the registers as supplied during initialization.
 */

void html_indent::get_reg (int *ind, int *pageoffset, int *linelength)
{
  *ind = in;
  *pageoffset = pg;
  *linelength = ll;
}

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:
