all: bullseye ubuntu

run:
	docker run --rm -t -i \
		-v `pwd`/../:/data \
		--workdir=/data/ \
		--name=gpx2video \
		gpx2video:latest \
		/bin/bash

bullseye:
	docker pull debian:bullseye
	docker build --pull --no-cache \
		-t gpx2video:debian-latest \
		-f docker/Dockerfile.bullseye .

ubuntu:
	docker pull ubuntu:22.04
	docker build --pull --no-cache \
		-t gpx2video:ubuntu-latest \
		-f docker/Dockerfile.ubuntu .

clean:

