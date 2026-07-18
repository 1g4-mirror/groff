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
    echo "...FAILED" >&2
    fail=YES
}

# Automatically generated numeric footnote marks, obtained with the ms
# string `\*[*]` (or `\**`) should work even in tbl(1) tables.

input='.
.LP
This is my
.I ms
document.
.TS
L.
foo\**
.TE
.FS
Bar.
.FE
.'

output=$(printf '%s\n' "$input" | "$groff" -t -m s -T ascii 2>&1)
echo "$output"

echo "checking number of footnote mark in table" >&2
echo "$output" | grep -Fq 'foo[1]' || wail

echo "checking number of footnote mark at page foot" >&2
echo "$output" | grep -Fq '[1] Bar.' || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
