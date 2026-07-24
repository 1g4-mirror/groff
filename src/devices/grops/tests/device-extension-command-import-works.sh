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

grops="${abs_top_builddir:-.}/grops"
srcdir="${abs_top_srcdir:-..}"

# Unit-test `ps: import` device extension command.

# Locate directory containing our test artifacts.
artifact_dir=

for buildroot in . .. ../..
do
    d=$buildroot/doc
    if [ -f $d/gnu.eps ]
    then
        artifact_dir=$d
        gnu_eps=$artifact_dir/gnu.eps
        break
    fi
done

# If we can't find it, we can't test.
if [ -z "$artifact_dir" ]
then
    echo "$0: cannot locate test artifact directory; skipping" >&2
    exit 77 # skip
fi

input="#
x T ps
x res 72000 1 1
x init
p 1
D F d
V 84000
H 72000
x font 5 TR
f 5
s 10000
m d
V 384000
H 72000
x X ps: import $gnu_eps 203 311 408 481 144000 119415
x X ps: import $gnu_eps a 203b 311c 408d 481e 144000f 119415g
x X ps: import $gnu_eps 203a
x trailer
V 792000
x stop
#"

echo "checking that 'ps: import' device extension command works" >&2
output=$(printf '%s\n' "$input" \
    | "$grops" -F font -F "$srcdir"/font)
echo "$output"
echo "$output" | grep -qx "colorimage"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
