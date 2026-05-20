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
.de PAGE-LOCATION-TRAP
.tm SPROING
..
.de DIVERSION-TRAP
.tm BOING
..
.wh 10v PAGE-LOCATION-TRAP
.pwh
.di DIVERSION
.tm inside diversion; reporting traps
.pwh
.tm setting diversion trap
.dt 2v DIVERSION-TRAP
.pwh
.nf
foo
bar
.tm diversion trap should have sprung; reporting traps
.pwh
.tm clearing diversion trap and reporting again
.dt
.pwh
.di
.tm outside diversion; reporting traps
.pwh
.'

# Expected error stream output:
#      1  PAGE-LOCATION-TRAP      120000
#      2  inside diversion; reporting traps
#      3  setting diversion trap
#      4  DIVERSION-TRAP  24000
#      5  BOING
#      6  diversion trap should have sprung; reporting traps
#      7  DIVERSION-TRAP  24000
#      8  clearing diversion trap and reporting again
#      9  outside diversion; reporting traps
#     10  PAGE-LOCATION-TRAP      120000

error=$(printf '%s\n' "$input" | "$groff" 2>&1 >/dev/null | nl -ba \
    | tr '\t' ' ')
echo "$error"

echo "checking initial report of page location traps" >&2
echo "$error" | grep -qx " *1 *PAGE-LOCATION-TRAP *[0-9]*" || wail

echo "checking initial report of diversion trap" >&2
echo "$error" | grep -qx " *4 *DIVERSION-TRAP *[0-9]*" || wail

echo "checking subsequent report of diversion trap" >&2
echo "$error" | grep -qx " *7 *DIVERSION-TRAP *[0-9]*" || wail

echo "checking subsequent report of page location traps" >&2
echo "$error" | grep -qx " *10 *PAGE-LOCATION-TRAP *[0-9]*" || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
