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
    echo ...FAILED >&2
    fail=YES
}

# Verify that GNU tbl's "nospaces" region option works.

input='.
.TS
allbox nospaces tab(#);
L L L L L L L.
foo # bar#  baz  #qux# ##jat
.TE
.'

output=$(printf "%s\n" "$input" | "$groff" -t -Tascii)
printf '%s\n' "$output"

echo "checking that 'nospaces' region option removes leading space" >&2
printf '%s\n' "$output" | grep -Fq '| foo |' || wail

echo "checking that 'nospaces' region option removes trailing space" >&2
printf '%s\n' "$output" | grep -Fq '| bar |' || wail

echo "checking that 'nospaces' region option removes leading and" \
    "trailing space" >&2
printf '%s\n' "$output" | grep -Fq '| baz |' || wail

echo "checking that 'nospaces' region option can be nilpotent" >&2
printf '%s\n' "$output" | grep -Fq '| qux |' || wail

echo "checking that 'nospaces' region handles space-only entry" >&2
printf '%s\n' "$output" | grep -Fq 'x |   | ' || wail

echo "checking that 'nospaces' region handles empty entry" >&2
printf '%s\n' "$output" | grep -Fq '|   | j' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
