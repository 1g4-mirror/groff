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
#

tbl="${abs_top_builddir:-.}/tbl"

# Regression-test Savannah #68570.
#
# Don't segfault if the table description consists solely of horizontal
# span column classifiers that we have to discard because they're
# invalid in the first column.  If the table "description" contains
# nothing but these, we never manage to construct one.
#
# Thanks to Elias Hasas for suggesting a very similar reproducer.

input='.
.TS
SS,S.
foo
.TE
fnord
.'

output=$(printf '%s\n' "$input" | "$tbl")
printf '%s\n' "$output"
printf '%s\n' "$output" | grep -Fqx "fnord"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
