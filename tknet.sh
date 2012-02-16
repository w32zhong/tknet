#!/bin/bash
function get_window_id() 
{
    window_id=$(wmctrl -l | grep "$1" | tail -1 | cut -f1 -d" ")
}

if [ ! $1 ]
then
	scons
	cp run ./testbin/bin0
	cp run ./testbin/bin1
	cp run ./testbin/bin2
	cp run ./testbin/bin3
 	mv run ./bin/
	ctags -R
elif [ $1 == 'c' ]
then
	scons
	ctags -R
elif [ $1 == 'arm' ]
then
	libfiles=' -lpthread'
	codefiles=`ls | grep '\.c$'`
	arm-linux-gcc $codefiles $libfiles
elif [ $1 == 'g++' ]
then
	libfiles=' -lssl -lcrypto -ldl -lpthread'
	codefiles=`ls | grep '\.c$'`
	g++ $codefiles $libfiles && echo ----run---- && ./a.out
	ctags -R
elif [ $1 == 'clean' ]
then
	scons -c
	rm -f a.out
	rm -f tags
elif [ $1 == 'bkup' ]
then
	date --rfc-3339=seconds | grep -o '.*+' > name.tmp
	sed -i -e 's/^/tknet /' name.tmp
	sed -i -e 's/:/-/g' name.tmp
	zipfilename=`sed -e 's/.$/m/' name.tmp `
	rm name.tmp
	zip "$zipfilename" ./* ./demos/*
	mkdir -p backups
	mv *.zip backups/
elif [ $1 == 'close' ]
then	
	for window in $(cat ~/windows.tmp)
	do
		wmctrl -i -c $window
		echo "close $window ..."
	done
elif [ $1 == 'open' ]
then	
	dir="/home/think/Desktop/lib/"
	rm -f ~/windows.tmp

	width1=$(wmctrl -d | grep -o WA.* | grep -o '[1-9]*x' | grep -o '[1-9]*')
	let "width2=$width1*2"
	let "width3=$width1*3"

	sleep 5
	
	wmctrl -o 0,0
	gnome-terminal --working-directory="$dir/testbin/bin0" --command="bash -c 'ls;exec bash'" &
	sleep 1.5
	get_window_id "think-laptop" ; echo "$window_id" >> ~/windows.tmp
	wmctrl -i -r "$window_id" -b remove,maximized_horz,maximized_vert
	wmctrl -i -r "$window_id" -e 0,0,0,580,300

	gnome-terminal --working-directory="$dir/testbin/bin1" --command="bash -c 'ls;exec bash'" &
	sleep 1.5
	get_window_id "think-laptop" ; echo "$window_id" >> ~/windows.tmp
	wmctrl -i -r "$window_id" -e 0,590,0,580,300

	gnome-terminal --working-directory="$dir/testbin/bin2" --command="bash -c 'ls;exec bash'" &
	sleep 1.5
	get_window_id "think-laptop" ; echo "$window_id" >> ~/windows.tmp
	wmctrl -i -r "$window_id" -e 0,590,320,580,300

	gnome-terminal --working-directory="$dir/testbin/bin3" --command="bash -c 'ls;exec bash'" &
	sleep 1.5
	get_window_id "think-laptop" ; echo "$window_id" >> ~/windows.tmp
	wmctrl -i -r "$window_id" -e 0,0,320,580,300

	wmctrl -o $width1,0
	sleep 2.5
	gnome-terminal --working-directory="$dir" --command="bash -c 'ls;./tknet.sh clean;exec bash'" &
	sleep 1.5
	get_window_id "think-laptop" ; echo "$window_id" >> ~/windows.tmp
	wmctrl -i -r "$window_id" -b add,maximized_horz,maximized_vert
	sleep 1.5

	wmctrl -o $width2,0
	sleep 1.5
	gnome-terminal --working-directory="$dir" --command="bash -c 'vi main.c;exec bash'" &
	sleep 1.5
	get_window_id "think-laptop"
	wmctrl -i -r "$window_id" -b add,maximized_horz,maximized_vert
	sleep 1.5

elif [ $1 == 'win' ]
then
	mkdir -p Dos\ format
	codefiles=`ls | grep '\.[ch]$'`
	for file in $codefiles
	do
		sed -e 's/$/\r/' $file >> Dos\ format/$file
	done
	rm -r -f ../Shared\ Folder/Dos\ format
	mv Dos\ format ../Shared\ Folder/
elif [ $1 == 'GPL' ]
then
	echo '
/*
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
		cat GPL.head $srcfile > ${srcfile}.temp
		mv ${srcfile}.temp $srcfile
	done
	rm GPL.head
else
	echo 'input unexpected.'
fi
