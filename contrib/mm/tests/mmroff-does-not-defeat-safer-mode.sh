#!/bin/sh
#
# Copyright 2026 G. Branden Robinson
#
# This file is part of mm, a reimplementation of the Documenter's
# Workbench (DWB) troff memorandum macro package for use with GNU troff.
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

mmroff="${abs_top_builddir:-.}/mmroff"

# Regression-test Savannah #68689.
#
# The mmroff script should not permit defeat of troff's "safer mode".

sandbox_dir=$(mktemp -d)
if [ $? -ne 0 ]
then
    echo "cannot create temporary directory; skipping" >&2
    exit 77 # skip
fi

# This method is based on a reproducer by Pavol Sloboda.

oops_file="$sandbox_dir"/oops

input='.
.tm .\\" PIC llx '"'"';qx(id>'"$oops_file"');#
.'

printf "%s\n" "$input" | "$mmroff"

if ! test -f "$oops_file"
then
    rmdir "$sandbox_dir"
else
    echo "test failed; file left in \"$oops_file\"" >&2
    exit 1
fi

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
