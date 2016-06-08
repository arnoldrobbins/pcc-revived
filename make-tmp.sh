#! /bin/bash

for i in pcc-libs pcc
do
	cd $i
	./configure --prefix=/tmp/pcc
	make -j && make install
	cd ..
done
