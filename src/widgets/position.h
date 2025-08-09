#ifndef __GPX2VIDEO__WIDGETS__POSITION_H__
#define __GPX2VIDEO__WIDGETS__POSITION_H__

#include "../utils.h"
#include "../videowidget.h"


class PositionWidget : public VideoWidget {
public:
	virtual ~PositionWidget() {
		clear();
	}

	static PositionWidget * create(GPXApplication &app) {
		PositionWidget *widget;

		widget = new PositionWidget(app, "position");

		return widget;
	}

	OIIO::ImageBuf * prepare(bool &is_update) {
		bool with_picto = this->hasFlag(VideoWidget::FlagPicto);

		if (bg_buf_ != NULL) {
			is_update = false;
			goto skip;
		}

		this->createBox(&bg_buf_, this->width(), this->height());
		this->drawBorder(bg_buf_);
		this->drawBackground(bg_buf_);
		if (with_picto)
			this->drawImage(bg_buf_, this->border(), this->border(), "./assets/picto/DataOverlay_icn_position.png", VideoWidget::ZoomFit);
		this->drawLabel(bg_buf_, label().c_str());

		is_update = true;
skip:
		return bg_buf_;
	}

	OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) {
		char s[128];

		// Refresh dynamic info
		if (fg_buf_ != NULL) {
			if (data.type() == TelemetryData::TypeUnchanged) {
				is_update = false;
				goto skip;
			}

			if (no_value_ && !data.hasValue(TelemetryData::DataFix)) {
				is_update = false;
				goto skip;
			}
		}

		// Format data
		no_value_ = !data.hasValue(TelemetryData::DataFix);

		if (!no_value_)
			sprintf(s, "%.4f, %.4f", data.latitude(), data.longitude());
		else
			sprintf(s, "--, --");

		// Refresh dynamic info
		if (fg_buf_ != NULL)
			delete fg_buf_;

		this->createBox(&fg_buf_, this->width(), this->height());
		this->drawValue(fg_buf_, s);

		is_update = true;
skip:
		return fg_buf_;
	}

	void clear(void) {
		no_value_ = false;

		if (bg_buf_)
			delete bg_buf_;

		if (fg_buf_)
			delete fg_buf_;

		bg_buf_ = NULL;
		fg_buf_ = NULL;
	}

protected:
	void xmlwrite(std::ostream &os) {
		VideoWidget::xmlwrite(os);

		IndentingOStreambuf indent(os, 4);

		os << "<text-shadow>" << textShadow() << "</text-shadow>" << std::endl;
	}

private:
	bool no_value_;

	OIIO::ImageBuf *bg_buf_;
	OIIO::ImageBuf *fg_buf_;

	PositionWidget(GPXApplication &app, std::string name)
		: VideoWidget(app, name)
   		, bg_buf_(NULL)
   		, fg_buf_(NULL) {
		no_value_ = false;
	}
};

#endif

