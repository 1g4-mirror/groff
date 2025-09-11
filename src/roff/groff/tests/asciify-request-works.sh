#!/bin/sh
#
# Copyright 2025 G. Branden Robinson
#
# This file is part of groff.
#
# groff is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# groff is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

groff="${abs_top_builddir:-.}/test-groff"

fail=

wail () {
  echo ...FAILED >&2
  fail=YES
}

input='.
.ec #
!#?qux#?!
.br
.box DIV1
foobar	baz#" leader, then tab
.char #[Z] ZYX
A#[dq]#[e aa]#[u00E1]#[u0106]i#[fl]o#[Fl]#[Z]
.br
1@#&2@#)3@#c
4@#c
.tag foo
.ds tab #t#"
.ds leader #a#"
5@#X"bar"6@#v".5v"7@#*[tab]8@#h"2m"9@# 10@#~11@#:12@#*[leader]13@#
#?jat#?14@
.br
.box
.asciify DIV1
.DIV1
.br
.ec $
.box DIV2
$%antimatter15@$b"hup"16@
.br
.box
.asciify DIV2
.DIV2
.ec
.'

output=$(printf "%s\n" "$input" | "$groff" -T ps -a)
echo "$output"

echo "checking textification of ordinary character 'A'" >&2
echo "$output" | grep -q ' A' || wail

echo "checking textification of special character 'dq'" >&2
echo "$output" | grep -q '<dq>' || wail

echo "checking textification of composite special character 'e aa'" >&2
echo "$output" | grep -q "<'e>" || wail

echo "checking textification of Unicode special character with Latin-1" \
  "mapping 'u00E1'" >&2
echo "$output" | grep -q "<'a>" || wail

echo "checking textification of decomposable (with Basic Latin base" \
  "character) Unicode special character 'u0106'" >&2
echo "$output" | grep -q "C<aa>" || wail

echo "checking textification of ligature special character 'fl'" >&2
echo "$output" | grep -q "i<fl>" || wail

echo "checking textification of ligature special character 'Fl'" >&2
echo "$output" | grep -q "of<fl>" || wail

echo "checking textification of user-defined special character 'Z'" >&2
echo "$output" | grep -q "ZYX" || wail

echo "checking textification of dummy character escape sequence" >&2
echo "$output" | grep -q "1@2" || wail

echo "checking textification of EOS-transparent dummy character" \
   "escape sequence" >&2
echo "$output" | grep -q "2@3" || wail

echo "checking textification of output line continuation escape" \
   "sequence" >&2
echo "$output" | grep -q "3@4" || wail

echo "checking textification of tag request" >&2
echo "$output" | grep -q "4@5" || wail

echo "checking textification of device extension escape sequence" >&2
echo "$output" | grep -q "5@6" || wail

echo "checking textification of vertical motion escape sequence" >&2
echo "$output" | grep -q "6@7" || wail

echo "checking textification of (tab) horizontal motion escape sequence" >&2
echo "$output" | grep -q "7@8" || wail

echo "checking textification of (plain) horizontal motion escape sequence" >&2
echo "$output" | grep -q "8@9" || wail

echo "checking textification of unadjustable space escape sequence" >&2
echo "$output" | grep -q "9@ 10" || wail

echo "checking textification of unbreakable space escape sequence" >&2
echo "$output" | grep -q "10@ 11" || wail

echo "checking textification of hyphenless break point escape sequence" >&2
echo "$output" | grep -q "11@12" || wail

echo "checking textification of uninterpreted leader escape sequence" >&2
echo "$output" | grep -q "12@13" || wail

echo "checking textification of non-diverted uninterpreted character sequence escape sequence" >&2
echo "$output" | grep -q '!!' || wail

echo "checking textification of diverted uninterpreted character sequence escape sequence" >&2
echo "$output" | grep -q '13@jat14' || wail

echo "checking textification of hyphenation inhibitor escape sequence" >&2
echo "$output" | grep -q '^antimatter15' || wail

echo "checking textification of bracket-building escape sequence" >&2
echo "$output" | grep -q '15@16' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
