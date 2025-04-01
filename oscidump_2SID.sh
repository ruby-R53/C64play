#!/bin/bash

# where everything will be recorded to
DUMPS="/home/$USER/Videos/corr/dumps/C64/"

if [[ $# != 0 ]]; then
	# SID #1
	c64play $@ -m2 -m3 -m4 -m5 -m6 -w$DUMPS/01\ -\ Voice\ 1.wav && # then dump Voice 1
	c64play $@ -m1 -m3 -m4 -m5 -m6 -w$DUMPS/02\ -\ Voice\ 2.wav && # Voice 2
	c64play $@ -m1 -m2 -m4 -m5 -m6 -w$DUMPS/03\ -\ Voice\ 3.wav && # and 3
	# SID #2
	c64play $@ -m1 -m2 -m3 -m5 -m6 -w$DUMPS/11\ -\ Voice\ 1.wav &&
	c64play $@ -m1 -m2 -m3 -m4 -m6 -w$DUMPS/12\ -\ Voice\ 2.wav &&
	c64play $@ -m1 -m2 -m3 -m4 -m5 -w$DUMPS/13\ -\ Voice\ 3.wav &&

	# exclude '-ma' and '-mb' from the arguments so that
	# the master audio and sample channel get
	# properly dumped
	for PARAMS; do
		if [[ $PARAMS = "-ma" || $PARAMS = "-mb" ]]; then
			for PARAMS; do
				[[ ! $PARAMS = "-ma" ]] && NEW_ARGS+=("$PARAMS")
			done
			set -- "${NEW_ARGS[@]}"
			c64play $@ -m1 -m2 -m3 -m4 -m5 -m6 -mb -w$DUMPS/04\ -\ Master\ Volume.wav
			c64play $@ -m1 -m2 -m3 -m4 -m5 -m6 -ma -w$DUMPS/14\ -\ Master\ Volume.wav
		fi
	done

	c64play $@ -w$DUMPS/00\ -\ Master.wav &&

	echo "Done."
else
	echo "Usage: $0 <tune> [c64playargs]"
fi
