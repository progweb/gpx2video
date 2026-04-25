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
	void setZoom(const int &zoom);

	const int& markerSize(void) const;
	void setMarkerSize(const int &size);

	const double& pathThick(void) const;
	void setPathThick(const double &thick);

	const double& pathBorder(void) const;
	void setPathBorder(const double &border);

	const float * pathBorderColor(void) const;
	bool setPathBorderColor(std::string color);
	bool setPathBorderColor(double r, double g, double b, double a);

	const float * pathPrimaryColor(void) const;
	bool setPathPrimaryColor(std::string color);
	bool setPathPrimaryColor(double r, double g, double b, double a);

	const float * pathSecondaryColor(void) const;
	bool setPathSecondaryColor(std::string color);
	bool setPathSecondaryColor(double r, double g, double b, double a);

	void getBoundingBox(double *lat1, double *lon1, double *lat2, double *lon2) const;
	void setBoundingBox(double lat1, double lon1, double lat2, double lon2);

protected:
	int zoom_;

	int marker_size_;

	double path_thick_;
	double path_border_;

	float path_border_color_[4];
	float path_primary_color_[4];
	float path_secondary_color_[4];

private:
	int width_, height_;

	double lat1_, lat2_;
	double lon1_, lon2_;
};

#endif

