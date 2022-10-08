#!/bin/sh
VKCV_BRANCH="$(git status | awk 'NR == 1 { print $3 }')"

if [ "$VKCV_BRANCH" != "develop" ]; then
	echo "WARNING: Please switch to origin/develop branch for preparing the next release!"
	exit
fi

VKCV_VERSION="$(grep -oEi "#define.+VKCV_FRAMEWORK_VERSION.+\(VK_MAKE_VERSION\(([0-9]+,.*[0-9]+,.*[0-9]+)\)\)" include/vkcv/Core.hpp | awk 'match($0, /.*\(([0-9]+),.*([0-9]+),.*([0-9]+)\)/, a) {print a[1]"."a[2]"."a[3]}')"

if [ $(git tag | grep "$VKCV_VERSION" | wc -l) -gt 0 ]; then
	echo "WARNING: Please adjust the version of the framework before uploading a release! (Duplicate version: $VKCV_VERSION)"
	exit
fi

vim CHANGELOG.md
git add CHANGELOG.md

git commit -S -s -m "==========  VkCV-$VKCV_VERSION  =========="
git push

git checkout master
git merge develop

git tag -a "$VKCV_VERSION" -m "VkCV-$VKCV_VERSION release"
git push
git push --tags
