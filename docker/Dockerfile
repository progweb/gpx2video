ARG BASE_IMAGE=debian:11.10-slim
FROM ${BASE_IMAGE}

RUN apt-get update \
    && apt-get --yes --force-yes dist-upgrade \
    && apt-get install --yes --force-yes --no-install-recommends \
        vim \
        procps \
        \
        build-essential \
        cmake \
        pkg-config \
        \
        libevent-dev \
        libssl-dev \
        libcurl4-gnutls-dev \
        libavutil-dev \
        libavformat-dev \
        libavcodec-dev \
        libavfilter-dev \
        libswresample-dev \
        libswscale-dev \
        libopenexr-dev \
        libopenimageio-dev \
        libgeographic-dev \
        libcairo2-dev \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get clean all
