#!/bin/sh
#
# Copyright 2026 G. Branden Robinson
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

groff="${abs_top_builddir:-.}/test-groff"

fail=

wail () {
   echo "...FAILED"
   fail=yes
}

# Regression-test Savannah #68430.

input='.
.de aa
A
..
.if 1 \{.aa\}
B
.'

echo "checking argumentless case" >&2
output=$(printf '%s\n' "$input" | "$groff" -T ascii 2>/dev/null)
echo "$output"
echo "$output" | grep -Fqx "A B" || wail

input='.
.de aa
A
..
.de bb
.tm bb: I have \\n(.$ args
B \\$1 \\$2
..
.if 1 \{.aa
.bb\} Q\}Z
C
.'

echo "checking argumentful case" >&2
output=$(printf '%s\n' "$input" | "$groff" -T ascii 2>/dev/null)
echo "$output"
echo "$output" | grep -Fqx "A B QZ C" || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
