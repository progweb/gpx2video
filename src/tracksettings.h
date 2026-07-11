#ifndef __GPX2VIDEO__TRACKSETTINGS_H__
#define __GPX2VIDEO__TRACKSETTINGS_H__

#include <iostream>
#include <string>

#include "videowidget.h"


class TrackSettings {
public:
	enum View {
		ViewDefault,
		ViewLockCenter,
		ViewZoomFit,
		ViewUnknown
	};

	enum Follow {
		FollowNone,
		FollowCourse,
		FollowHeading,
		FollowUnknown
	};

	enum Icon {
		IconDefault = -1,

		// Internal type icons
		IconPlay = 0,
		IconStop,
		IconPosition,

		// Other internal icons
		IconFinish,
		IconNeedle,
		IconSpot,
		IconPositionPlay,
		IconPositionStop,
		IconPositionBike,
		IconPositionDronePotensic,

		// At last user icons
		IconUserFile,

		IconUnknown
	};

#define IconStart IconPlay
#define IconEnd IconStop

	TrackSettings();
	virtual ~TrackSettings();

	const int& width(void) const;
	const int& height(void) const;
	void setSize(const int &width, const int &height);

	const int& zoom(void) const;
	void setZoom(const int &zoom);

	const bool& zoomfit(void) const;
	void setZoomfit(const bool &enable);

	const View& view(void) const;
	void setView(const View &view);

	const double& divider(void) const;
	void setDivider(const double &divier);

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

	const Follow& follow(void) const;
	void setFollow(const Follow &follow);

	Icon icon(Icon type) const {
		switch (type) {
		case IconStart:
			return icon_start_;
		case IconEnd:
			return icon_end_;
		case IconPosition:
			return icon_position_;
		default:
			return IconUnknown;
		}
	}

	void setIcon(const Icon &type, const Icon &icon) {
		switch (type) {
		case IconStart:
			icon_start_ = icon;
			break;
		case IconEnd:
			icon_end_ = icon;
			break;
		case IconPosition:
			icon_position_ = icon;
			break;
		default:
			break;
		}
	}

	const std::string& iconFile(const Icon &type) const {
		switch (type) {
		case IconStart:
			return icon_start_file_;
		case IconEnd:
			return icon_end_file_;
		case IconPosition:
			return icon_position_file_;
		default:
			return dummy_;
		}
	}

	void setIconFile(const Icon &type, const std::string &file) {
		switch (type) {
		case IconStart:
			icon_start_file_ = file;
			break;
		case IconEnd:
			icon_end_file_ = file;
			break;
		case IconPosition:
			icon_position_file_ = file;
			break;
		default:
			break;
		}
	}

	const float * iconColor(const Icon &type) const {
		switch (type) {
		case IconStart:
			return icon_start_color_;
		case IconEnd:
			return icon_end_color_;
		case IconPosition:
			return icon_position_color_;
		default:
			return NULL;
		}
	}

	bool setIconColor(const Icon &type, std::string color) {
		switch (type) {
		case IconStart:
			return VideoWidget::Theme::hex2color(icon_start_color_, color);
		case IconEnd:
			return VideoWidget::Theme::hex2color(icon_end_color_, color);
		case IconPosition:
			return VideoWidget::Theme::hex2color(icon_position_color_, color);
		default:
			return false;
		}
	}

	bool setIconColor(const Icon &type, double r, double g, double b, double a) {
		switch (type) {
		case IconStart:
			icon_start_color_[0] = r;
			icon_start_color_[1] = g;
			icon_start_color_[2] = b;
			icon_start_color_[3] = a;
			break;
		case IconEnd:
			icon_end_color_[0] = r;
			icon_end_color_[1] = g;
			icon_end_color_[2] = b;
			icon_end_color_[3] = a;
			break;
		case IconPosition:
			icon_position_color_[0] = r;
			icon_position_color_[1] = g;
			icon_position_color_[2] = b;
			icon_position_color_[3] = a;
			break;
		default:
			return false;
		}
		return true;
	}

	const double& iconSize(const Icon &type) const {
		switch (type) {
		case IconStart:
			return icon_start_size_;
		case IconEnd:
			return icon_end_size_;
		case IconPosition:
			return icon_position_size_;
		default:
			return null_;
		}
	}

	void setIconSize(const Icon &type, const double &size) {
		if (size < 0)
			return;

		switch (type) {
		case IconStart:
			icon_start_size_ = size;
			break;
		case IconEnd:
			icon_end_size_ = size;
			break;
		case IconPosition:
			icon_position_size_ = size;
			break;
		default:
			break;
		}
	}

	void getBoundingBox(double *lat1, double *lon1, double *lat2, double *lon2) const;
	void setBoundingBox(double lat1, double lon1, double lat2, double lon2);

	static View string2view(std::string &s);
	static Follow string2follow(std::string &s);
	static Icon string2icon(std::string &s);

	static std::string view2string(View view);
	static std::string follow2string(Follow follow);
	static std::string icon2string(Icon icon);

protected:
	int zoom_;
	View view_;

	double null_;
	double divider_;

	double path_thick_;
	double path_border_;

	float path_border_color_[4];
	float path_primary_color_[4];
	float path_secondary_color_[4];

	Follow follow_;

	Icon icon_start_;
	std::string icon_start_file_;
	float icon_start_color_[4];
	double icon_start_size_;

	Icon icon_end_;
	std::string icon_end_file_;
	float icon_end_color_[4];
	double icon_end_size_;

	Icon icon_position_;
	std::string icon_position_file_;
	float icon_position_color_[4];
	double icon_position_size_;

	std::string dummy_;

private:
	int width_, height_;

	double lat1_, lat2_;
	double lon1_, lon2_;
};

#endif

