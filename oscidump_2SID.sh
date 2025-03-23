#!/bin/bash

# where everything will be recorded to
DUMPS="/home/$USER/Videos/corr/dumps/C64/"

if [[ $# != 0 ]]; then
	cd $DUMPS # go to the proper directory first
	
	# SID #1
	c64play $@ -m2 -m3 -m4 -m5 -m6 -w01\ -\ Voice\ 1.wav && # then dump Voice 1
	c64play $@ -m1 -m3 -m4 -m5 -m6 -w02\ -\ Voice\ 2.wav && # Voice 2
	c64play $@ -m1 -m2 -m4 -m5 -m6 -w03\ -\ Voice\ 3.wav && # and 3
	# SID #2
	c64play $@ -m1 -m2 -m3 -m5 -m6 -w11\ -\ Voice\ 1.wav &&
	c64play $@ -m1 -m2 -m3 -m4 -m6 -w12\ -\ Voice\ 2.wav &&
	c64play $@ -m1 -m2 -m3 -m4 -m5 -w13\ -\ Voice\ 3.wav &&

	# exclude '-ma' and '-mb' from the arguments so that
	# the master audio and sample channel get
	# properly dumped
	for PARAMS; do
		if [[ $PARAMS = "-ma" || $PARAMS = "-mb" ]]; then
			for PARAMS; do
				[[ ! $PARAMS = "-ma" ]] && NEW_ARGS+=("$PARAMS")
			done
			set -- "${NEW_ARGS[@]}"
			c64play $@ -m1 -m2 -m3 -m4 -m5 -m6 -mb -w04\ -\ Master\ Volume.wav
			c64play $@ -m1 -m2 -m3 -m4 -m5 -m6 -ma -w14\ -\ Master\ Volume.wav
		fi
	done

	c64play $@ -w00\ -\ Master.wav &&

	echo "Done."
else
	echo "Usage: $0 <tune> [c64playargs]"
fi
