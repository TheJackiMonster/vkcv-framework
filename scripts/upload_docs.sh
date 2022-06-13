#!/bin/sh

VKCV_VERSION="$(grep -oEi "#define.+VKCV_FRAMEWORK_VERSION.+\(VK_MAKE_VERSION\(([0-9]+,.*[0-9]+,.*[0-9]+)\)\)" include/vkcv/Core.hpp | awk 'match($0, /.*\(([0-9]+),.*([0-9]+),.*([0-9]+)\)/, a) {print a[1]"."a[2]"."a[3]}')"

DOXYFILE_TMP=$(mktemp)
sed -r "s/(PROJECT_NUMBER.*=.*)([0-9]+\.[0-9]+\.[0-9]+)/\1$VKCV_VERSION/" Doxyfile > $DOXYFILE_TMP
mv $DOXYFILE_TMP Doxyfile

doxygen Doxyfile
rsync -r doc/html/ vkcv@penguin2.uni-koblenz.de:/home/vkcv/public_html/doc/
