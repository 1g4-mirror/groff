#!/bin/sh
#
# Copyright 2022-2025 G. Branden Robinson
#
# This file is part of groff, the GNU roff typesetting system.
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
#

groff="${abs_top_builddir:-.}/test-groff"
tbl="${abs_top_builddir:-.}/tbl"

fail=

wail () {
    echo "...FAILED" >&2
    fail=yes
}

# Regression-test Savannah #62366.
#
# Do not SEGV when a text block begins with a repeating glyph token, and
# do not malformat the output if it ends with one.

input='.
.TS
L.
T{
\Ra
T}
.TE
.TS
L.
T{
foo
\Ra
T}
.TE
.TS
L.
T{
foo
\Ra
bar
T}
.TE
.'

tbl_output=$(printf '%s\n' "$input" | "$tbl")
printf '%s\n' "$tbl_output"

groff_output=$(printf '%s\n' "$input" | "$groff" -t -Tascii -P-cbou)
printf '%s\n' "$groff_output"

echo "checking that tbl doesn't segfault" >&2
printf '%s\n' "$tbl_output" | grep -Fqx ".TE" || wail

echo "checking text block starting with repeating glyph" >&2
printf '%s\n' "$groff_output" | sed -n 1p | grep -qx 'a' || wail

echo "checking text block ending with repeating glyph" >&2
printf '%s\n' "$groff_output" | sed -n 2p | grep -qx 'foo a' || wail

echo "checking text block containing repeating glyph" >&2
printf '%s\n' "$groff_output" | sed -n 3p | grep -qx 'foo a bar' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
