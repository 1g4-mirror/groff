#!/bin/sh
#
# Copyright 2026 G. Branden Robinson
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
   echo "...FAILED"
   fail=yes
}

# Unit-test `shift` request.

input='.
.ec @
.nf
.de mac
arg 1=@@$1
.shift
arg 2=@@$1
.shift 1
arg 3=@@$1
.shift 2
arg 5=@@$1
..
.mac foo bar baz qux jat
.'

output=$(printf '%s\n' "$input" | "$groff" -a)
echo "$output"

echo "checking that 'shift' with no argument works" >&2
echo "$output" | grep -q "arg 2=bar" || wail

echo "checking that 'shift' with '1' argument works" >&2
echo "$output" | grep -q "arg 3=baz" || wail

echo "checking that 'shift' with '2' argument works" >&2
echo "$output" | grep -q "arg 5=jat" || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
