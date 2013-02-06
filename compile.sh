#!/bin/bash -x

base_path=$(cd $(dirname $0);pwd)

compile_dir(){
	cd $1
	sources=$(find . -name "*.sp")
	for source in $sources; do
		target=$(basename $source .sp)
		$base_path/sprc -o $target.o $source
		if [ $? -ne 0 ]; then
			exit 1
		fi
		gcc -o $target $target.o $base_path/lib/sysapi.o $base_path/gc/lib/libgc.a
		if [ $? -ne 0 ]; then
			if [ -e $target.o ]; then
				rm $target.o
			fi
			exit 1
		else
			if [ -e $target.o ]; then
				rm $target.o
			fi
		fi
	done
}

compile_file(){
	source=$1
	target=$(basename $source .sp)
	$base_path/sprc -o $target.o $source
	if [ $? -ne 0 ]; then
		exit 1
	fi
	gcc -o $target $target.o $base_path/lib/sysapi.o $base_path/gc/lib/libgc.a
	if [ -e $target.o ]; then
		rm $target.o
	fi
}

############################### main #################################

if [ ! -e $base_path/sprc ]; then
	echo "sprc not exists"
	exit 1
fi

if [ ! -e $base_path/lib/sysapi.o ]; then
	echo "lib/sysapi.o not exists"
	exit 1
fi

if [ $# -eq 0 ]; then
	if [ -d $base_path/example ]; then
		compile_dir $base_path/example
	else
		echo no default example path
	fi
elif [ -d $1 ]; then
	compile_dir $1
elif [ -f $1 ]; then
	compile_file $1
else
	echo "file $1 not exists"
fi
