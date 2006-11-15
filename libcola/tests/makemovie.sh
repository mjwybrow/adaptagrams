pattern=containment2
rm ${pattern}*.pdf
for file in ${pattern}*.svg
do
	echo adding $file...
	inkscape -w200 -h200 -A ${file%.svg}.pdf $file 2> /dev/null
done
convert -density 120 -page 200x200 -delay 20 ${pattern}*.pdf $pattern.gif
