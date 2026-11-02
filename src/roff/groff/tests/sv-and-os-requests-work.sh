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

# Unit-test `sv` and `os` requests.

input='.
.ec @
.nf
1
2
3
4
5
6
7
8
9
.sv 1i/2u
.tm T1=@n[.sv]
13
14
15
16
17
18
19
20
21
22
23
24
25
26
27
28
29
30
31
32
33
34
35
36
37
38
39
40
41
42
43
44
45
46
47
48
49
50
51
52
53
54
55
56
57
58
59
60
.sv 1i
.tm T2=@n[.sv]
A
B
C
D
E
F
.os
.tm T3=@n[.sv]
G
H
I
DONE
.'

output=$(printf '%s\n' "$input" | "$groff" -T ascii | nl -ba)
echo "$output"
output=$(echo $output) # condense onto one line

echo "checking that 'sv' far from a trap works" >&2
echo "$output" | grep -q "9 9 10 11 12 13 13" || wail

# In GNU troff, the `sv` request does not spring a trap.
echo "checking that 'sv' close to a trap works" >&2
echo "$output" \
    | grep -q \
    "60 60 61 A 62 B 63 C 64 D 65 E 66 F 67 68 69 70 71 72 73 G" || wail

echo "checking that 'os' works" >&2
echo "$output" | grep -q "72 73 G 74 H 75 I 76 DONE 77 78" || wail

error=$(printf '%s\n' "$input" | "$groff" -z -T ascii 2>&1)
echo "$error"

echo "checking that 'sv' far from a trap" \
    "does not populate .sv register" >&2
echo "$error" | grep -qx 'T1=0'

echo "checking that 'sv' close to a trap" \
    "populates .sv register" >&2
echo "$error" | grep -qx 'T2=240'

echo "checking that 'os' zeroes the .sv register" >&2
echo "$error" | grep -qx 'T3=0'

test -z "$fail"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
