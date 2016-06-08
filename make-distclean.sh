#! /bin/bash

for i in pcc-libs pcc
do
	cd $i
	make distclean
	cd ..
done
