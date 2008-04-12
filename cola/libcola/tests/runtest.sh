#!/bin/bash
r=0
while [ $r == 0 ]
do
	./beautify
	r=$?
	echo result=$r
done
