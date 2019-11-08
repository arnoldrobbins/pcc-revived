#! /bin/bash

for i in pcc-libs pcc
do
	cd $i
	./configure
	make -j && make install
	cd ..
done
