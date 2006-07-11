#! /bin/bash

if [ $# -lt 1 ] ; then
  echo "Need case letter (eg: A) or ALL for all cases"
  exit
else
  CASE=$1
  shift
fi

cd /home/kierans/libcola3/libcola/tests
 
if [ $CASE == 'a' -o $CASE == 'all' ] ; then
  inkview cycle_detector_case_a.svg
fi

if [ $CASE == 'b' -o $CASE == 'all' ] ; then
  inkview cycle_detector_case_b.svg
fi

if [ $CASE == 'c' -o $CASE == 'all' ] ; then
  inkview cycle_detector_case_c.svg
fi

if [ $CASE == 'd' -o $CASE == 'all' ] ; then
  inkview cycle_detector_case_d.svg
fi

if [ $CASE == 'e' -o $CASE == 'all' ] ; then
  inkview cycle_detector_case_e.svg
fi

if [ $CASE == 'f' -o $CASE == 'all' ] ; then
  inkview cycle_detector_case_f.svg
fi
