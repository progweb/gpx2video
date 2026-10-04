BASE_IMAGE?=debian:13-slim
BUILD_DIR=build-debian-trixie

# Your video directory
VIDEO_DIR?=$(PWD)

# GPU passthrough: make GPU=1 run-ubuntu-resolute
GPU?=0
DOCKER_GPU=$(if $(filter 1,$(GPU)),--gpus all,)


# 12: bookworm
config-debian-bookworm:
	$(eval GTK=ON)
	$(eval BASE_IMAGE=debian:bookworm)
	$(eval BUILD_DIR=build-debian-bookworm)

# 13: trixie
config-debian-trixie:
	$(eval GTK=ON)
	$(eval BASE_IMAGE=debian:trixie)
	$(eval BUILD_DIR=build-debian-trixie)

# 24.04: noble
config-ubuntu-noble:
	$(eval GTK=ON)
	$(eval BASE_IMAGE=ubuntu:24.04)
	$(eval BUILD_DIR=build-ubuntu-noble)

# 26.04: resolute
config-ubuntu-resolute:
	$(eval GTK=ON)
	$(eval BASE_IMAGE=ubuntu:26.04)
	$(eval BUILD_DIR=build-ubuntu-resolute)


all: debian ubuntu


run: build-gpx2video run-gpx2video


# Debian
debian: debian-bookworm debian-trixie

debian-bookworm: config-debian-bookworm build-docker build-gpx2video
debian-trixie: config-debian-trixie build-docker build-gpx2video


# Ubuntu
ubuntu: ubuntu-noble ubuntu-resolute

ubuntu-noble: config-ubuntu-noble build-docker build-gpx2video
ubuntu-resolute: config-ubuntu-resolute build-docker build-gpx2video


# Build
build-debian-bookworm: config-debian-bookworm build-gpx2video
build-debian-trixie: config-debian-trixie build-gpx2video
build-ubuntu-noble: config-ubuntu-noble build-gpx2video
build-ubuntu-resolute: config-ubuntu-resolute build-gpx2video


# Exec
run-debian-bookworm: config-debian-bookworm run-gpx2video
run-debian-trixie: config-debian-trixie run-gpx2video
run-ubuntu-noble: config-ubuntu-noble run-gpx2video
run-ubuntu-resolute: config-ubuntu-resolute run-gpx2video


# Dev
dev-debian-bookworm: config-debian-bookworm dev-gpx2video
dev-debian-trixie: config-debian-trixie dev-gpx2video
dev-ubuntu-noble: config-ubuntu-noble dev-gpx2video
dev-ubuntu-resolute: config-ubuntu-resolute dev-gpx2video


build-docker:
	-cp /etc/locale.gen docker
	docker build --pull --build-arg BASE_IMAGE=$(BASE_IMAGE) \
		-t "gpx2video-$(BASE_IMAGE)" \
		-f docker/$(if $(patsubst ON,,$(GTK)),DockerfileWithoutGTK,DockerfileWithGTK) .
	-rm docker/locale.gen


dev-gpx2video:
	mkdir -p $(BUILD_DIR)
	docker run --rm -it \
		$(DOCKER_GPU) \
		-e LANG=$(LANG) \
		-e LANGUAGE=$(LANG) \
		-e LC_ALL=$(LANG) \
		-e XDG_RUNTIME_DIR=/tmp \
		-e WAYLAND_DISPLAY=$(WAYLAND_DISPLAY) \
		-v /etc/timezone:/etc/timezone \
		-v /etc/localtime:/etc/localtime \
		-v /etc/locale.gen:/etc/locale.gen \
		-v /etc/locale.conf:/etc/locale.conf \
		-v $(XDG_RUNTIME_DIR)/$(WAYLAND_DISPLAY):/tmp/$(WAYLAND_DISPLAY)  \
		-u $(shell id -u):$(shell id -g) \
		-v $(PWD)/$(BUILD_DIR):/app/build \
		-v $(PWD):/app \
		-v $(VIDEO_DIR):/data \
		--workdir=/app/build \
		gpx2video-$(BASE_IMAGE) \
		/bin/bash

build-gpx2video:
	mkdir -p $(BUILD_DIR)
	docker run --rm -it \
		-u $(shell id -u):$(shell id -g) \
		-v $(PWD)/$(BUILD_DIR):/app/build \
		-v $(PWD):/app \
		--workdir=/app/build \
		gpx2video-$(BASE_IMAGE) \
		/bin/bash -c \
		"cmake -DBUILD_GTK=$(GTK) .. \
		&& $(MAKE) -j"


run-gpx2video:
	mkdir -p $(BUILD_DIR)
	docker run --rm -it \
		$(DOCKER_GPU) \
		-e LANG=$(LANG) \
		-e LC_ALL=$(LANG) \
		-e XDG_RUNTIME_DIR=/tmp \
		-e WAYLAND_DISPLAY=$(WAYLAND_DISPLAY) \
		-v /etc/timezone:/etc/timezone \
		-v /etc/localtime:/etc/localtime \
		-v /etc/locale.gen:/etc/locale.gen \
		-v $(XDG_RUNTIME_DIR)/$(WAYLAND_DISPLAY):/tmp/$(WAYLAND_DISPLAY)  \
		-u $(shell id -u):$(shell id -g) \
		-v $(PWD)/$(BUILD_DIR):/app/build \
		-v $(PWD)/assets:/app/build/assets \
		-v $(VIDEO_DIR):/data \
		--workdir=/app/build \
		--name=gpx2video \
		gpx2video-$(BASE_IMAGE) \
		/bin/bash


clean:
	rm -rf ./$(BUILD_DIR)

