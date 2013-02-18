#!/bin/bash
find .. \( -name "*.c" -o -name "*.h" \) -print0 | while read -d $'\0' file
do
	head -1 "$file" | grep "^/\*" 1>/dev/null && 
	echo "Insert Lisence to $file ..." &&
	sed -i -f license.sed "$file"
done
exit 0
