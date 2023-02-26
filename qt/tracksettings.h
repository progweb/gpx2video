#ifndef __GPX2VIDEO__TRACKSETTINGS_H__
#define __GPX2VIDEO__TRACKSETTINGS_H__

#include <iostream>
#include <string>


class TrackSettings {
public:
	TrackSettings();
	virtual ~TrackSettings();

	const int& width(void) const;
	const int& height(void) const;
	void setSize(const int &width, const int &height);

	const int& zoom(void) const;

	const int& markerSize(void) const;
	void setMarkerSize(const int &size);

	void getBoundingBox(double *lat1, double *lon1, double *lat2, double *lon2) const;
	void setBoundingBox(double lat1, double lon1, double lat2, double lon2);

protected:
	int zoom_;

	int marker_size_;

private:
	int width_, height_;

	double lat1_, lat2_;
	double lon1_, lon2_;
};

#endif

