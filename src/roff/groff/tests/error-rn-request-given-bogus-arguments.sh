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
   echo "...FAILED"
   fail=yes
}

# Regression-test Savannah #68797.

echo "checking whether 'rn' request crashes on invalid first argument"
printf '.rn \002 output-device\n' | "$groff" -T ascii 2>&1 || wail

echo "checking whether 'rn' request crashes on invalid second argument"
printf '.rn .T \002\n' | "$groff" -T ascii 2>&1 || wail

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
