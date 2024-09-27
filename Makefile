BASE_IMAGE ?=debian:11.10-slim
DATA_DIR ?=$(PWD)/data

all: bullseye ubuntu

run: build
	docker run --rm -it \
		-v $(DATA_DIR):/app/data \
		-v $(PWD)/build:/app/build \
		-v $(PWD)/assets:/app/build/assets \
		--workdir=/app/build \
		--name=gpx2video \
		gpx2video:latest \
		/bin/bash

run-dev:
	docker run --rm -it \
		-v $(PWD):/app \
		--workdir=/app \
		--name=gpx2video \
		gpx2video:latest \
		/bin/bash

bullseye: BASE_IMAGE=debian:bullseye
bullseye: build-docker

ubuntu: BASE_IMAGE=ubuntu:22.04
ubuntu: build-docker

build-docker:
	docker build --build-arg BASE_IMAGE=$(BASE_IMAGE) \
		-t gpx2video:latest \
		-f docker/Dockerfile .

build:
	mkdir -p build
	docker run --rm -it -v $(PWD):/app -w /app gpx2video:latest /bin/bash -c "cd build \
		&& cmake .. \
		&& $(MAKE) -j"

clean:
	rm -rf ./build
