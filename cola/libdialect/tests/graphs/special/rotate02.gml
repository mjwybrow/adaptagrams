Creator	"yFiles"
Version	"2.12"
graph
[
	hierarchic	1
	label	""
	directed	1
	node
	[
		id	0
		label	"1"
		graphics
		[
			x	377.5
			y	200.0
			w	127.0
			h	30.0
			type	"rectangle"
			fill	"#FFCC00"
			outline	"#000000"
		]
		LabelGraphics
		[
			text	"1"
			fontSize	12
			fontName	"Dialog"
			anchor	"c"
		]
	]
	node
	[
		id	1
		label	"2"
		graphics
		[
			x	522.5
			y	200.0
			w	127.0
			h	30.0
			type	"rectangle"
			fill	"#FFCC00"
			outline	"#000000"
		]
		LabelGraphics
		[
			text	"2"
			fontSize	12
			fontName	"Dialog"
			anchor	"c"
		]
	]
	node
	[
		id	2
		label	"3"
		graphics
		[
			x	450.0
			y	325.5
			w	98.0
			h	87.0
			type	"rectangle"
			fill	"#FFCC00"
			outline	"#000000"
		]
		LabelGraphics
		[
			text	"3"
			fontSize	12
			fontName	"Dialog"
			anchor	"c"
		]
	]
	edge
	[
		source	0
		target	2
		graphics
		[
			fill	"#000000"
			targetArrow	"standard"
		]
	]
	edge
	[
		source	1
		target	2
		graphics
		[
			fill	"#000000"
			targetArrow	"standard"
		]
	]
	edge
	[
		source	0
		target	1
		graphics
		[
			fill	"#000000"
			targetArrow	"standard"
		]
	]
]
