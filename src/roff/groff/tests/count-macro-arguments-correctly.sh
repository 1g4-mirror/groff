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

# A bug in macro argument counting, such that a right-brace escape
# sequence created an empty macro argument out of nothing, originated in
# Ossanna troff and endured through early years of Kernighan
# (device-independent) troff, possibly through DWB 2.0, whence it
# propagated into Solaris troff and Plan 9 troff.  DWB 3.3 fixed it;
# Carsten Kunze did likelwse for "portable" Solaris 10 troff, and he or
# Gunnar Ritter applied the fix to Heirloom Doctools troff as well.
#
# See Savannah #42675 and Savannah #68430 for more information.

input='.
.de aa
\\n(.$
..
.aa\}
.'

output=$(printf '%s\n' "$input" | "$groff" -T ascii 2>/dev/null)
echo "$output"
echo "$output" | grep -Fqx "0"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
