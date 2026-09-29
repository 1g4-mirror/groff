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

# Regression-test Savannah #68680.
#
# hpftodit should manage memory responsibly.

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

printf 'II\0\0\0\0\0\0\001\0\220\001\003\0\001\0\0\0\002\0\0\0' \
    > "$tfm_file"
printf '1\t0041\tu0041\n' > "$map_file"

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
