#!/bin/sh
#
# Copyright (C) 2024 Free Software Foundation, Inc.
#
# This file is part of groff.
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
#

groff="${abs_top_builddir:-.}/test-groff"

fail=

wail () {
  echo "...FAILED" >&2
  fail=yes
}

# Regression-test Savannah #66255.

tmpfile=savannah-66255.txt

cleanup () {
  rm -f $tmpfile
}

trap 'trap - HUP INT QUIT TERM; cleanup; kill -INT $$' HUP INT QUIT TERM

input='.
.open mystream '$tmpfile'
.write mystream a
.write mystream b
.write mystream
.write mystream c
.close mystream
.'

output=$(echo "$input" | "$groff" -U)
od -c $tmpfile

echo "verifying presence of desired text in output file" >&2
grep -qx 'c' $tmpfile || wail

echo "verifying absence of garbage text in output file" >&2
grep -q 'write' $tmpfile && wail

cleanup
test -z "$fail"

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
