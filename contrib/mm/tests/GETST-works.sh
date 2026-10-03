#!/bin/sh
#
# Copyright 2026 Free Software Foundation, Inc.
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
fontdir="${abs_top_builddir:-.}/font"
src="${abs_top_srcdir:-..}"

# Locate directory containing our test artifacts.
artifacts_dir=

for buildroot in . .. ../..
do
    d=$buildroot/contrib/mm/tests/artifacts
    if [ -d "$d" ]
    then
        artifacts_dir=$d
        break
    fi
done

# If we can't find it, we can't test.
if [ -z "$artifacts_dir" ]
then
    echo "cannot find test artifact directory; skipping" >&2
    exit 77 # skip
fi

sandbox_dir=$(mktemp -d)
if [ $? -ne 0 ]
then
    echo "cannot create temporary directory; skipping" >&2
    exit 77 # skip
fi

# Regression-test Savannah #68727.

mmdir=$src/contrib/mm/

GROFF_BIN_PATH=. GROFF_FONT_PATH="$fontdir":"$src"/font \
    GROFF_TMAC_PATH="$src"/contrib/mm:"$src"/tmac \
    "$mmroff" -t -T utf8 -P -cbou "$artifacts_dir"/use-reference.mm \
    | grep -q 'Use Table 1 to'
status=$?

# Nuke the reference file to keep `make distclean` happy.  Honoring a
# file name argument to `INITR` with slashes in it in a way that we can
# test looks like a bit of a lift.  :-\
rm -f reference.qrf
test $status -eq 0 && rm -rf "$sandbox_dir"
exit $status

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
