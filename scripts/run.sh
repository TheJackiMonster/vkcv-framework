#!/bin/sh
# Navigate to the scripts directory
cd "$(dirname "$0")" || exit

# Check if there is a project name as argument
if [ $# -lt 1 ]; then
	echo "You need to specify a project name to run!">&2
	exit
fi

RUN_WITH_HUD="no"
BUILD_FLAGS=""
while [ $# -gt 1 ]; do
  case "$1" in
    "--"*)
      if [ "$1" = "--hud" ]; then
        RUN_WITH_HUD="yes"
      else
        BUILD_FLAGS="$BUILD_FLAGS$1 "
      fi
      shift 1;;
    *) break;;
  esac
done

PROJECT="$1"
PROJECT_DIR="../projects/$PROJECT"
shift 1

# Check if the project name is valid
if [ ! -d "$PROJECT_DIR" ]; then
	echo "There is no project with the name '$PROJECT'!">&2
	exit
fi

./build.sh $FLAGS "$PROJECT"
cd "$PROJECT_DIR" || exit

if [ ! -f "$PROJECT" ]; then
	echo "Building the project '$PROJECT' failed!">&2
	exit
fi

if [ "$RUN_WITH_HUD" = "yes" ]; then
  MANGOHUD=1 ./"$PROJECT" "$@"
else
  ./"$PROJECT" "$@"
fi
