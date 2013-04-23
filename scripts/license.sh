#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
sed -r 's,^(.*)$,//\1,g' $DIR/../LICENSE > /tmp/LICENSE.$$

for file in $(find $DIR/.. -name "*.[ch]" -o -name "*.[ch]pp")
do
	if ! grep -qi "Copyright" $file
	then
		echo "Adding license to $file ..";
		cat /tmp/LICENSE.$$ > $file.$$
		echo "" >> $file.$$
		cat $file >> $file.$$
		mv $file.$$ $file
	fi
done

rm -f /tmp/LICENSE.$$
