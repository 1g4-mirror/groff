#!/bin/sh
#
# Copyright 2025-2026 G. Branden Robinson
#
# This file is part of groff, the GNU roff typesetting system.
#
# groff is free software; you can redistribute it and/or modify it over
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
  echo "...FAILED" >&2
  fail=yes
}

# troff should not perform invalid memory access when using `box` to
# close a regular diversion.  Savannah #67139.

input1='.
.box d1
.di
.box
HAPAX
.'

output1=$(printf '%s\n' "$input1" | "$groff" -a)
echo "$output1"

echo "checking that closing box diversion with 'di' is not fatal" >&2
echo "$output1" | grep -q 'HAPAX' || wail

input2='.
.br
.di d2
.box
LEGOMENON
.'

output=$(printf '%s\n' "$input2" | "$groff" -a)
echo "$output2"

echo "checking that closing non-box diversion with 'box' is fatal" >&2
! echo "$output2" | grep -q 'LEGOMENON' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
