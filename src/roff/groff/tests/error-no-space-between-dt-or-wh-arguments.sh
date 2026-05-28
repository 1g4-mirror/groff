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

# Regression-test Savannah #68357.  We expect NO output.

input='.
.de PT
OUCH
..
.de DT
WHOOPS
.br
..
.wh 9vPT
.pwh
.di DD
.dt 3vDT
.pwh
.br
.di
.'

error=$(printf '%s\n' "$input" | "$groff" 2>&1 >/dev/null | nl -ba \
    | tr '\t' ' ')
echo "$error"

echo "checking that invalid 'wh' request did not plant trap" >&2
echo "$error" | grep -q 'PT' && wail

echo "checking that invalid 'dt' request did not plant trap" >&2
echo "$error" | grep -q 'DT' && wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
