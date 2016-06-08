#! /bin/bash

for i in pcc-libs pcc
do
	cd $i
	./configure && make -j && sudo make install
	cd ..
done
