#ifndef __GPX2VIDEO__TCX_H__
#define __GPX2VIDEO__TCX_H__

#include <string.h>
//#define __USE_XOPEN  // For strptime
#include <time.h>
#include <unistd.h>

#include <string>
#include <algorithm>

#include "tcxlib/Parser.h"
#include "tcxlib/ReportCerr.h"
#include "log.h"
#include "telemetrymedia.h"


class TCX : public TelemetrySource {
public:
	enum Version {
		V1_0,
		V2_0,
	};

	TCX(const std::string &filename)
		: TelemetrySource(filename)
		, root_(NULL)
		, activity_(NULL)
		, eof_(false) 
		, laps_(laps_dummy_list_)
		, tracks_(tracks_dummy_list_) 
		, trkpts_(trkpts_dummy_list_) {
		tcx::Parser parser(NULL); //&report);

//		tcx::ReportCerr report;

		if (!stream_.is_open()) {
			log_error("Open '%s' TCX file failure, please check that file is readable", filename.c_str());
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

	virtual ~TCX() {
	}

	std::string name(void) {
		return std::string("TCX");
	}

	void reset(void) {
		std::list<tcx::Activity*> &activities = root_->activities().activities().list();

		log_call();

		eof_ = false;

		for (std::list<tcx::Activity *>::iterator node = activities.begin(); node != activities.end(); ++node) {
			tcx::Activity *activity = (*node);

			if (activity == nullptr)
				continue;

			laps_ = activity->laps().list();

			// Retrieve first activity, first lap & first track
			activity_ = activity;
			iter_lap_ = laps_.begin();

			if (iter_lap_ != laps_.end()) {
				tcx::Lap *lap = (*iter_lap_);

				tracks_ = lap->tracks().list();
				iter_trk_ = tracks_.begin();

				if (iter_trk_ != tracks_.end()) {
					tcx::Track *track = (*iter_trk_);

					trkpts_ = track->trackPoints().list();
					iter_pts_ = trkpts_.begin();
				}
			}

			// Parse only the first activity
			break;
		}
	}

	enum TelemetrySource::Data read(TelemetrySource::Point &point) {
		tcx::TrackPoint *trkpt = NULL;

		enum TelemetrySource::Data type = TelemetrySource::DataUnknown;

		log_call();

		type = readNode(&trkpt);

		if (type == TelemetrySource::DataEof)
			goto eof;

		writePoint(trkpt, point);
eof:
		return type;
	}

	enum TelemetrySource::Data readNode(tcx::TrackPoint **trkpt) {
		tcx::Lap *lap = (*iter_lap_);
		tcx::Track *track = (*iter_trk_);

		log_call();

		if (eof_)
			return TelemetrySource::DataEof;

		*trkpt = (*iter_pts_);

		// Next point
		iter_pts_++;

		for (; iter_pts_ == trkpts_.end();) {
			iter_trk_++;

			if (iter_trk_ == tracks_.end()) {
				iter_lap_++;

				if (iter_lap_ == laps_.end()) {
					eof_ = true;
					break;
				}

				lap = (*iter_lap_);

				tracks_ = lap->tracks().list();
				iter_trk_ = tracks_.begin();
			}

			track = (*iter_trk_);

			trkpts_ = track->trackPoints().list();
			iter_pts_ = trkpts_.begin();
		}

		return TelemetrySource::DataAgain;
	}

	void writePoint(tcx::TrackPoint *trkpt, TelemetrySource::Point &point) {
		bool ok = true;

		uint64_t ts = 0;

		const char *str;

		std::string name;

		tcx::Node *heartrate = NULL;
		tcx::Node *extensions = NULL;

		// Convert time - TCX file contains UTC time
		str = trkpt->time().getValue().c_str();

		// Parse datetime string
		ts = Datetime::string2timestamp(str);

		// Line
		point.setLine(trkpt->line());

		if (!ok)
			return;

		// Build result
		point.setPosition(
				ts,
				(double) trkpt->position().latitude(),
				(double) trkpt->position().longitude()
		);
		point.setElevation((double) trkpt->altitude());

		// Heartrate
		heartrate = &trkpt->heartrate();
		if (heartrate) {
			for (std::list<tcx::Node*>::const_iterator iter = heartrate->getElements().begin(); 
				iter != heartrate->getElements().end(); ++iter) {
				tcx::Node *node = (*iter);

				name = node->getName();
		
				std::transform(name.begin(), name.end(), name.begin(), ::tolower);

				if (name.find("value") != std::string::npos)
					point.setHeartrate(std::stoi(node->getValue()));
			}
		}

		// Cadence
		for (std::list<tcx::Node*>::const_iterator iter = trkpt->getElements().begin(); 
			iter != trkpt->getElements().end(); ++iter) {

			tcx::Node *node = (*iter);

			name = node->getName();
	
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);

			if (name.find("cadence") != std::string::npos)
				point.setCadence(std::stoi(node->getValue()));
		}

		// Extensions
		extensions = &trkpt->extensions();
		if (extensions) {
			for (std::list<tcx::Node*>::const_iterator iter = extensions->getElements().begin(); 
				iter != extensions->getElements().end(); ++iter) {
				tcx::Node *node = (*iter);

				name = node->getName();
		
				std::transform(name.begin(), name.end(), name.begin(), ::tolower);

				if (name.find("watts") != std::string::npos)
					point.setPower(std::stoi(node->getValue()));
			}
		}

	}

private:
	tcx::TCX *root_;

	tcx::Activity *activity_;

	bool eof_;

	std::list<tcx::Lap*>::iterator iter_lap_;
	std::list<tcx::Track*>::iterator iter_trk_;
	std::list<tcx::TrackPoint*>::iterator iter_pts_;

	std::list<tcx::Lap*> laps_dummy_list_;
	std::list<tcx::Lap*> &laps_;

	std::list<tcx::Track*> tracks_dummy_list_;
	std::list<tcx::Track*> &tracks_;

	std::list<tcx::TrackPoint*> trkpts_dummy_list_;
	std::list<tcx::TrackPoint*> &trkpts_;
};

#endif

