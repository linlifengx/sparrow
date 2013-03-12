#!/bin/bash -x

base_path=$(cd $(dirname $0);pwd)

compile_dir(){
	cd $1
	sources=$(find . -name "*.sp")
	for source in $sources; do
		subdir=$(dirname $source)
		progam=$(basename $source .sp)
		$base_path/sprc -o $subdir/$progam.out $source
		if [ $? -ne 0 ]; then
			exit 1
		fi
	done
}

compile_file(){
	source=$1
	progam=$(basename $source .sp)
	$base_path/sprc -o $progam.out $source
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
