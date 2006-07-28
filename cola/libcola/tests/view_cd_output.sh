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
  inkview cycle_detector_case_a.svg
fi

if [ $CASE == 'b' -o $CASE == 'B' ] ; then
  inkview cycle_detector_case_b.svg
fi

if [ $CASE == 'c' -o $CASE == 'C' ] ; then
  inkview cycle_detector_case_c.svg
fi

if [ $CASE == 'd' -o $CASE == 'D' ] ; then
  inkview cycle_detector_case_d.svg
fi

if [ $CASE == 'e' -o $CASE == 'E' ] ; then
  inkview cycle_detector_case_e.svg
fi

if [ $CASE == 'f' -o $CASE == 'F' ] ; then
  inkview cycle_detector_case_f.svg
fi

if [ $CASE == 'all' -o $CASE == 'ALL' ] ; then
  inkview cycle_detector_case_a.svg cycle_detector_case_b.svg cycle_detector_case_c.svg cycle_detector_case_d.svg cycle_detector_case_e.svg cycle_detector_case_f.svg
fi

if [ $CASE == 'clean' ] ; then
  rm cycle_detector_case* -fv
fi
