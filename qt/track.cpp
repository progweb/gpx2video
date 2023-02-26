#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>

#include "track.h"


#define TILESIZE 256



TrackSettings::TrackSettings() {
	width_ = 320;
	height_ = 240;
	zoom_ = 12;
	marker_size_ = 60;
}


TrackSettings::~TrackSettings() {
}


const int& TrackSettings::width(void) const {
	return width_;
}


const int& TrackSettings::height(void) const {
	return height_;
}


void TrackSettings::setSize(const int &width, const int &height) {
	width_ = width;
	height_ = height;
}


const int& TrackSettings::zoom(void) const {
	return zoom_;
}


const int& TrackSettings::markerSize(void) const {
	return marker_size_;
}


void TrackSettings::setMarkerSize(const int &size) {
	marker_size_ = size;
}


void TrackSettings::getBoundingBox(double *lat1, double *lon1, double *lat2, double *lon2) const {
	*lat1 = lat1_;
	*lon1 = lon1_;

	*lat2 = lat2_;
	*lon2 = lon2_;
}


void TrackSettings::setBoundingBox(double lat1, double lon1, double lat2, double lon2) {
	lat1_ = lat1;
	lon1_ = lon1;

	lat2_ = lat2;
	lon2_ = lon2;
}

