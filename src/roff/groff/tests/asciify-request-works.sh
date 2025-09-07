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
.box DIV
.ec #
.char #[Z] ZYX
A#[dq]#[e aa]#[u00E1]#[u0106]i#[fl]o#[Fl]#[Z]
.ec
.br
.box
.asciify DIV
.DIV
.'

output=$(printf "%s\n" "$input" | "$groff" -T ps -a)
echo "$output"

echo "checking textification of ordinary character 'A'" >&2
echo "$output" | grep -q '^A' || wail

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

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
