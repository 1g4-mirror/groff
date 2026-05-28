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

input='.
.de t1
.tm SPROING
..
.de 1i
.tm BOING
..
.wh 9v t1
.do pwh
.ch t1
.wh 10v t2
.do pwh
.'

# Expected error stream output:
#      1 t1 108000
#      2 t2 120000

error=$(printf '%s\n' "$input" | "$groff" 2>&1 >/dev/null | nl -ba \
    | tr '\t' ' ')
echo "$error"

echo "checking page location trap placement with 'wh' request" >&2
echo "$error" | grep -qx " *1 *t1 *[0-9]*" || wail

echo "checking page location trap removal with 'ch' request" >&2
echo "$error" | grep -qx " *2 *t2 *[0-9]*" || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
