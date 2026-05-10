#include <iomanip>
#include <iostream>
#include <memory>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "log.h"
#include "utils.h"
#include "oiio.h"
#include "oiioutils.h"
#include "videowidget.h"



/**
 * VideoWidget::Theme
 */

VideoWidget::Theme::Theme() {
	setSize(240, 50);
	setFlags(VideoWidget::Theme::FlagAll);

	setPadding(VideoWidget::Theme::PaddingAll, 3);

	setBorder(1);
	setBorderColor(0.0, 0.0, 0.0, 1.0);
	setBackgroundColor(0.0, 0.0, 0.0, 0.8);

	setTickAlign(VideoWidget::Theme::AlignCenter);
	setTickColor(1.0, 1.0, 1.0, 1.0);
	setTickLabelColor(1.0, 1.0, 1.0, 1.0);

	setNeedleType(VideoWidget::Theme::NeedleTypeBasic);
	setNeedlePrimaryColor(1.0, 1.0, 1.0, 1.0);
	setNeedleSecondaryColor(1.0, 0.0, 0.0, 1.0);

	setGaugeBorder(0);
	setGaugeBorderColor(1.0, 1.0, 1.0, 1.0);
	setGaugeBackgroundColor(0.0, 0.0, 0.0, 0.0);
	setGaugeColor(0, 0.0, 0.8, 0.0, 0.8);
	setGaugeColor(1, 1.0, 0.0, 0.0, 1.0);

	setCursorColor(0.8, 0.0, 0.0, 0.8);

	setFont("./assets/fonts/Helvetica.ttf");

	setLabelFontFamily("Sans");
	setLabelFontSize(15);
	setLabelFontStyle(VideoWidget::Theme::FontStyleNormal);
	setLabelFontWeight(VideoWidget::Theme::FontWeightNormal);
	setLabelShadowOpacity(80);
	setLabelShadowDistance(3);
	setLabelAlign(VideoWidget::Theme::AlignLeft);
	setLabelColor(1.0, 1.0, 1.0, 1.0);
	setLabelBorderWidth(1);
	setLabelBorderColor(0.0, 0.0, 0.0, 1.0);

	setValueMin(0);
	setValueMax(1000);
	setValueFontFamily("Sans");
	setValueFontSize(25);
	setValueFontStyle(VideoWidget::Theme::FontStyleNormal);
	setValueFontWeight(VideoWidget::Theme::FontWeightNormal);
	setValueShadowOpacity(80);
	setValueShadowDistance(3);
	setValueAlign(VideoWidget::Theme::AlignLeft);
	setValueColor(1.0, 1.0, 1.0, 1.0);
	setValueBorderWidth(1);
	setValueBorderColor(0.0, 0.0, 0.0, 1.0);
	setValueBackgroundColor(0.0, 0.0, 0.0, 0.8);

//	setTextLineSpace(10);
}


bool VideoWidget::Theme::hex2color(float color[4], std::string hex) {
	if (hex.empty())
		return false;

	if (hex.at(0) == '#')
		hex.erase(0, 1);

	while (hex.length() < 8)
		hex += "0";

	color[0] = std::stoi(hex.substr(0, 2), NULL, 16) / 255.0;
	color[1] = std::stoi(hex.substr(2, 2), NULL, 16) / 255.0;
	color[2] = std::stoi(hex.substr(4, 2), NULL, 16) / 255.0;
	color[3] = std::stoi(hex.substr(6, 2), NULL, 16) / 255.0;

	return true;
}


std::string VideoWidget::Theme::color2hex(const float color[4]) {
	log_call();

	std::ostringstream stream;

	stream << "#" 
		<< std::setfill('0') << std::setw(2) << std::setbase(16) 
		<< (int) (color[0] * 255.0)  // R
		<< std::setfill('0') << std::setw(2) << std::setbase(16) 
		<< (int) (color[1] * 255.0)  // G
		<< std::setfill('0') << std::setw(2) << std::setbase(16) 
		<< (int) (color[2] * 255.0)  // B
		<< std::setfill('0') << std::setw(2) << std::setbase(16) 
		<< (int) (color[3] * 255.0); // A

	return stream.str();
}


/**
 * VideoWidget
 */

VideoWidget::Widget VideoWidget::string2widget(std::string &s) {
	VideoWidget::Widget type;

	if (s == "avgspeed")
		type = VideoWidget::WidgetAverageSpeed;
	else if (s == "avgridespeed")
		type = VideoWidget::WidgetAverageRideSpeed;
	else if (s == "batterylevel")
		type = VideoWidget::WidgetBatteryLevel;
	else if (s == "cadence")
		type = VideoWidget::WidgetCadence;
	else if (s == "date")
		type = VideoWidget::WidgetDate;
	else if (s == "distance")
		type = VideoWidget::WidgetDistance;
	else if (s == "duration")
		type = VideoWidget::WidgetDuration;
	else if (s == "elevation")
		type = VideoWidget::WidgetElevation;
	else if (s == "gforce")
		type = VideoWidget::WidgetGForce;
	else if (s == "gpx")
		type = VideoWidget::WidgetGPX;
	else if (s == "grade")
		type = VideoWidget::WidgetGrade;
	else if (s == "heading")
		type = VideoWidget::WidgetHeading;
	else if (s == "heartrate")
		type = VideoWidget::WidgetHeartRate;
	else if (s == "homedistance")
		type = VideoWidget::WidgetHomeDistance;
	else if (s == "image")
		type = VideoWidget::WidgetImage;
	else if (s == "lap")
		type = VideoWidget::WidgetLap;
	else if (s == "map")
		type = VideoWidget::WidgetMap;
	else if (s == "maxspeed")
		type = VideoWidget::WidgetMaxSpeed;
	else if (s == "position")
		type = VideoWidget::WidgetPosition;
	else if (s == "power")
		type = VideoWidget::WidgetPower;
	else if (s == "speed")
		type = VideoWidget::WidgetSpeed;
	else if (s == "temperature")
		type = VideoWidget::WidgetTemperature;
	else if (s == "text")
		type = VideoWidget::WidgetText;
	else if (s == "time")
		type = VideoWidget::WidgetTime;
	else if (s == "track")
		type = VideoWidget::WidgetTrack;
	else if (s == "verticalspeed")
		type = VideoWidget::WidgetVerticalSpeed;
	else
		type = VideoWidget::WidgetUnknown;

	return type;
}


VideoWidget::Shape VideoWidget::string2shape(std::string &s) {
	VideoWidget::Shape shape;

	if (s.empty() || (s == "none"))
		shape = VideoWidget::ShapeNone;
	else if (s == "text")
		shape = VideoWidget::ShapeText;
	else if (s == "arc")
		shape = VideoWidget::ShapeArc;
	else if (s == "bar")
		shape = VideoWidget::ShapeBar;
	else
		shape = VideoWidget::ShapeUnknown;

	return shape;
}


VideoWidget::Position VideoWidget::string2position(std::string &s) {
	VideoWidget::Position position;

	if (s.empty() || (s == "none"))
		position = VideoWidget::PositionNone;
	else if (s == "left")
		position = VideoWidget::PositionLeft;
	else if (s == "right")
		position = VideoWidget::PositionRight;
	else if (s == "bottom")
		position = VideoWidget::PositionBottom;
	else if (s == "top")
		position = VideoWidget::PositionTop;
	else if ((s == "bottom-left") || (s == "left-bottom"))
		position = VideoWidget::PositionBottomLeft;
	else if ((s == "bottom-right") || (s == "right-bottom"))
		position = VideoWidget::PositionBottomRight;
	else if ((s == "top-left") || (s == "left-top"))
		position = VideoWidget::PositionTopLeft;
	else if ((s == "top-right") || (s == "right-top"))
		position = VideoWidget::PositionTopRight;
	else
		position = VideoWidget::PositionUnknown;

	return position;
}


VideoWidget::Orientation VideoWidget::string2orientation(std::string &s) {
	VideoWidget::Orientation orientation;

	if (s.empty() || (s == "none"))
		orientation = VideoWidget::OrientationNone;
	else if (s == "horizontal")
		orientation = VideoWidget::OrientationHorizontal;
	else if (s == "vertical")
		orientation = VideoWidget::OrientationVertical;
	else
		orientation = VideoWidget::OrientationUnknown;

	return orientation;
}


VideoWidget::Theme::Align VideoWidget::string2align(std::string &s) {
	VideoWidget::Theme::Align align;

	if (s.empty() || (s == "none") || (s == "left"))
		align = VideoWidget::Theme::AlignLeft;
	else if (s == "center")
		align = VideoWidget::Theme::AlignCenter;
	else if (s == "right")
		align = VideoWidget::Theme::AlignRight;
	else
		align = VideoWidget::Theme::AlignUnknown;

	return align;
}


VideoWidget::Theme::FontStyle VideoWidget::string2fontstyle(std::string &s) {
	VideoWidget::Theme::FontStyle style;

	if (s.empty() || (s == "none") || (s == "normal"))
		style = VideoWidget::Theme::FontStyleNormal;
	else if (s == "oblique")
		style = VideoWidget::Theme::FontStyleOblique;
	else if (s == "italic")
		style = VideoWidget::Theme::FontStyleItalic;
	else
		style = VideoWidget::Theme::FontStyleUnknown;

	return style;
}


VideoWidget::Theme::FontWeight VideoWidget::string2fontweight(std::string &s) {
	int v = 0;

	VideoWidget::Theme::FontWeight weight;

	if (s.empty() || (s == "none"))
		weight = VideoWidget::Theme::FontWeightNormal;
	else {
		// Catch stoi errors
		try {
			v = std::stoi(s);
		}
		catch (const std::invalid_argument &e) {
		}
		catch (const std::out_of_range &e) {
		}

		if ((s == "thin") || (v == 100))
			weight = VideoWidget::Theme::FontWeightThin;
		else if ((s == "ultralight") || (v == 200))
			weight = VideoWidget::Theme::FontWeightUltraLight;
		else if ((s == "light") || (v == 300))
			weight = VideoWidget::Theme::FontWeightLight;
		else if ((s == "semilight") || (v == 350))
			weight = VideoWidget::Theme::FontWeightSemiLight;
		else if ((s == "book") || (v == 380))
			weight = VideoWidget::Theme::FontWeightBook;
		else if ((s == "normal") || (v == 400))
			weight = VideoWidget::Theme::FontWeightNormal;
		else if ((s == "medium") || (v == 500))
			weight = VideoWidget::Theme::FontWeightMedium;
		else if ((s == "semibold") || (v == 600))
			weight = VideoWidget::Theme::FontWeightSemiBold;
		else if ((s == "bold") || (v == 700))
			weight = VideoWidget::Theme::FontWeightBold;
		else if ((s == "ultrabold") || (v == 800))
			weight = VideoWidget::Theme::FontWeightUltraBold;
		else if ((s == "heavy") || (v == 900))
			weight = VideoWidget::Theme::FontWeightHeavy;
		else if ((s == "ultraheavy") || (v == 1000))
			weight = VideoWidget::Theme::FontWeightUltraHeavy;
		else
			weight = VideoWidget::Theme::FontWeightUnknown;
	}

	return weight;
}


VideoWidget::Theme::NeedleType VideoWidget::string2needletype(std::string &s) {
	VideoWidget::Theme::NeedleType type;

	if (s.empty() || (s == "none") || (s == "baisc"))
		type = VideoWidget::Theme::NeedleTypeBasic;
	else if (s == "thin")
		type = VideoWidget::Theme::NeedleTypeThin;
	else if (s == "light")
		type = VideoWidget::Theme::NeedleTypeLight;
	else if (s == "design")
		type = VideoWidget::Theme::NeedleTypeDesign;
	else
		type = VideoWidget::Theme::NeedleTypeUnknown;

	return type;
}


VideoWidget::Unit VideoWidget::string2unit(std::string &s) {
	VideoWidget::Unit unit;

	if (s.empty() || (s == "none"))
		unit = VideoWidget::UnitNone;
	else if (s == "mps")
		unit = VideoWidget::UnitMPS;
	else if (s == "mph")
		unit = VideoWidget::UnitMPH;
	else if (s == "kph")
		unit = VideoWidget::UnitKPH;
	else if ((s == "mpm") || (s == "minmile"))
		unit = VideoWidget::UnitMPM;
	else if ((s == "mpk") || (s == "minkm"))
		unit = VideoWidget::UnitMPK;
	else if (s == "km")
		unit = VideoWidget::UnitKm;
	else if (s == "m")
		unit = VideoWidget::UnitMeter;
	else if (s == "ft")
		unit = VideoWidget::UnitFoot;
	else if (s == "miles")
		unit = VideoWidget::UnitMiles;
	else if ((s == "C") || (s == "celsius"))
		unit = VideoWidget::UnitCelsius;
	else if ((s == "F") || (s == "farenheit"))
		unit = VideoWidget::UnitFarenheit;
	else if ((s == "g") || (s == "G"))
		unit = VideoWidget::UnitG;
	else if (s == "mps2")
		unit = VideoWidget::UnitMeterPS2;
	else
		unit = VideoWidget::UnitUnknown;

	return unit;
}


VideoWidget::Zoom VideoWidget::string2zoom(std::string &s) {
	VideoWidget::Zoom zoom;

	if (s.empty() || (s == "none"))
		zoom = VideoWidget::ZoomNone;
	else if (s == "fit")
		zoom = VideoWidget::ZoomFit;
	else if (s == "fill")
		zoom = VideoWidget::ZoomFill;
	else if (s == "crop")
		zoom = VideoWidget::ZoomCrop;
	else if (s == "stretch")
		zoom = VideoWidget::ZoomStretch;
	else
		zoom = VideoWidget::ZoomUnknown;

	return zoom;
}


std::string VideoWidget::bool2string(bool value) {
	return value ? "true" : "false";
}


std::string VideoWidget::widget2string(VideoWidget::Widget type) {
	switch (type) {
	case VideoWidget::WidgetAverageSpeed:
		return "avgspeed";
	case VideoWidget::WidgetAverageRideSpeed:
		return "avgridespeed";
	case VideoWidget::WidgetBatteryLevel:
		return "batterylevel";
	case VideoWidget::WidgetCadence:
		return "cadence";
	case VideoWidget::WidgetDate:
		return "date";
	case VideoWidget::WidgetDistance:
		return "distance";
	case VideoWidget::WidgetDuration:
		return "duration";
	case VideoWidget::WidgetElevation:
		return "elevation";
	case VideoWidget::WidgetGForce:
		return "gforce";
	case VideoWidget::WidgetGPX:
		return "gpx";
	case VideoWidget::WidgetGrade:
		return "grade";
	case VideoWidget::WidgetHeading:
		return "heading";
	case VideoWidget::WidgetHeartRate:
		return "heartrate";
	case VideoWidget::WidgetHomeDistance:
		return "homedistance";
	case VideoWidget::WidgetImage:
		return "image";
	case VideoWidget::WidgetLap:
		return "lap";
	case VideoWidget::WidgetMap:
		return "map";
	case VideoWidget::WidgetMaxSpeed:
		return "maxspeed";
	case VideoWidget::WidgetPosition:
		return "position";
	case VideoWidget::WidgetPower:
		return "power";
	case VideoWidget::WidgetSpeed:
		return "speed";
	case VideoWidget::WidgetTemperature:
		return "temperature";
	case VideoWidget::WidgetText:
		return "text";
	case VideoWidget::WidgetTime:
		return "time";
	case VideoWidget::WidgetTrack:
		return "track";
	case VideoWidget::WidgetVerticalSpeed:
		return "verticalspeed";
	default:
		return "";
	}
}


std::string VideoWidget::position2string(Position position) {
	switch (position) {
	case VideoWidget::PositionLeft:
		return "left";
	case VideoWidget::PositionRight:
		return "right";
	case VideoWidget::PositionBottom:
		return "bottom";
	case VideoWidget::PositionTop:
		return "top";
	case VideoWidget::PositionBottomLeft:
		return "bottom-left";
	case VideoWidget::PositionBottomRight:
		return "bottom-right";
	case VideoWidget::PositionTopLeft:
		return "top-left";
	case VideoWidget::PositionTopRight:
		return "top-right";
	default:
		return "";
	}
}


std::string VideoWidget::orientation2string(Orientation orientation) {
	switch (orientation) {
	case OrientationHorizontal:
		return "horizontal";
	case OrientationVertical:
		return "vertical";
	default:
		return "";
	}
}


std::string VideoWidget::align2string(Theme::Align align) {
	switch (align) {
	case Theme::AlignLeft:
		return "left";
	case Theme::AlignCenter:
		return "center";
	case Theme::AlignRight:
		return "right";
	default:
		return "";
	}
}


std::string VideoWidget::fontstyle2string(Theme::FontStyle style) {
	switch (style) {
	case Theme::FontStyleNormal:
		return "normal";
	case Theme::FontStyleOblique:
		return "oblique";
	case Theme::FontStyleItalic:
		return "italic";
	default:
		return "";
	}
}


std::string VideoWidget::fontweight2string(Theme::FontWeight weight) {
	switch (weight) {
	case Theme::FontWeightThin:
		return "thin";
	case Theme::FontWeightUltraLight:
		return "ultralight";
	case Theme::FontWeightLight:
		return "light";
	case Theme::FontWeightSemiLight:
		return "semilight";
	case Theme::FontWeightBook:
		return "book";
	case Theme::FontWeightNormal:
		return "normal";
	case Theme::FontWeightMedium:
		return "medium";
	case Theme::FontWeightSemiBold:
		return "semibold";
	case Theme::FontWeightBold:
		return "bold";
	case Theme::FontWeightUltraBold:
		return "ultrabold";
	case Theme::FontWeightHeavy:
		return "heavy";
	case Theme::FontWeightUltraHeavy:
		return "ultraheavy";
	default:
		return "";
	}
}

std::string VideoWidget::needletype2string(Theme::NeedleType type) {
	switch (type) {
	case Theme::NeedleTypeThin:
		return "thin";
	case Theme::NeedleTypeLight:
		return "light";
	case Theme::NeedleTypeBasic:
		return "basic";
	case Theme::NeedleTypeDesign:
		return "design";
	default:
		return "";
	}
}


std::string VideoWidget::unit2string(VideoWidget::Unit unit, bool label) {
	switch (unit) {
	case VideoWidget::UnitMPS:
		return label ? "m/s" : "mps";
	case VideoWidget::UnitMPH:
		return label ? "m/h" : "mph";
	case VideoWidget::UnitKPH:
		return label ? "km/h" : "kph";
	case VideoWidget::UnitMPM:
		return label ? "min/mile" : "mpm";
	case VideoWidget::UnitMPK:
		return label ? "min/km" : "mpk";
	case VideoWidget::UnitKm:
		return "km";
	case VideoWidget::UnitMeter:
		return "m";
	case VideoWidget::UnitFoot:
		return "ft";
	case VideoWidget::UnitMiles:
		return "miles";
	case VideoWidget::UnitCelsius:
		return label ? "°C" : "celsius";
	case VideoWidget::UnitFarenheit:
		return label ? "F" : "farenheit";
	case VideoWidget::UnitG:
		return "g";
	case VideoWidget::UnitMeterPS2:
		return label ? "m/s²" : "mps2";
	case VideoWidget::UnitNone:
	case VideoWidget::UnitUnknown:
	default:
		return "";
	}
}


void VideoWidget::save(std::ostream &os) {
	log_call();

	os.imbue(std::locale::classic());

	xmlopen(os);
	xmlwrite(os);
	xmlclose(os);
}


void VideoWidget::xmlopen(std::ostream &os) {
	log_call();

	os << "<widget";
	os <<   " x=\"" << x() << "\" y=\"" << y() << "\"";
	os <<   " width=\"" << theme().width() << "\" height=\"" << theme().height() << "\"";
	os <<   " position=\"" << position2string(position()) << "\"";
	os <<   " orientation=\"" << orientation2string(orientation()) << "\"";
	os <<   " display=\"true\"";
   	os <<   ">" << std::endl;
}

void VideoWidget::xmlclose(std::ostream &os) {
	log_call();

	os << "</widget>" << std::endl;
}


void VideoWidget::xmlwrite(std::ostream &os) {
	log_call();

	IndentingOStreambuf indent(os, 4);

	os << "<type>" << name() << "</type>" << std::endl;
	os << "<name>" << label() << "</name>" << std::endl;
	os << "<margin-left>" << margin(Margin::MarginLeft) << "</margin-left>" << std::endl;
	os << "<margin-right>" << margin(Margin::MarginRight) << "</margin-right>" << std::endl;
	os << "<margin-top>" << margin(Margin::MarginTop) << "</margin-top>" << std::endl;
	os << "<margin-bottom>" << margin(Margin::MarginBottom) << "</margin-bottom>" << std::endl;
}


std::string VideoWidget::getIconFilename(VideoWidget::Widget type) {
	log_call();

	std::string path = GPXApplication::assets("icons");

	switch (type) {
	case VideoWidget::WidgetAverageSpeed:
	case VideoWidget::WidgetAverageRideSpeed:
		return path + "/DataOverlay_icn_avgspeed.svg";
	case VideoWidget::WidgetBatteryLevel:
		return path + "/DataOverlay_icn_battery.svg";
	case VideoWidget::WidgetCadence:
		return path + "/DataOverlay_icn_cadence.svg";
	case VideoWidget::WidgetDate:
		return path + "/DataOverlay_icn_date.svg";
	case VideoWidget::WidgetDistance:
		return path + "/DataOverlay_icn_distance.svg";
	case VideoWidget::WidgetDuration:
		return path + "/DataOverlay_icn_duration.svg";
	case VideoWidget::WidgetElevation:
		return path + "/DataOverlay_icn_elevation.svg";
	case VideoWidget::WidgetGForce:
		return path + "/DataOverlay_icn_gforce.svg";
	case VideoWidget::WidgetGPX:
		return path + "/DataOverlay_icn_position.svg";
	case VideoWidget::WidgetGrade:
		return path + "/DataOverlay_icn_grade.svg";
	case VideoWidget::WidgetHeading:
		return path + "/DataOverlay_icn_compass.svg";
	case VideoWidget::WidgetHeartRate:
		return path + "/DataOverlay_icn_heartrate.svg";
	case VideoWidget::WidgetHomeDistance:
		return path + "/DataOverlay_icn_home.svg";
	case VideoWidget::WidgetImage:
		return path + "/DataOverlay_icn_image.svg";
	case VideoWidget::WidgetLap:
		return path + "/DataOverlay_icn_laps.svg";
	case VideoWidget::WidgetMap:
		return path + "/DataOverlay_icn_map.svg";
	case VideoWidget::WidgetMaxSpeed:
		return path + "/DataOverlay_icn_maxspeed.svg";
	case VideoWidget::WidgetPosition:
		return path + "/DataOverlay_icn_position.svg";
	case VideoWidget::WidgetPower:
		return path + "/DataOverlay_icn_power.svg";
	case VideoWidget::WidgetSpeed:
		return path + "/DataOverlay_icn_speed.svg";
	case VideoWidget::WidgetTemperature:
		return path + "/DataOverlay_icn_temperature.svg";
	case VideoWidget::WidgetText:
		return path + "/DataOverlay_icn_text.svg";
	case VideoWidget::WidgetTime:
		return path + "/DataOverlay_icn_time.svg";
	case VideoWidget::WidgetTrack:
		return path + "/DataOverlay_icn_track.svg";
	case VideoWidget::WidgetVerticalSpeed:
		return path + "/DataOverlay_icn_aviationAltitude.svg";
	default:
		return "";
	}
}


std::string VideoWidget::getFriendlyName(VideoWidget::Widget type) {
	switch (type) {
	case VideoWidget::WidgetAverageSpeed:
		return "Average speed";
	case VideoWidget::WidgetAverageRideSpeed:
		return "Average ride speed";
	case VideoWidget::WidgetBatteryLevel:
		return "Battery level";
	case VideoWidget::WidgetCadence:
		return "Cadence";
	case VideoWidget::WidgetDate:
		return "Date";
	case VideoWidget::WidgetDistance:
		return "Distance";
	case VideoWidget::WidgetDuration:
		return "Duration";
	case VideoWidget::WidgetElevation:
		return "Elevation";
	case VideoWidget::WidgetGForce:
		return "GForce";
	case VideoWidget::WidgetGPX:
		return "GPX";
	case VideoWidget::WidgetGrade:
		return "Grade / Slope";
	case VideoWidget::WidgetHeading:
		return "Heading";
	case VideoWidget::WidgetHeartRate:
		return "Heart rate";
	case VideoWidget::WidgetHomeDistance:
		return "Home distance";
	case VideoWidget::WidgetImage:
		return "Image";
	case VideoWidget::WidgetLap:
		return "Lap";
	case VideoWidget::WidgetMap:
		return "Map";
	case VideoWidget::WidgetMaxSpeed:
		return "Maximum speed";
	case VideoWidget::WidgetPosition:
		return "Position";
	case VideoWidget::WidgetPower:
		return "Power";
	case VideoWidget::WidgetSpeed:
		return "Speed";
	case VideoWidget::WidgetTemperature:
		return "Temperature";
	case VideoWidget::WidgetText:
		return "Text";
	case VideoWidget::WidgetTime:
		return "Time";
	case VideoWidget::WidgetTrack:
		return "Track";
	case VideoWidget::WidgetVerticalSpeed:
		return "Vertical speed";
	default:
		return "";
	}
}


void VideoWidget::dump(void) {
	log_call();

	printf("[%s] x: %d / y: %d - size: %dx%d\n", 
			name().c_str(), x(), y(), theme().width(), theme().height());
}

