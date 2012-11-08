#!/bin/bash

DIRNAME=unity
PREFIX=unity


mkdir $DIRNAME 2>1 >/dev/null
SOURCE=`find src/ -type f -name "*.cpp" -printf "%P\n" | grep -v "^$DIRNAME/"`

PARTS=$1
TOTAL=0
for source in $SOURCE; do \
  echo "#include \"$source\"" >> $DIRNAME/$PREFIX.new.cpp
  TOTAL=$(($TOTAL + 1))
done

if [ ! -f $DIRNAME/$PREFIX.cpp ] || \
   [ `ls -l $DIRNAME/$PREFIX-*.cpp | wc -l` -ne $PARTS ] || \
   ! diff $DIRNAME/$PREFIX.cpp $DIRNAME/$PREFIX.new.cpp >/dev/null; then
   
  mv $DIRNAME/$PREFIX.new.cpp $DIRNAME/$PREFIX.cpp
  rm -f $DIRNAME/$PREFIX-*.cpp
  UPDATING=1
else
  rm -f $DIRNAME/$PREFIX.new.cpp
  UPDATING=0
fi

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
  if [ $UPDATING = "1" ]; then
    echo $LINE >> $PARTNAME
  fi
  LINENUM=$(($LINENUM + 1))
done
