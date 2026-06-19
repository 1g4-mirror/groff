#!/bin/sh
#
# Copyright 2026 Free Software Foundation, Inc.
#
# This file is part of groff, the GNU roff typesetting system.
#
# groff is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# groff is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

refer="${abs_top_builddir:-.}/refer"

# Test basic refer(1) functionality.

# Locate directory containing our test artifacts.
artifact_dir=

for buildroot in . .. ../..
do
    d=$buildroot/src/preproc/refer/tests/artifacts
    if [ -d "$d" ]
    then
        artifact_dir=$d
        break
    fi
done

# If we can't find it, we can't test.
test -z "$artifact_dir" && exit 77 # skip

input='.
.LP
Read the book
.[
friedman
.]
on your summer vacation.
Commentary is available.\*{*\*}
.FS \*{*\*}
Space reserved for penetrating insight.
.FE
.'

output=$(printf '%s\n' "$input" \
    | REFER="$artifact_dir"/little-schemer.bib "$refer")

echo "$output"
echo "$output" | grep -Fqx 'Read the book\*([.1\*(.]'

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
