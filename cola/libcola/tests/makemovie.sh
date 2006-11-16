pattern=containment
width=90
height=90
rm ${pattern}*.pdf
for file in ${pattern}*.svg
do
	echo adding $file...
	inkscape -w$width -h$height -A ${file%.svg}.pdf $file 2> /dev/null
done
convert -page ${width}x${height} -delay 5 ${pattern}*.pdf $pattern.gif
