BASE_IMAGE?=debian:12.8-slim
BUILD_DIR=build-debian-12

# Your video directory
VIDEO_DIR?=$(PWD)


# 11: bullseye
config-debian-bullseye:
	$(eval BASE_IMAGE=debian:bullseye)
	$(eval BUILD_DIR=build-debian-bullseye)

# 12: bookworm
config-debian-bookworm:
	$(eval BASE_IMAGE=debian:bookworm)
	$(eval BUILD_DIR=build-debian-bookworm)

# 22.04: jammy
config-ubuntu-jammy: 
	$(eval BASE_IMAGE=ubuntu:22.04)
	$(eval BUILD_DIR=build-ubuntu-jammy)

# 24.04: noble
config-ubuntu-noble:
	$(eval BASE_IMAGE=ubuntu:24.04)
	$(eval BUILD_DIR=build-ubuntu-noble)


all: debian ubuntu


run: build-gpx2video run-gpx2video


# Debian
debian: debian-bullseye debian-bookworm 
	
debian-bullseye: config-debian-bullseye build-docker build-gpx2video
debian-bookworm: config-debian-bullseye build-docker build-gpx2video


# Ubuntu
ubuntu: ubuntu-jammy ubuntu-noble

ubuntu-jammy: config-ubuntu-jammy build-docker build-gpx2video
ubuntu-noble: config-ubuntu-noble build-docker build-gpx2video


# Build
build-debian-bullseye: config-debian-bullseye build-gpx2video
build-debian-bookworm: config-debian-bullseye build-gpx2video
build-ubuntu-jammy: config-ubuntu-jammy build-gpx2video
build-ubuntu-noble: config-ubuntu-noble build-gpx2video


# Exec
run-debian-bullseye: config-debian-bullseye run-gpx2video
run-debian-bookworm: config-debian-bullseye run-gpx2video
run-ubuntu-jammy: config-ubuntu-jammy run-gpx2video
run-ubuntu-noble: config-ubuntu-noble run-gpx2video


build-docker:
	docker build --build-arg BASE_IMAGE=$(BASE_IMAGE) \
		-t "gpx2video-$(BASE_IMAGE)" \
		-f docker/Dockerfile .


dev-gpx2video:
	mkdir -p $(BUILD_DIR)
	docker run --rm -it \
		-e XDG_RUNTIME_DIR=/tmp \
		-e WAYLAND_DISPLAY=$(WAYLAND_DISPLAY) \
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
		"cmake -DBUILD_GTK=ON .. \
		&& $(MAKE) -j"


run-gpx2video:
	mkdir -p $(BUILD_DIR)
	docker run --rm -it \
		-e XDG_RUNTIME_DIR=/tmp \
		-e WAYLAND_DISPLAY=$(WAYLAND_DISPLAY) \
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

