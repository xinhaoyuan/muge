#!/bin/sh

# A example script that extract the sprite image from RPG Maker resource format
# Be care of the license in the source images.

# You should install the imagemagick utils to run the script

id=$1
shift

while [ $# -gt 0 ]; do 
	source=$1
	width=`identify -format %w $1`
	height=`identify -format %h $1`

	w_max=$(( $width  / 96 - 1))
	h_max=$(( $height / 128 - 1 ))
	
	for j in `seq 0 $h_max`; do
		for i in `seq 0 $w_max`; do
			echo Get $id
			convert \
				\( $source -crop 96x128+$(( $i * 96 ))+$(( $j * 128 )) \)  \
				\( $source -crop 32x128+$(( $i * 96 + 32 ))+$(( $j * 128 )) \)  \
				+append $id.png
			id=$(( $id + 1 ))	
		done
	done
	shift
done

