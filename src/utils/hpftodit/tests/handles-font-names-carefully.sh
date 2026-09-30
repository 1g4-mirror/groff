#!/bin/sh
#
# Copyright 2026 G. Branden Robinson
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

hpftodit="${abs_top_builddir:-.}/hpftodit"

# Regression-test Savannah #68684.
#
# hpftodit should manage memory carefully.

sandbox_dir=$(mktemp -d)
if [ $? -ne 0 ]
then
    echo "cannot create temporary directory; skipping" >&2
    exit 77 # skip
fi

# This method is based on a reproducer by Pavol Sloboda.

tfm_file="$sandbox_dir"/test.tfm
map_file="$sandbox_dir"/test.map
out_file="$sandbox_dir"/TR # (not really) a description of Times Roman

for byte in \
   'I'   'I'  '\0'  '\0'  '\0'  '\0'  '\0'  '\0' \
'\017'  '\0''\220''\001''\003'  '\0''\001'  '\0' \
  '\0'  '\0''\002'  '\0'  '\0'  '\0''\223''\001' \
'\003'  '\0''\001'  '\0'  '\0'  '\0''\276'  '\0' \
  '\0'  '\0''\224''\001''\001'  '\0''\016'  '\0' \
  '\0'  '\0''\300'  '\0'  '\0'  '\0''\226''\001' \
'\005'  '\0''\001'  '\0'  '\0'  '\0''\324'  '\0' \
  '\0'  '\0''\230''\001''\005'  '\0''\001'  '\0' \
  '\0'  '\0''\334'  '\0'  '\0'  '\0''\233''\001' \
'\001'  '\0''\001'  '\0'  '\0'  '\0''\200'  '\0' \
  '\0'  '\0''\234''\001''\003'  '\0''\001'  '\0' \
  '\0'  '\0'  '\0'  '\0'  '\0'  '\0''\241''\001' \
'\001'  '\0''\001'  '\0'  '\0'  '\0'   'A'   'B' \
   'C'   'D''\245''\001''\003'  '\0''\001'  '\0' \
  '\0'  '\0'   'd'  '\0'  '\0'  '\0''\261''\001' \
'\003'  '\0''\001'  '\0'  '\0'  '\0''\344'  '\0' \
  '\0'  '\0''\263''\001''\021'  '\0''\001'  '\0' \
  '\0'  '\0''\346'  '\0'  '\0'  '\0''\264''\001' \
'\003'  '\0''\001'  '\0'  '\0'  '\0''\350'  '\0' \
  '\0'  '\0''\265''\001''\021'  '\0''\001'  '\0' \
  '\0'  '\0''\352'  '\0'  '\0'  '\0''\266''\001' \
'\021'  '\0''\001'  '\0'  '\0'  '\0''\354'  '\0' \
  '\0'  '\0''\272''\001''\003'  '\0''\001'  '\0' \
  '\0'  '\0'  '\0'  '\0'  '\0'  '\0'   'A'  '\0' \
'\300'  '\0'  '\0'  '\0''\316'  '\0'  '\0'  '\0' \
'\322'  '\0'  '\0'  '\0'  '\0'  '\0'   '1'   '9' \
   'U'  '\0'   'A'  '\0'   'd'  '\0'  '\0'  '\0' \
   '?''\034'  '\0'  '\0'  '\0'  '\b'  '\0'  '\0' \
'\001'  '\0'  '\0'  '\0''\364''\001'  '\0'  '\0' \
'\364''\001''\274''\002'   '8''\377'
do
    printf "$byte" >> "$tfm_file"
done

> "$map_file" # empty

# We expect a controlled failure, not death by a signal.
if ! "$hpftodit" "$tfm_file" "$map_file" "$out_file" 2>&1 \
    | grep -q 'fatal error'
then
    echo "test failed; files left in \"$sandbox_dir\"" >&2
    exit 1
fi

rm "$tfm_file" "$map_file" "$out_file"
rmdir "$sandbox_dir"

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
