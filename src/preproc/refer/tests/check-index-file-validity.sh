#!/bin/sh
#
# Copyright 2026 Free Software Foundation, Inc.
#
# This file is part of groff, the GNU roff typesetting system.
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

indxbib="${abs_top_builddir:-.}/indxbib"
lkbib="${abs_top_builddir:-.}/lkbib"

fail=

wail () {
    echo ...FAILED >&2
    fail=YES
}

# Index files are in host-dependent data format.  Skip this test if
# we're not on the most common (and worst) host environment.
if [ "$(uname -m)" != "x86_64" ]
then
    echo "cannot be sure of index file data format; skipping" >&2
    exit 77 # skip
fi

# Locate directory containing our test artifacts.
artifact_dir=

for buildroot in . .. ../..
do
    d=$buildroot/src/preproc/refer/tests/artifacts
    if [ -d "$d" ]
    then
        artifact_dir=$d
        break
    fi
done

# If we can't find it, we can't test.
if [ -z "$artifact_dir" ]
then
    echo "cannot find test artifact directory; skipping" >&2
    exit 77 # skip
fi

common_words_file="$buildroot/src/utils/indxbib/eign"

sandbox_dir=$(mktemp -d)
if [ $? -ne 0 ]
then
    echo "cannot create temporary directory; skipping" >&2
    exit 77 # skip
fi

if ! cp "$artifact_dir/little-schemer.bib" "$sandbox_dir"
then
    echo "cannot populate temporary directory; skipping" >&2
    exit 77 # skip
fi

# Regression-test Savannah #68679.

REFER="$sandbox_dir"/little-schemer.bib
export REFER

if ! "$indxbib" -c "$common_words_file" "$REFER"
then
    echo "cannot generate index for bibliography file; skipping" >&2
    exit 77 # skip
fi

# Corrupt the index file.  Thanks to Pavol Sloboda.
#
# dash's built-in printf doesn't support \x or \u escapes, so likely
# other shells don't either, and expecting one that does to be in the
# $PATH seems optimistic.
printf '\377\377\377\377' | dd of="$REFER".i bs=1 seek=24 conv=notrunc
echo "checking behavior when index file parameters corrupt" >&2
"$lkbib" Schemer 2>&1 > /dev/null | grep -q 'error.*corrupt' || wail

# Regression-test Savannah #68681.

REFER="$sandbox_dir"/string-pool-test.bib
echo '%A Test' > "$REFER"

if ! "$indxbib" -c "$common_words_file" "$REFER"
then
    echo "cannot generate index for bibliography file; skipping" >&2
    exit 77 # skip
fi

# Corrupt the beginning of the string pool.  Thanks to Pavol Sloboda.
printf '\0' | dd of="$REFER".i bs=1 seek=4044 conv=notrunc
echo "checking behavior when index file string pool corrupt" >&2
"$lkbib" Test 2>&1 > /dev/null | grep -q 'error.*corrupt string' || wail

# Regression-test Savannah #68682.

REFER="$sandbox_dir"/file-name-index.bib
echo '%A Test' > "$REFER"

if ! "$indxbib" -c "$common_words_file" "$REFER"
then
    echo "cannot generate index for bibliography file; skipping" >&2
    exit 77 # skip
fi

# Corrupt the bibliography file's index number.  Thanks to Pavol
# Sloboda.
#
# dash's built-in printf doesn't support \x or \u escapes, so likely
# other shells don't either, and expecting one that does to be in the
# $PATH seems optimistic.
printf '\377\377\377\377' | dd of="$REFER".i bs=1 seek=36 conv=notrunc
echo "checking behavior when index file name index number corrupt" >&2
"$lkbib" Test 2>&1 > /dev/null | grep -q 'invalid.*file name index' \
    || wail

# Regression-test Savannah #68685.

REFER="$sandbox_dir"/common-words-index.bib
cp "$artifact_dir/little-schemer.bib" "$REFER"

if ! "$indxbib" -c "$common_words_file" "$REFER"
then
    echo "cannot generate index for bibliography file; skipping" >&2
    exit 77 # skip
fi

# Corrupt the bibliography file's count of common words.  Thanks to
# Pavol Sloboda.
#
# dash's built-in printf doesn't support \x or \u escapes, so likely
# other shells don't either, and expecting one that does to be in the
# $PATH seems optimistic.
printf '\377\377\377?' | dd of="$REFER".i bs=1 seek=32 conv=notrunc
echo "checking behavior when common word list size is bogus" >&2
"$lkbib" Test 2>&1 > /dev/null | grep -q 'implausible count.*common' \
    || wail

# We need `-f` because "little-schemer.bib" gets mode 444 in a "make
# distcheck" build.
rm -rf "$sandbox_dir"
test -z "$fail" || exit 1

# vim:set autoindent expandtab shiftwidth=4 tabstop=4 textwidth=72:
