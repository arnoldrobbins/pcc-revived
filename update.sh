#! /bin/sh

for i in pcc pcc-libs
do
	rm -fr $i
	cvs -d :pserver:anonymous@pcc.ludd.ltu.se:/cvsroot co $i
done

git status
