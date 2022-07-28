#ifndef __GPX2VIDEO__MAPSETTINGS_H__
#define __GPX2VIDEO__MAPSETTINGS_H__

#include <iostream>
#include <string>

#include "tracksettings.h"


class MapSettings : public TrackSettings {
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

	const Source& source(void) const;
	void setSource(const Source &source);

	void setZoom(const int &zoom);

	const double& divider(void) const;
	void setDivider(const double &divier);

	static const std::string getFriendlyName(const Source &source);
	static const std::string getCopyright(const Source &source);
	static int getMinZoom(const Source &source);
	static int getMaxZoom(const Source &source);
	static const std::string getRepoURI(const Source &source);

private:
	double divider_;

	enum Source source_;
};

#endif

