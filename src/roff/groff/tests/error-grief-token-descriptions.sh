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

# Validate error path and improve code coverage.
#
# Put stress on `token::description()` and its supporting functions,
# like `describe_node()`, which have to manage a lot of string buffers
# to construct diagnostic messages for the user.
#
# Node descriptions are also used by node dumpers.
#
# Memory debugging can provoke program aborts from the code paths these
# inputs exercise.

input='.
.ec @
.nr @a 1
.nr @b
.rr @a
.rr @b @c
fnord
.'

output=$(printf '%s\n' "$input" | "$groff" -a)
echo "$output" | grep -qx "fnord"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
