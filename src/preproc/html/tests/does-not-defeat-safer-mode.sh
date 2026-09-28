#!/bin/sh
#
# Copyright 2026 G. Branden Robinson
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

groff="${abs_top_builddir:-.}/test-groff"

# Regression-test Savannah #68688.
#
# pre-grohtml should not permit defeat of troff's "safer mode".

sandbox_dir=$(mktemp -d)
if [ $? -ne 0 ]
then
    echo "cannot create temporary directory; skipping" >&2
    exit 77 # skip
fi

# This method is based on a reproducer by Pavol Sloboda.

oops_file="$sandbox_dir"/oops

# Use of pic(1) does not trip the problem.  Using either tbl(1) or
# eqn(1) does, so exercise both.
input='.
text line 1
.TS
L.
table cell
.TE
.sp
.EQ
x = 1
.EN
\O[5inonexistent.png;id>'"$oops_file"']
\O[4]
.br
text line 2
\O[3]\O[2]
.'

printf "%s\n" "$input" | "$groff" -et -T html > /dev/null

if ! test -f "$oops_file"
then
    rmdir "$sandbox_dir"
    # This removal is nasty, but forced upon us by grohtml's own
    # nastiness in using '<img>' elements and relative paths.  Employing
    # a Base64 data URI would be better...and _much_ more robust.
    #   <img src="data:image/png;base64,ABCDEFGHIJK...">
    rm -f grohtml-*-3.png
else
    echo "test failed; file left in \"$oops_file\"" >&2
    exit 1
fi

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
