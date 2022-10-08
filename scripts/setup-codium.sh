#!/bin/bash

CODIUM=codium

if [ ! -f /usr/bin/codium ]; then
	if [ -f /usr/bin/code ]; then
		CODIUM=code
	else
		echo "Please install VSCodium on your system!"
		echo "More information here: https://github.com/VSCodium/vscodium"
		exit
	fi
fi

cat "resources/extensions.txt" | while read EXTENSION || [[ -n $EXTENSION ]];
do
  $CODIUM --install-extension $EXTENSION --force
done

