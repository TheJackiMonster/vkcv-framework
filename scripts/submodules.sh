#!/bin/sh
if [ "$1" = "init" ]; then
	git submodule init
    git submodule update
elif [ "$1" = "reset" ]; then
    git submodule deinit -f .
	git submodule update --init
elif [ "$1" = "update" ]; then
    git submodule foreach git pull
fi