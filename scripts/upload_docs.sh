#!/bin/sh
doxygen Doxyfile
rsync -r doc/html/ vkcv@penguin2.uni-koblenz.de:/home/vkcv/public_html/doc/
