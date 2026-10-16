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

# Locate directory containing our test artifacts.
artifacts_dir=

for buildroot in . .. ../..
do
    d=$buildroot/src/roff/groff/tests/artifacts
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

# Keep preconv from being run.
#
# The "unset" in Solaris /usr/xpg4/bin/sh can actually fail.
if ! unset GROFF_ENCODING
then
    echo "$0: unable to clear environment; skipping" >&2
    exit 77 # skip
fi

input='.
.so '"$artifacts_dir"'/sourced-file
.'

output=$(echo "$input" | "$groff" -Tascii 2>&1)
echo "$output"
echo "$output" | grep -qx "incomparably corny"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
