#!/bin/bash

DIRNAME=unity
PREFIX=unity


rm -f $DIRNAME/*

SOURCE=`find . -type f -name "*.cpp" -printf "%P\n" | grep -v "^$DIRNAME/"`

PARTS=$1
TOTAL=0
for source in $SOURCE; do \
  echo "#include \"$source\"" >> $DIRNAME/$PREFIX.cpp
  TOTAL=$(($TOTAL + 1))
done
TOTAL=$(($TOTAL + $PARTS - 1))
PERPART=$(($TOTAL / $PARTS))
PART=0
LINENUM=0
cat $DIRNAME/$PREFIX.cpp | while read LINE; do \
  if [ $(($LINENUM % $PERPART)) -eq 0 ]; then \
    PART=$(($PART + 1))
    PARTNAME=$DIRNAME/$PREFIX-$PART.cpp
    echo $PARTNAME
  fi
  echo $LINE >> $PARTNAME
  LINENUM=$(($LINENUM + 1))
done
