#!/bin/bash

VERSION=`git log --decorate | grep "^commit " | grep -n "tag: " | \
		sed -r 's|^([0-9]+):.+tag: ([^),]+).+$$|\2-\1|p' | head -n1`

if ! grep -q "\"$VERSION\"" src/version.hpp; then \
echo "const char* version = \"$VERSION\";" > src/version.hpp; \
fi
