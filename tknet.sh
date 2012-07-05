#!/bin/bash
function get_window_id() 
{
	window_id=$(wmctrl -l | grep "$1" | tail -1 | cut -f1 -d" ")
}

function set_build_time()
{
	echo "#define TKNET_VER \"$(date --rfc-3339=seconds)\"" > head.h
}

#dependlibs='-lssl -lcrypto -ldl -lpthread'
dependlibs='-lpthread'

if [ ! $1 ]
then
	./tknet.sh build

elif [ $1 == 'buildall' ]
then
	./tknet.sh buildarm
	./tknet.sh build

elif [ $1 == 'buildarm' ]
then
	set_build_time
	codefiles=`ls | grep '\.c$'`
        arm-linux-gcc -c $codefiles
        objfiles=`ls | grep '\.o$'`
        arm-linux-ar -rcs libtknet.a $objfiles

	arm-linux-gcc -c './test/demo.c' -o ./demo.o
	arm-linux-gcc demo.o $dependlibs -L ./ -ltknet -o ./demo

	mkdir -p ./bin/arm
	mv libtknet.a ./bin/arm/
	mv demo ./bin/arm/
	cp ./bin/tknet.info ./bin/arm/

elif [ $1 == 'build' ]
then
	ctags -R *
	scons -c
	set_build_time
	scons
	gcc -c './test/demo.c' -o ./demo.o
	gcc demo.o $dependlibs -L ./ -ltknet -o ./demo

	mkdir -p ./bin/x86
	mv libtknet.a ./bin/x86/
	mv demo ./bin/x86/
	cp ./bin/tknet.info ./bin/x86/

	cp ./bin/x86/demo ./test/bin/dir0
	cp ./bin/x86/demo ./test/bin/dir1
	cp ./bin/x86/demo ./test/bin/dir2
	cp ./bin/x86/demo ./test/bin/dir3

elif [ $1 == 'unit_test' ]
then
	gcc -o ./test/unit.out ./test/unit_test.c -I . -L ./bin/x86 -lpthread -ltknet

elif [ $1 == 'clean' ]
then
        objfiles=`ls | grep '\.o$'`
	rm -f $objfiles
	rm -f tags
	rm -rf ./bin/arm
	rm -rf ./bin/x86
	rm -rf ./dosformat
	
	rm -f ./test/bin/dir0/demo
	rm -f ./test/bin/dir1/demo
	rm -f ./test/bin/dir2/demo
	rm -f ./test/bin/dir3/demo

	find . -name '*.log' | xargs rm -f
	find . -name '*.exp' | xargs rm -f

elif [ $1 == 'history' ]
then
	./tknet.sh clean

	date --rfc-3339=seconds | grep -o '.*+' > name.tmp
	sed -i -e 's/^/tknet /' name.tmp
	sed -i -e 's/:/-/g' name.tmp
	zipfilename=`sed -e 's/.$/m/' name.tmp `
	rm name.tmp
	find . -type d \( -name '.git' -o -name 'history' \) -prune -o -print0 | xargs -0 zip "$zipfilename"
	mv *.zip 'history/'

elif [ $1 == 'test_close' ]
then	
	for window in $(cat ~/windows.tmp)
	do
		wmctrl -i -c $window
		echo "close $window ..."
	done

elif [ $1 == 'test' ]
then	
	dir=$(pwd)
	rm -f ~/windows.tmp

	wmctrl -o 0,0
	gnome-terminal --working-directory="$dir/test/bin/dir0" --command="bash -c 'cat tknet.info;exec bash'" &
	sleep 1.5
	get_window_id "think-laptop" ; echo "$window_id" >> ~/windows.tmp
	wmctrl -i -r "$window_id" -b remove,maximized_horz,maximized_vert
	wmctrl -i -r "$window_id" -e 0,0,0,580,300

	gnome-terminal --working-directory="$dir/test/bin/dir1" --command="bash -c 'cat tknet.info;exec bash'" &
	sleep 1.5
	get_window_id "think-laptop" ; echo "$window_id" >> ~/windows.tmp
	wmctrl -i -r "$window_id" -e 0,590,0,580,300

	gnome-terminal --working-directory="$dir/test/bin/dir2" --command="bash -c 'cat tknet.info;exec bash'" &
	sleep 1.5
	get_window_id "think-laptop" ; echo "$window_id" >> ~/windows.tmp
	wmctrl -i -r "$window_id" -e 0,590,320,580,300

	gnome-terminal --working-directory="$dir/test/bin/dir3" --command="bash -c 'cat tknet.info;exec bash'" &
	sleep 1.5
	get_window_id "think-laptop" ; echo "$window_id" >> ~/windows.tmp
	wmctrl -i -r "$window_id" -e 0,0,320,580,300

elif [ $1 == 'win' ]
then
	mkdir -p dosformat
	codefiles=`ls | grep '\.[ch]$'`
	codefiles+=' test/demo.c'
	for file in $codefiles
	do
		cp $file ./dosformat
	done
	cd ./dosformat
	for file in *
	do
		sed -i -e 's/$/\r/' $file
	done

elif [ $1 == 'GPL' ]
then
	echo '/*
*      This file is part of the tknet project. 
*    which be used under the terms of the GNU General Public 
*    License version 3.0 as published by the Free Software
*    Foundation and appearing in the file LICENSE.GPL included 
*    in the packaging of this file.  Please review the following 
*    information to ensure the GNU General Public License 
*    version 3.0 requirements will be met: 
*    http://www.gnu.org/copyleft/gpl.html
*
*    Copyright  (C)   2012   Zhong Wei <clock126@126.com>  .
*/ 
' > GPL.head
	
	for srcfile in *.c *.h
	do
		rm -rf ./test/bin
		cat GPL.head $srcfile > ${srcfile}.temp
		mv ${srcfile}.temp $srcfile
	done
	rm GPL.head

elif [ $1 == 'lshead' ]
then
	for srcfile in *.c *.h
	do
		echo "$srcfile:"
		cat $srcfile | head -2
	done

elif [ $1 == 'rmhead' ]
then
	for srcfile in *.c *.h
	do
		sed -i '1,9d' $srcfile
	done

else
	echo 'input unexpected.'
fi
