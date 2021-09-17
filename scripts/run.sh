#!/bin/sh
# Navigate to the scripts directory
cd "$(dirname "$0")" || exit

# Check if there is a project name as argument
if [ $# -lt 1 ]; then
	echo "You need to specify a project name to run!">&2
	exit
fi

PROJECT=$1
PROJECT_DIR="../projects/$PROJECT"

# Check if the project name is valid
if [ ! -d "$PROJECT_DIR" ]; then
	echo "There is no project with the name '$PROJECT'!">&2
	exit
fi

./build.sh $PROJECT
cd "$PROJECT_DIR" || exit

if [ ! -f "$PROJECT" ]; then
	echo "Building the project '$PROJECT' failed!">&2
	exit
fi

./$PROJECT