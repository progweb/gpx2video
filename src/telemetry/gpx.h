#ifndef __GPX2VIDEO__GPX_H__
#define __GPX2VIDEO__GPX_H__

#include <string.h>
//#define __USE_XOPEN  // For strptime
#include <time.h>

#include "unistd.h"

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

	void reset() {
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

	/**
	 * Parse datetime string to timestamp (in ms)
	 */
	static bool parseDatetime(const char *str, uint64_t *timestamp) {
		bool ok = true;

		uint64_t ts = 0;

		struct tm time;

		char buf[128];
		const char *s;

		log_call();

		// reset
		memset(&time, 0, sizeof(time));

		// Try format: "2020:12:13 08:55:48"
		// Try format: "2020:12:13 08:55:48.123"
		// Try format: "2020:12:13 08:55:48.123456"
		// Try format: "2020:12:13 08:55:48.123456+0200"
		if ((s = strptime(str, "%Y:%m:%d %H:%M:%S", &time)) != NULL)
			ts = timegm(&time) * 1000;
		// Try format: "2020-07-28T07:04:43"
		// Try format: "2020-07-28T07:04:43Z"
		// Try format: "2020-07-28T07:04:43.123"
		// Try format: "2020-07-28T07:04:43.123456"
		// Try format: "2020-07-28T07:04:43.123456+0200"
		else if ((s = strptime(str, "%Y-%m-%dT%H:%M:%S", &time)) != NULL)
			ts = timegm(&time) * 1000;
		else
			ok = false;

		// Parse ms precision
		if (ok) {
			char *ms;

			strncpy(buf, s, sizeof(buf));
			buf[sizeof(buf) - 1] = '\0';

			if ((ms = ::strchr(buf, '.')) != NULL) {
				ms += 1; // skip '.' char

				if (strlen(ms) >= 3) {
					ms[3] = '\0';

					ts += atoi(ms);
				}
			}
		}

		// Parse +0200 / -0200 / +02:00 / -02:00
		if (ok) {
			char *tz;
			char *pos;
			int offset;

			::strcpy(buf, s);

			if ((pos = ::strchr(buf, ':')) != NULL) {
				strcpy(pos, pos + 1);
			}

			if ((tz = ::strchr(buf, '+')) != NULL) {
				tz += 1; // skip '+' char

				if (strlen(tz) >= 4) {
					tz[4] = '\0';

					offset = atoi(tz);

					ts -= (offset / 100) * 60 * 60 * 1000; // hour offset
					ts -= (offset % 100) * 60 * 1000; // min offset
				}
			}
			else if ((tz = ::strchr(buf, '-')) != NULL) {
				tz += 1; // skip '-' char

				if (strlen(tz) >= 4) {
					tz[4] = '\0';

					offset = atoi(tz);

					ts += (offset / 100) * 60 * 60 * 1000; // hour offset
					ts += (offset % 100) * 60 * 1000; // min offset
				}
			}
		}

		if (ok && (timestamp != NULL))
			*timestamp = ts;

		return ok;
	}

	void writePoint(gpx::WPT *wpt, TelemetrySource::Point &point) {
		bool ok = true;

		uint64_t ts = 0;

		const char *str;

		std::string name;

		// Convert time - GPX file contains UTC time
		str = wpt->time().getValue().c_str();

		// Parse datetime string
		parseDatetime(str, &ts);

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
		gpx::Node *extensions = wpt->extensions().getElements().front();

		if (extensions) {
			name = extensions->getName();

			if (name.find("TrackPointExtension") == std::string::npos)
				extensions = &wpt->extensions();

			if (extensions) {
				for (std::list<gpx::Node*>::const_iterator iter = extensions->getElements().begin(); 
					iter != extensions->getElements().end(); ++iter) {
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
	}

private:
	gpx::GPX *root_;

	gpx::TRK *trk_;

	bool eof_;

	std::list<gpx::WPT*>::iterator iter_pts_;
	std::list<gpx::TRKSeg*>::iterator iter_seg_;
};

#endif

