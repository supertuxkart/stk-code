#!/bin/sh

src="$(dirname $(readlink -f $0))"

if [ "x$1" = "x-f" ]; then
	dst="$(readlink -mn $2)/"
	args="-f"
else
	dst="$(readlink -mn $1)/"
	args="$2"
fi

echo "Exporting the modules from '$src' to '$dst'"

mkdir -p $dst
(
	cd "$src"
	git checkout-index -a $args "--prefix=$dst"
)

echo "Done!"

