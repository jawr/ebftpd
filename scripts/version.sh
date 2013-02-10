#!/bin/bash

VERSION_HPP=$1

if [ -z $VERSION_HPP ]; then
  echo "No path to version.hpp specified." 1>&2
  exit 1
fi

VERSION=`git log --decorate --all | grep "^commit " | grep -n "tag: " | \
		sed -r 's|^([0-9]+):.+tag: ([^),]+).+$$|\2-\1|p' | head -n1`

if [ ! -f $VERSION_HPP ] || ! grep -q "\"$VERSION\"" $VERSION_HPP; then \
echo "const char* version = \"$VERSION\";" > $VERSION_HPP; \
fi

