#ifndef __GPX2VIDEO__GPX_H__
#define __GPX2VIDEO__GPX_H__

#include <string.h>
//#define __USE_XOPEN  // For strptime
#include <time.h>
#include <unistd.h>

#include <string>
#include <algorithm>

#include "gpxlib/Parser.h"
#include "gpxlib/ReportCerr.h"
#include "log.h"
#include "telemetrymedia.h"


class GPX : public TelemetrySource {
public:
	enum Version {
		V1_0,
		V1_1,
	};

	GPX(const std::string &filename)
		: TelemetrySource(filename)
		, root_(NULL)
		, trk_(NULL)
		, eof_(false) {
		gpx::Parser parser(NULL); //&report);

//		gpx::ReportCerr report;

		if (!stream_.is_open()) {
			log_error("Open '%s' GPX file failure, please check that file is readable", filename.c_str());
			goto failure;
		}

		root_ = parser.parse(stream_);

		if (root_ == NULL) {
			log_error("Parsing of '%s' failed due to %s on line %d and column %d", 
				filename.c_str(), parser.errorText().c_str(),
				parser.errorLineNumber(), parser.errorColumnNumber());
			goto failure;
		}

failure:
		return;
	}

	virtual ~GPX() {
	}

	std::string name(void) {
		return std::string("GPX");
	}

	void reset(void) {
		std::list<gpx::TRK*> &trks = root_->trks().list();

		log_call();

		eof_ = false;

		for (std::list<gpx::TRK *>::iterator node = trks.begin(); node != trks.end(); ++node) {
			gpx::TRK *trk = (*node);

			if (trk == nullptr)
				continue;

			std::list<gpx::TRKSeg*> &trksegs = trk->trksegs().list();

			// Retrieve first segment & first track
			trk_ = trk;
			iter_seg_ = trksegs.begin();
			
			if (iter_seg_ != trksegs.end()) {
				gpx::TRKSeg *trkseg = (*iter_seg_);

				std::list<gpx::WPT*> &trkpts = trkseg->trkpts().list();

				iter_pts_ = trkpts.begin();
			}

			// Parse only the first track
			break;
		}
	}

	enum TelemetrySource::Data read(TelemetrySource::Point &point) {
		gpx::WPT *wpt = NULL;

		enum TelemetrySource::Data type = TelemetrySource::DataUnknown;

		log_call();

		type = readNode(&wpt);

		if (type == TelemetrySource::DataEof)
			goto eof;

		writePoint(wpt, point);

eof:
		return type;
	}

	enum TelemetrySource::Data readNode(gpx::WPT **wpt) {
		gpx::TRKSeg *trkseg = (*iter_seg_);

		std::list<gpx::WPT*> &trkpts = trkseg->trkpts().list();
		std::list<gpx::TRKSeg*> &trksegs = trk_->trksegs().list();

		log_call();

		if (eof_)
			return TelemetrySource::DataEof;

		*wpt = (*iter_pts_);

		// Next point
		iter_pts_++;

		for (; iter_pts_ == trkpts.end();) {
			iter_seg_++;

			if (iter_seg_ == trksegs.end()) {
				eof_ = true;
				break;
			}

			trkseg = (*iter_seg_);

			trkpts = trkseg->trkpts().list();
			iter_pts_ = trkpts.begin();
		}

		return TelemetrySource::DataAgain;
	}

	void writePoint(gpx::WPT *wpt, TelemetrySource::Point &point) {
		bool ok = true;

		uint64_t ts = 0;

		const char *str;

		std::string name;

		gpx::Node *extensions = NULL;
		gpx::Node *trackPointExtensions = NULL;

		// Convert time - GPX file contains UTC time
		str = wpt->time().getValue().c_str();

		// Parse datetime string
		ts = Datetime::string2timestamp(str);

		// Line
		point.setLine(wpt->line());

		if (!ok)
			return;

		// Build result
		point.setPosition(
				ts,
				(double) wpt->lat(),
				(double) wpt->lon()
		);
		point.setElevation((double) wpt->ele());

		// Extensions
		extensions = &wpt->extensions();
		if (extensions) {
			for (std::list<gpx::Node*>::const_iterator iter = extensions->getElements().begin(); 
				iter != extensions->getElements().end(); ++iter) {
				gpx::Node *node = (*iter);

				name = node->getName();

				if ((name.find("power") != std::string::npos)
						|| (name.find("Power") != std::string::npos))
					point.setPower(std::stoi(node->getValue()));
			
				if (name.find("TrackPointExtension") != std::string::npos)
					trackPointExtensions = node;
			}
		}

		// TrackPointExtensions
		if (trackPointExtensions) {
			for (std::list<gpx::Node*>::const_iterator iter = trackPointExtensions->getElements().begin(); 
				iter != trackPointExtensions->getElements().end(); ++iter) {
				gpx::Node *node = (*iter);

				name = node->getName();

				if (name.find("atemp") != std::string::npos)
					point.setTemperature(std::stod(node->getValue()));
				else if (name.find("cad") != std::string::npos)
					point.setCadence(std::stoi(node->getValue()));
				else if (name.find("hr") != std::string::npos)
					point.setHeartrate(std::stoi(node->getValue()));
			}
		}
	}

private:
	gpx::GPX *root_;

	gpx::TRK *trk_;

	bool eof_;

	std::list<gpx::WPT*>::iterator iter_pts_;
	std::list<gpx::TRKSeg*>::iterator iter_seg_;
};

#endif

