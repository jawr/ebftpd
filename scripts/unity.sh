#!/bin/bash

SRCDIR=$1
UNITYDIR=$2
PREFIX=$3
PARTS=$4

if [ -z "$PARTS" ]; then
	echo "Usage: $0 <srcdir> <unitydir> <prefix> <parts>" > 1&2
	exit 1
fi

mkdir $UNITYDIR &>/dev/null
SOURCE=`find $SRCDIR/ -type f -name "*.cpp" -printf "%P\n" | grep -v "^$UNITYDIR/"`

TOTAL=0
for source in $SOURCE; do \
  echo "#include \"$source\"" >> $UNITYDIR/$PREFIX.new.cpp
  TOTAL=$(($TOTAL + 1))
done

if [ ! -f $UNITYDIR/$PREFIX.cpp ] || \
   [ `ls -l $UNITYDIR/$PREFIX-*.cpp 2>/dev/null | wc -l` -ne $PARTS ] || \
   ! diff $UNITYDIR/$PREFIX.cpp $UNITYDIR/$PREFIX.new.cpp >/dev/null; then
   
  mv $UNITYDIR/$PREFIX.new.cpp $UNITYDIR/$PREFIX.cpp
  rm -f $UNITYDIR/$PREFIX-*.cpp
  UPDATING=1
else
  rm -f $UNITYDIR/$PREFIX.new.cpp
  UPDATING=0
fi

TOTAL=$(($TOTAL + $PARTS - 1))
PERPART=$(($TOTAL / $PARTS))
PART=0
LINENUM=0
cat $UNITYDIR/$PREFIX.cpp | while read LINE; do \
  if [ $(($LINENUM % $PERPART)) -eq 0 ]; then \
    PART=$(($PART + 1))
    PARTNAME=$UNITYDIR/$PREFIX-$PART.cpp
  fi
  if [ $UPDATING = "1" ]; then
    echo $LINE >> $PARTNAME
  fi
  LINENUM=$(($LINENUM + 1))
done
