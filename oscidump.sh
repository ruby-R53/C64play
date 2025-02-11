#!/bin/bash

# where everything will be recorded to
DUMPS="/home/$USER/Videos/corr/dumps/C64/"

if [[ $# != 0 ]]; then
	cd $DUMPS # go to the proper directory first

	c64play $@ -m2 -m3 -w01\ -\ Voice\ 1.wav && # then dump Voice 1
	c64play $@ -m1 -m3 -w02\ -\ Voice\ 2.wav && # Voice 2
	c64play $@ -m1 -m2 -w03\ -\ Voice\ 3.wav && # and 3

	# exclude '-ma' from the arguments so that
	# the master audio and sample channel get
	# properly dumped
	for PARAMS; do
		if [[ $PARAMS = "-ma" ]]; then
			for PARAMS; do
				[[ ! $PARAMS = "-ma" ]] && NEW_ARGS+=("$PARAMS")
			done
			set -- "${NEW_ARGS[@]}"
			c64play $@ -m1 -m2 -m3 -w04\ -\ Master\ volume.wav
		fi
	done

	c64play $@ -w00\ -\ Master.wav &&

	echo "Done."
else
	echo "Usage: $0 <tune> [c64playargs]"
fi
