#! /bin/sh

for i in pcc pcc-libs
do
	(cd $i ; cvs update)
done

git status
