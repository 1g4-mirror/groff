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
  echo ...FAILED >&2
  fail=YES
}

input='.
.nf
\X#bogus1: esc \%to-do\[u1F63C]\\[u1F00] -\[aq]\[dq]\[ga]\[ha]\[rs]\[ti]\[`a]#
.device bogus1: req \%to-do\[u1F63C]\\[u1F00] -\[aq]\[dq]\[ga]\[ha]\[rs]\[ti]\[`a]
.ec @
@X#bogus2: esc @%to-do@[u1F63C]@@[u1F00] -@[aq]@[dq]@[ga]@[ha]@[rs]@[ti]@[`a]#
.device bogus2: req @%to-do@[u1F63C]@@[u1F00] -@[aq]@[dq]@[ga]@[ha]@[rs]@[ti]@[`a]
.'

output=$(printf '%s\n' "$input" | "$groff" -T ps -Z 2> /dev/null \
  | grep '^x X')
error=$(printf '%s\n' "$input" | "$groff" -T ps -Z 2>&1 > /dev/null)

echo "$output"
echo "$error"

# Expected:
# x X bogus1: esc to-do\[u1F63C]\[u1F00] -'"`^\~\[u00E0]
# x X bogus1: req @%to-do\[u1F63C]\[u1F00] -\[aq]\[dq]\[ga]\[ha]\[rs]\[ti]\[`a]#
# x X bogus2: esc to-do\[u1F63C]\[u1F00] -'"`^\~\[u00E0]
# x X bogus2: req @%to-do@[u1F63C]@[u1F00] -@[aq]@[dq]@[ga]@[ha]@[rs]@[ti]@[`a]#

echo "checking X escape sequence, default escape character" >&2
echo "$output" \
  | grep -Fqx \
  'x X bogus1: esc to-do\[u1F63C]\[u1F00] -'"'"'"`^\~\[u00E0]' \
  || wail

#echo "checking device request, default escape character" >&2
#echo "$output" \
#  | grep -qx 'x X bogus1: req to-do\\\[u1F00\] -'"'"'"`^\\~' \
#  || wail

echo "checking X escape sequence, alternate escape character" >&2
echo "$output" \
  | grep -Fqx \
  'x X bogus1: esc to-do\[u1F63C]\[u1F00] -'"'"'"`^\~\[u00E0]' \
  || wail

#echo "checking device request, alternate escape character" >&2
#echo "$output" \
#  | grep -qx 'x X bogus2: req to-do\\\[u1F00\] -'"'"'"`^\\~' \
#  || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=2 tabstop=2 textwidth=72:
