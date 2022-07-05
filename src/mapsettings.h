#ifndef __GPX2VIDEO__MAPSETTINGS_H__
#define __GPX2VIDEO__MAPSETTINGS_H__

#include <iostream>
#include <string>


class MapSettings {
public:
	enum Source {
		SourceNull,

		SourceOpenStreetMap,
		SourceOpenStreetMapRenderer,
		SourceOpenAerialMap,
		SourceMapsForFree,
		SourceOpenCycleMap,
		SourceOpenTopoMap,
		SourceOSMPublicTransport,
		SourceGoogleStreet,
		SourceGoogleSatellite,
		SourceGoogleHybrid,
		SourceVirtualEarthStreet,
		SourceVirtualEarthSatellite,
		SourceVirtualEarthHybrid,
		SourceOSMCTrails,
		SourceIGNEssentielMap,
		SourceIGNEssentielPhoto,

		SourceCount
	};

	MapSettings();
	virtual ~MapSettings();

	const int& width(void) const;
	const int& height(void) const;
	void setSize(const int &width, const int &height);

	const Source& source(void) const;
	void setSource(const Source &source);

	const int& zoom(void) const;
	void setZoom(const int &zoom);

	const double& divider(void) const;
	void setDivider(const double &divier);

	void getBoundingBox(double *lat1, double *lon1, double *lat2, double *lon2) const;
	void setBoundingBox(double lat1, double lon1, double lat2, double lon2);

	static const std::string getFriendlyName(const Source &source);
	static const std::string getCopyright(const Source &source);
	static int getMinZoom(const Source &source);
	static int getMaxZoom(const Source &source);
	static const std::string getRepoURI(const Source &source);

private:
	int width_, height_;

	int zoom_;
	double divider_;

	enum Source source_;

	double lat1_, lat2_;
	double lon1_, lon2_;
};

#endif

