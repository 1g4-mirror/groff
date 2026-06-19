/* Copyright 1989-2002 Free Software Foundation, Inc.
             2025-2026 G. Branden Robinson

Written by James Clark (jjc@jclark.com)

This file is part of groff, the GNU roff typesetting system.

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stddef.h> // size_t
#include <stdio.h> // FILE, putc(), sprintf()
#include <stdlib.h> // calloc()
#include <string.h> // memchr(), memcmp(), memcpy(), memmem(), memset(),
		    // strlen(), size_t

// POSX/operating system services
#include <sys/types.h> // ssize_t

#include <new> // std::bad_alloc

#include "cset.h" // csprint()
#include "errarg.h" // prerequisite of "error.h"
#include "error.h" // fatal()
#include "lib.h"
#include "json-encode.h" // json_char, json_encode_char()

#include "stringclass.h"

// TODO 1: Replace all this memory management stuff with vector<char>.
// TODO 2: Replace this entire class.  See Savannah #67735.

// Invariant: a groff string must always be null-terminated, because its
// `contents()` member function returns a `const char *`.

// An initial buffer size of 64 appears to balance build time against
// minimization of reallocations.
static const size_t initial_string_buffer_size = 64;

static char *salloc(size_t len, size_t *sizep)
{
  char *p = 0 /* nullptr */;
  size_t amount = initial_string_buffer_size;
  if (len >= amount)
    amount = len + 1 /* '\0' */;
  try {
    p = new char[*sizep = amount];
  }
  catch (const std::bad_alloc &exc) {
    fatal("cannot allocate %1 bytes for string allocation", amount);
  }
  assert(*sizep > 0);
  memset(p, 0, *sizep);
  return p;
}

static char *sfree_alloc(char *ptr, size_t oldsz, size_t len,
			 size_t *sizep)
{
  if (oldsz >= len) {
    *sizep = oldsz;
    memset((ptr + len), 0, (oldsz - len));
    return ptr;
  }
  delete[] ptr;
  char *p = 0 /* nullptr */;
  size_t amount = initial_string_buffer_size;
  if (len >= amount)
    amount = len + 1 /* '\0' */;
  try {
    p = new char[*sizep = amount];
  }
  catch (const std::bad_alloc &exc) {
    fatal("cannot allocate %1 bytes for string replacement allocation",
	  amount);
  }
  memset(p, 0, amount);
  return p;
}

static char *srealloc(char *ptr, size_t oldsz, size_t oldlen,
		      size_t newlen, size_t *sizep)
{
  if (oldsz >= newlen) {
    *sizep = oldsz;
    memset((ptr + oldlen), 0, (newlen - oldsz));
    return ptr;
  }
  size_t amount = newlen;
  if (0 == amount)
    amount = initial_string_buffer_size;
  else
    // If the string changes size once, assume it will change again, in
    // an effort to avoid excessive reallocations.
    amount = newlen * 2;
  char *p = 0 /* nullptr */;
  try {
    p = new char[*sizep = amount];
  }
  catch (const std::bad_alloc &exc) {
    fatal("cannot allocate %1 bytes for string reallocation", amount);
  }
  if ((oldlen < newlen) && (oldlen != 0)) {
    assert(amount > 0);
    memset(p, 0, amount);
    memcpy(p, ptr, oldlen);
  }
  delete[] ptr;
  return p;
}

string::string() : len(0), sz(initial_string_buffer_size)
{
  ptr = salloc(initial_string_buffer_size, &sz);
  assert(ptr != 0 /* nullptr */);
}

string::string(const char *p, size_t n) : len(n)
{
  ptr = salloc(n, &sz);
  assert(ptr != 0 /* nullptr */);
  memset(ptr, 0, sz);
  if (n != 0)
    memcpy(ptr, p, n);
}

string::string(const char *p)
{
  if (0 /* nullptr */ == p) {
    len = 0;
    ptr = salloc(initial_string_buffer_size, &sz);
    assert(ptr != 0 /* nullptr */);
  }
  else {
    len = strlen(p);
    ptr = salloc(len, &sz);
    assert(ptr != 0 /* nullptr */);
    if (len < sz)
      memset(ptr, 0, sz);
    if (len != 0)
      memcpy(ptr, p, len);
  }
}

#if 0
string::string(char c) : len(1)
{
  ptr = salloc(1, &sz);
  assert(ptr != 0 /* nullptr */);
  *ptr = c;
}
#endif

string::string(const string &s) : len(s.len)
{
  ptr = salloc(len, &sz);
  assert(ptr != 0 /* nullptr */);
  if (sz > 0)
    memset(ptr, 0, sz);
  if (len != 0)
    memcpy(ptr, s.ptr, len);
}

string::~string()
{
  delete[] ptr;
}

string &string::operator=(const string &s)
{
  ptr = sfree_alloc(ptr, sz, s.len, &sz);
  assert(ptr != 0 /* nullptr */);
  len = s.len;
  if (len != 0)
    memcpy(ptr, s.ptr, len);
  return *this;
}

string &string::operator=(const char *p)
{
  assert(p != 0 /* nullptr */);
  if (0 /* nullptr */ == p)
    p = "";
  size_t slen = strlen(p);
  ptr = sfree_alloc(ptr, sz, slen, &sz);
  assert(ptr != 0 /* nullptr */);
  len = slen;
  if (len != 0)
    memcpy(ptr, p, len);
  return *this;
}

string &string::operator=(char c)
{
  ptr = sfree_alloc(ptr, sz, 1, &sz);
  assert(ptr != 0 /* nullptr */);
  len = 1;
  *ptr = c;
  return *this;
}

void string::move(string &s)
{
  sfree_alloc(ptr, sz, s.len, &sz);
  memcpy(ptr, s.ptr, s.len);
  len = s.len;
  s.clear();
  assert(ptr != 0 /* nullptr */);
}

void string::embiggen()
{
  ptr = srealloc(ptr, sz, len, len + 1, &sz);
  assert(ptr != 0 /* nullptr */);
}

string &string::operator+=(const char *p)
{
  if (p != 0 /* nullptr */) {
    size_t n = strlen(p);
    size_t newlen = len + n;
    if (newlen > sz) {
      ptr = srealloc(ptr, sz, len, newlen, &sz);
      assert(ptr != 0 /* nullptr */);
    }
    memcpy(ptr + len, p, n);
    len = newlen;
  }
  return *this;
}

string &string::operator+=(const string &s)
{
  if (s.len != 0) {
    size_t newlen = len + s.len;
    if (newlen > sz) {
      ptr = srealloc(ptr, sz, len, newlen, &sz);
      assert(ptr != 0 /* nullptr */);
    }
    memcpy(ptr + len, s.ptr, s.len);
    len = newlen;
  }
  return *this;
}

void string::append(const char *p, size_t n)
{
  if (n > 0) {
    size_t newlen = len + n;
    if (newlen > sz) {
      ptr = srealloc(ptr, sz, len, newlen, &sz);
      assert(ptr != 0 /* nullptr */);
    }
    memcpy(ptr + len, p, n);
    len = newlen;
  }
}

string::string(const char *s1, size_t n1, const char *s2, size_t n2)
{
  len = n1 + n2;
  if (0 == len) {
    ptr = salloc(initial_string_buffer_size, &sz);
    assert(ptr != 0 /* nullptr */);
  }
  else {
    ptr = salloc(len, &sz);
    assert(ptr != 0 /* nullptr */);
    if (0 == n1)
      memcpy(ptr, s2, n2);
    else {
      memcpy(ptr, s1, n1);
      if (n2 != 0)
	memcpy(ptr + n1, s2, n2);
    }
  }
}

#if 0
bool operator<=(const string &s1, const string &s2)
{
  return ((s1.len <= s2.len)
	  ? ((s1.len == 0) || (memcmp(s1.ptr, s2.ptr, s1.len) <= 0))
	  : ((s2.len != 0) && (memcmp(s1.ptr, s2.ptr, s2.len) < 0)));
}

bool operator<(const string &s1, const string &s2)
{
  return ((s1.len < s2.len)
	  ? ((s1.len == 0) || (memcmp(s1.ptr, s2.ptr, s1.len) <= 0))
	  : ((s2.len != 0) && (memcmp(s1.ptr, s2.ptr, s2.len) < 0)));
}

bool operator>=(const string &s1, const string &s2)
{
  return ((s1.len >= s2.len)
	  ? ((s2.len == 0) || (memcmp(s1.ptr, s2.ptr, s2.len) >= 0))
	  : ((s1.len != 0) && (memcmp(s1.ptr, s2.ptr, s1.len) > 0)));
}

bool operator>(const string &s1, const string &s2)
{
  return ((s1.len > s2.len)
	  ? ((s2.len == 0) || (memcmp(s1.ptr, s2.ptr, s2.len) >= 0))
	  : ((s1.len != 0) && (memcmp(s1.ptr, s2.ptr, s1.len) > 0)));
}
#endif

void string::set_length(size_t i)
{
  if (i > sz) {
    ptr = srealloc(ptr, sz, len, i, &sz);
    assert(ptr != 0 /* nullptr */);
  }
  len = i;
}

void string::clear()
{
  assert(ptr != 0 /* nullptr */);
  if (ptr != 0 /* nullptr */)
    memset(ptr, 0, sz);
  else
    ptr = salloc(0, &sz); // unreachable unless `NDEBUG`
  len = 0;
}

int string::search(const char c) const
{
  const char *p = ptr
		  ? static_cast<const char *>(memchr(ptr, c, len))
		  : 0 /* nullptr */;
  return (p != 0 /* nullptr */) ? (p - ptr) : -1;
}

bool string::contains(const char c) const
{
  return (search(c) >= 0);
}

// Return index of substring `c` in string, -1 if not found.
ssize_t string::find(const char *c) const
{
  const char *p = ptr
		  ? static_cast<const char *>(memmem(ptr, len, c,
						     strlen(c)))
		  : 0  /* nullptr */;
  return (p != 0 /* nullptr */) ? (p - ptr) : -1;
}

// Return pointer to null-terminated C string; any nulls internal to the
// string are omitted.  The caller is responsible for `free()`ing the
// returned storage.
char *string::extract() const
{
  char *p = ptr;
  size_t n = len;
  int nnuls = 0;
  size_t i;
  for (i = 0; i < n; i++)
    if (p[i] == '\0')
      nnuls++;
  char *q = static_cast<char *>(calloc(n + 1 - nnuls, sizeof(char)));
  if (q != 0 /* nullptr */) {
    char *r = q;
    for (i = 0; i < n; i++)
      if (p[i] != '\0')
	*r++ = p[i];
    *r = '\0';
  }
  return q;
}

// Compute length of JSON representation of object.
size_t string::json_length() const
{
  size_t n = len;
  const char *p = ptr;
  char ch;
  int nextrachars = 2; // leading and trailing double quotes
  for (size_t i = 0; i < n; i++) {
    ch = p[i];
    assert ((ch >= 0) && (ch <= 127));
    // These printable characters require escaping.
    if (('"' == ch) || ('\\' == ch) || ('/' == ch))
      nextrachars++;
    else if (csprint(ch))
      ;
    else
      switch (ch) {
      case '\b':
      case '\f':
      case '\n':
      case '\r':
      case '\t':
	nextrachars++;
	break;
      default:
	nextrachars += 5;
    }
  }
  return (n + nextrachars);
}

// Like `extract()`, but double-quote the string and escape characters
// per JSON and emit nulls.
const char *string::json_extract() const
{
  const char *p = ptr;
  char *r;
  size_t n = len;
  size_t i;
  char *q = static_cast<char *>(calloc(this->json_length() + 1,
				       sizeof (char)));
  if (q != 0 /* nullptr */) {
    r = q;
    *r++ = '"';
    json_char ch;
    for (i = 0; i < n; i++, p++) {
      ch = json_encode_char(*p);
      for (size_t j = 0; j < ch.len; j++)
	*r++ = ch.buf[j];
    }
    *r++ = '"';
  }
  else
    return strdup("\"\"");
  *r++ = '\0';
  return q;
}

// Dump string in JSON representation to standard error stream.
void string::json_dump() const
{
  const char *repr = this->json_extract();
  size_t jsonlen = this->json_length();
  // Write it out by character to keep libc string functions from
  // interpreting escape sequences.
  for (size_t i = 0; i < jsonlen; i++)
    fputc(repr[i], stderr);
  free(const_cast<char *>(repr));
}

// TODO: This function has 1 call site, in tbl/main.cpp:process_data().
// Consider either open-coding this logic there, or generalizing this
// function to a `filter()` that takes a character parameter.
void string::remove_spaces()
{
  // This method is arguably inefficient, but see above regarding the
  // one call site.
  size_t l = len - 1;
  while ((l < len) && (ptr[l] == ' '))
    l--;
  char *p = ptr;
  if (l > 0)
    while (*p == ' ') {
      p++;
      l--;
    }
  if ((len - 1) != l) {
    len = l + 1;
    char *tmp = 0 /* nullptr */;
    assert(sz > 0);
    try {
      tmp = new char[sz];
    }
    catch (const std::bad_alloc &exc) {
      fatal("cannot allocate %1 bytes for removal of spaces",
	    " from string", sz);
    }
    memset(tmp, 0, sz);
    memcpy(tmp, p, len);
    delete[] ptr;
    ptr = tmp;
    assert(ptr != 0 /* nullptr */);
  }
}

void put_string(const string &s, FILE *fp)
{
  size_t len = s.length();
  const char *ptr = s.contents();
  assert(ptr != 0 /* nullptr */);
  for (size_t i = 0; i < len; i++)
    putc(ptr[i], fp);
}

string as_string(size_t i)
{
  static char buf[INT_DIGITS + 2];
  sprintf(buf, "%lu", i);
  return string(buf);
}

// Local Variables:
// fill-column: 72
// mode: C++
// End:
// vim: set cindent noexpandtab shiftwidth=2 textwidth=72:
