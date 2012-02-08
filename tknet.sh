#!/bin/bash
if [ ! $1 ]
then
	scons && echo ----run---- && ./run
	cp run ./testbin/bin0
	cp run ./testbin/bin1
	cp run ./testbin/bin2
	ctags -R
elif [ $1 == 'c' ]
then
	scons
	ctags -R
elif [ $1 == 'arm' ]
then
	codefiles=`ls | grep '\.c$'`
	arm-linux-gcc $codefiles
elif [ $1 == 'g++' ]
then
	libfiles=' -lssl -lcrypto -ldl -lpthread'
	codefiles=`ls | grep '\.c$'`
	g++ $codefiles $libfiles && echo ----run---- && ./a.out
	ctags -R
elif [ $1 == 'clean' ]
then
	scons -c
	rm a.out
	rm tags
	rm *.log
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
else
	echo 'input unexpected.'
fi
