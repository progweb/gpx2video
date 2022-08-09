FROM debian:stable

RUN apt-get update
RUN apt-get install -y cmake g++ libevent-dev libssl-dev libcurl4-gnutls-dev libavutil-dev libavformat-dev libavcodec-dev libavfilter-dev libswresample-dev libswscale-dev libopenimageio-dev libgeographic-dev libcairo2-dev libopenexr-dev

RUN mkdir /gpx2video
WORKDIR /gpx2vide

COPY CMakeLists.txt /gpx2video/
COPY LICENSE /gpx2video/
COPY README.md /gpx2video/
COPY assets /gpx2video/assets
COPY cmake /gpx2video/cmake
COPY gpxlib /gpx2video/gpxlib
COPY layoutlib /gpx2video/layoutlib
COPY samples /gpx2video/samples
COPY src /gpx2video/src
COPY tests /gpx2video/tests
COPY utmconvert /gpx2video/utmconvert

RUN mkdir build
WORKDIR /gpx2video/build
RUN cmake ..
RUN make
RUN ln -s ../assets assets

ENTRYPOINT ["/gpx2video/build/gpx2video"]
