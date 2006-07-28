#! /bin/bash

if [ $# -lt 1 ] ; then
  echo "Need case letter (eg: A) or ALL for all cases (CLEAN to delete)"
  exit
else
  CASE=$1
  shift
fi

cd /home/kierans/libcola3/libcola/tests
 
if [ $CASE == 'a' -o $CASE == 'A' ] ; then
  inkview mas_case_a.svg
fi

if [ $CASE == 'b' -o $CASE == 'B' ] ; then
  inkview mas_case_b.svg
fi

if [ $CASE == 'c' -o $CASE == 'C' ] ; then
  inkview mas_case_c.svg
fi

if [ $CASE == 'd' -o $CASE == 'D' ] ; then
  inkview mas_case_d.svg
fi

if [ $CASE == 'e' -o $CASE == 'E' ] ; then
  inkview mas_case_e.svg
fi

if [ $CASE == 'f' -o $CASE == 'F' ] ; then
  inkview mas_case_f.svg
fi

if [ $CASE == 'all' -o $CASE == 'ALL' ] ; then
  inkview mas_case_a.svg mas_case_b.svg mas_case_c.svg mas_case_d.svg mas_case_e.svg mas_case_f.svg
fi

if [ $CASE == 'clean' ] ; then
  rm mas_case* -fv
fi
