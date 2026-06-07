#include <iomanip>
#include <iostream>
#include <memory>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "log_i.h"
#include "utils.h"
#include "i18n.h"
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

	setIcon(VideoWidget::Theme::IconDefault);
	setIconColor(1.0, 1.0, 1.0, 1.0);

	setCursorColor(0.8, 0.0, 0.0, 0.8);

	setLabelFontFamily("Sans");
	setLabelFontSize(15);
	setLabelFontStyle(VideoWidget::Theme::FontStyleNormal);
	setLabelFontWeight(VideoWidget::Theme::FontWeightNormal);
	setLabelShadowOpacity(80);
	setLabelShadowDistance(3);
	setLabelHorizontalAlign(VideoWidget::Theme::AlignLeft);
	setLabelVerticalAlign(VideoWidget::Theme::AlignTop);
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
	setValueHorizontalAlign(VideoWidget::Theme::AlignLeft);
	setValueVerticalAlign(VideoWidget::Theme::AlignBottom);
	setValueColor(1.0, 1.0, 1.0, 1.0);
	setValueBorderWidth(1);
	setValueBorderColor(0.0, 0.0, 0.0, 1.0);
	setValueBackgroundColor(0.0, 0.0, 0.0, 0.8);

	setUnitFontSize(25);

	setLineSpace(10);

	setGaugeAngle(300);
	setGaugeRotation(0);
	setGaugeFlip(false);
	setGaugeWidth(10);
	setGaugeCap(VideoWidget::Theme::GaugeCapSquare);
	setGaugeBorder(0);
	setGaugeBorderColor(1.0, 1.0, 1.0, 1.0);
	setGaugeBackgroundColor(0.0, 0.0, 0.0, 0.0);
	setGaugePrimaryColor(0.0, 0.8, 0.0, 0.8);
	setGaugeSecondaryColor(1.0, 0.0, 0.0, 1.0);

	setTickSize(10);
	setTickAlign(VideoWidget::Theme::AlignCenter);
	setTickColor(1.0, 1.0, 1.0, 1.0);
	setTickLabelDistance(5);
	setTickLabelFontSize(10);
	setTickLabelColor(1.0, 1.0, 1.0, 1.0);
	setTickLabelBorderColor(0.0, 0.0, 0.0, 1.0);

	setNeedleType(VideoWidget::Theme::NeedleTypeBasic);
	setNeedlePrimaryColor(1.0, 1.0, 1.0, 1.0);
	setNeedleSecondaryColor(1.0, 0.0, 0.0, 1.0);

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

	if (s.empty() || (s == "none"))
		align = VideoWidget::Theme::AlignNone;
	else if (s == "left")
		align = VideoWidget::Theme::AlignLeft;
	else if (s == "right")
		align = VideoWidget::Theme::AlignRight;
	else if (s == "center")
		align = VideoWidget::Theme::AlignCenter;
	else if (s == "bottom")
		align = VideoWidget::Theme::AlignBottom;
	else if (s == "top")
		align = VideoWidget::Theme::AlignTop;
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


VideoWidget::Theme::GaugeCap VideoWidget::string2gaugecap(std::string &s) {
	VideoWidget::Theme::GaugeCap cap;

	if (s.empty() || (s == "square"))
		cap = VideoWidget::Theme::GaugeCapSquare;
	else if (s == "round")
		cap = VideoWidget::Theme::GaugeCapRound;
	else
		cap = VideoWidget::Theme::GaugeCapUnknown;

	return cap;
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


TelemetryData::Unit VideoWidget::string2unit(std::string &s) {
	TelemetryData::Unit unit;

	if (s.empty() || (s == "none"))
		unit = TelemetryData::UnitNone;
	else if ((s == "mps") || (s == "meterpersec"))
		unit = TelemetryData::UnitMeterPerSec;
	else if (s == "milespersec")
		unit = TelemetryData::UnitMilesPerSec;
	else if ((s == "ftps") || (s == "feetpersec"))
		unit = TelemetryData::UnitFeetPerSec;
	else if (s == "meterpermin")
		unit = TelemetryData::UnitMeterPerMin;
	else if (s == "milespermin")
		unit = TelemetryData::UnitMilesPerMin;
	else if ((s == "ftpmin") || (s == "feetpermin"))
		unit = TelemetryData::UnitFeetPerMin;
	else if (s == "meterperhour")
		unit = TelemetryData::UnitMeterPerHour;
	else if ((s == "mph") || (s == "milesperhour"))
		unit = TelemetryData::UnitMilesPerHour;
	else if ((s == "kph") || (s == "kmperhour"))
		unit = TelemetryData::UnitKmPerHour;
	else if ((s == "ftph") || (s == "feetperhour"))
		unit = TelemetryData::UnitFeetPerHour;
	else if ((s == "mpm") || (s == "minpermile"))
		unit = TelemetryData::UnitMinPerMile;
	else if ((s == "mpk") || (s == "minperkm"))
		unit = TelemetryData::UnitMinPerKm;
	else if (s == "km")
		unit = TelemetryData::UnitKm;
	else if ((s == "m") || (s == "meter"))
		unit = TelemetryData::UnitMeter;
	else if ((s == "ft") || (s == "feet"))
		unit = TelemetryData::UnitFeet;
	else if (s == "miles")
		unit = TelemetryData::UnitMiles;
	else if ((s == "C") || (s == "celsius"))
		unit = TelemetryData::UnitCelsius;
	else if ((s == "F") || (s == "farenheit"))
		unit = TelemetryData::UnitFarenheit;
	else if ((s == "g") || (s == "G"))
		unit = TelemetryData::UnitG;
	else if (s == "meterpersec2")
		unit = TelemetryData::UnitMeterPerSec2;
	else if (s == "bpm")
		unit = TelemetryData::UnitBPM;
	else if (s == "trpermin")
		unit = TelemetryData::UnitTrPerMin;
	else if (s == "watt")
		unit = TelemetryData::UnitWatt;
	else
		unit = TelemetryData::UnitUnknown;

	return unit;
}


VideoWidget::Theme::Icon VideoWidget::string2icon(std::string &s) {
	VideoWidget::Theme::Icon icon;

	if (s.empty() || (s == "default"))
		icon = VideoWidget::Theme::IconDefault;
	else if (s == "running")
		icon = VideoWidget::Theme::IconRunning;
	else if (s == "avgspeed")
		icon = VideoWidget::Theme::IconAverageSpeed;
	else if (s == "avgridespeed")
		icon = VideoWidget::Theme::IconAverageRideSpeed;
	else if (s == "batterylevel")
		icon = VideoWidget::Theme::IconBatteryLevel;
	else if (s == "cadence")
		icon = VideoWidget::Theme::IconCadence;
	else if (s == "date")
		icon = VideoWidget::Theme::IconDate;
	else if (s == "distance")
		icon = VideoWidget::Theme::IconDistance;
	else if (s == "duration")
		icon = VideoWidget::Theme::IconDuration;
	else if (s == "elevation")
		icon = VideoWidget::Theme::IconElevation;
	else if (s == "gforce")
		icon = VideoWidget::Theme::IconGForce;
	else if (s == "gpx")
		icon = VideoWidget::Theme::IconGPX;
	else if (s == "grade")
		icon = VideoWidget::Theme::IconGrade;
	else if (s == "heading")
		icon = VideoWidget::Theme::IconHeading;
	else if (s == "heartrate")
		icon = VideoWidget::Theme::IconHeartRate;
	else if (s == "homedistance")
		icon = VideoWidget::Theme::IconHomeDistance;
	else if (s == "image")
		icon = VideoWidget::Theme::IconImage;
	else if (s == "lap")
		icon = VideoWidget::Theme::IconLap;
	else if (s == "map")
		icon = VideoWidget::Theme::IconMap;
	else if (s == "maxspeed")
		icon = VideoWidget::Theme::IconMaxSpeed;
	else if (s == "position")
		icon = VideoWidget::Theme::IconPosition;
	else if (s == "power")
		icon = VideoWidget::Theme::IconPower;
	else if (s == "speed")
		icon = VideoWidget::Theme::IconSpeed;
	else if (s == "temperature")
		icon = VideoWidget::Theme::IconTemperature;
	else if (s == "text")
		icon = VideoWidget::Theme::IconText;
	else if (s == "time")
		icon = VideoWidget::Theme::IconTime;
	else if (s == "track")
		icon = VideoWidget::Theme::IconTrack;
	else if (s == "verticalspeed")
		icon = VideoWidget::Theme::IconVerticalSpeed;
	else
		icon = VideoWidget::Theme::IconUnknown;

	return icon;
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


std::string VideoWidget::shape2string(VideoWidget::Shape shape) {
	switch (shape) {
	case VideoWidget::ShapeArc:
		return "arc";
	case VideoWidget::ShapeBar:
		return "bar";
	case VideoWidget::ShapeText:
		return "text";
	default:
		return "";
	}
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
	case Theme::AlignTop:
		return "top";
	case Theme::AlignCenter:
		return "center";
	case Theme::AlignRight:
		return "right";
	case Theme::AlignBottom:
		return "bottom";
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

std::string VideoWidget::gaugecap2string(Theme::GaugeCap cap) {
	switch (cap) {
	case Theme::GaugeCapSquare:
		return "square";
	case Theme::GaugeCapRound:
		return "round";
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


std::string VideoWidget::unit2string(TelemetryData::Unit unit) {
	switch (unit) {
	case TelemetryData::UnitMeterPerSec:
		return "meterpersec";
	case TelemetryData::UnitMilesPerSec:
		return "milespersec";
	case TelemetryData::UnitFeetPerSec:
		return "feetpersec";
	case TelemetryData::UnitMeterPerMin:
		return "meterpermin";
	case TelemetryData::UnitMilesPerMin:
		return "milespermin";
	case TelemetryData::UnitFeetPerMin:
		return "feetpermin";
	case TelemetryData::UnitMeterPerHour:
		return "meterperhour";
	case TelemetryData::UnitMilesPerHour:
		return "milesperhour";
	case TelemetryData::UnitKmPerHour:
		return "kmperhour";
	case TelemetryData::UnitFeetPerHour:
		return "feetperhour";
	case TelemetryData::UnitMinPerMile:
		return "minpermile";
	case TelemetryData::UnitMinPerKm:
		return "minperkm";
	case TelemetryData::UnitKm:
		return "km";
	case TelemetryData::UnitMeter:
		return "meter";
	case TelemetryData::UnitFeet:
		return "ft";
	case TelemetryData::UnitMiles:
		return "miles";
	case TelemetryData::UnitCelsius:
		return "celsius";
	case TelemetryData::UnitFarenheit:
		return "farenheit";
	case TelemetryData::UnitG:
		return "g";
	case TelemetryData::UnitMeterPerSec2:
		return "meterpersec2";
	case TelemetryData::UnitBPM:
		return "bpm";
	case TelemetryData::UnitTrPerMin:
		return "trpermin";
	case TelemetryData::UnitWatt:
		return "watt";
	case TelemetryData::UnitNone:
	case TelemetryData::UnitUnknown:
	default:
		return "";
	}
}


std::string VideoWidget::icon2string(VideoWidget::Theme::Icon icon) {
	switch (icon) {
	case VideoWidget::Theme::IconAverageSpeed:
		return "avgspeed";
	case VideoWidget::Theme::IconAverageRideSpeed:
		return "avgridespeed";
	case VideoWidget::Theme::IconBatteryLevel:
		return "batterylevel";
	case VideoWidget::Theme::IconCadence:
		return "cadence";
	case VideoWidget::Theme::IconDate:
		return "date";
	case VideoWidget::Theme::IconDistance:
		return "distance";
	case VideoWidget::Theme::IconDuration:
		return "duration";
	case VideoWidget::Theme::IconElevation:
		return "elevation";
	case VideoWidget::Theme::IconGForce:
		return "gforce";
	case VideoWidget::Theme::IconGPX:
		return "gpx";
	case VideoWidget::Theme::IconGrade:
		return "grade";
	case VideoWidget::Theme::IconHeading:
		return "heading";
	case VideoWidget::Theme::IconHeartRate:
		return "heartrate";
	case VideoWidget::Theme::IconHomeDistance:
		return "homedistance";
	case VideoWidget::Theme::IconImage:
		return "image";
	case VideoWidget::Theme::IconLap:
		return "lap";
	case VideoWidget::Theme::IconMap:
		return "map";
	case VideoWidget::Theme::IconMaxSpeed:
		return "maxspeed";
	case VideoWidget::Theme::IconPosition:
		return "position";
	case VideoWidget::Theme::IconPower:
		return "power";
	case VideoWidget::Theme::IconSpeed:
		return "speed";
	case VideoWidget::Theme::IconTemperature:
		return "temperature";
	case VideoWidget::Theme::IconText:
		return "text";
	case VideoWidget::Theme::IconTime:
		return "time";
	case VideoWidget::Theme::IconTrack:
		return "track";
	case VideoWidget::Theme::IconVerticalSpeed:
		return "verticalspeed";

	case VideoWidget::Theme::IconDefault:
		return "default";
	case VideoWidget::Theme::IconRunning:
		return "running";

	default:
		return "";
	}
}


void VideoWidget::save(std::ostream &os) {
	log_call();

	os.imbue(std::locale::classic());

	xmlopen(os);

	{
		IndentingOStreambuf indent(os, 4);

		xmlwrite(os);
	}

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

	os << "<type>" << name() << "</type>" << std::endl;
	os << "<name>" << label() << "</name>" << std::endl;
	os << "<margin-left>" << margin(Margin::MarginLeft) << "</margin-left>" << std::endl;
	os << "<margin-right>" << margin(Margin::MarginRight) << "</margin-right>" << std::endl;
	os << "<margin-top>" << margin(Margin::MarginTop) << "</margin-top>" << std::endl;
	os << "<margin-bottom>" << margin(Margin::MarginBottom) << "</margin-bottom>" << std::endl;
}


std::string VideoWidget::getIconFilename(VideoWidget::Widget type) {
	std::string path = GPXApplication::assets("icons");

	switch (type) {
	case VideoWidget::WidgetAverageSpeed:
	case VideoWidget::WidgetAverageRideSpeed:
		return path + "/avgspeed.svg";
	case VideoWidget::WidgetBatteryLevel:
		return path + "/battery.svg";
	case VideoWidget::WidgetCadence:
		return path + "/cadence.svg";
	case VideoWidget::WidgetDate:
		return path + "/date.svg";
	case VideoWidget::WidgetDistance:
		return path + "/distance.svg";
	case VideoWidget::WidgetDuration:
		return path + "/duration.svg";
	case VideoWidget::WidgetElevation:
		return path + "/elevation.svg";
	case VideoWidget::WidgetGForce:
		return path + "/gforce.svg";
	case VideoWidget::WidgetGPX:
		return path + "/position.svg";
	case VideoWidget::WidgetGrade:
		return path + "/grade.svg";
	case VideoWidget::WidgetHeading:
		return path + "/compass.svg";
	case VideoWidget::WidgetHeartRate:
		return path + "/heartrate.svg";
	case VideoWidget::WidgetHomeDistance:
		return path + "/home.svg";
	case VideoWidget::WidgetImage:
		return path + "/image.svg";
	case VideoWidget::WidgetLap:
		return path + "/laps.svg";
	case VideoWidget::WidgetMap:
		return path + "/map.svg";
	case VideoWidget::WidgetMaxSpeed:
		return path + "/maxspeed.svg";
	case VideoWidget::WidgetPosition:
		return path + "/position.svg";
	case VideoWidget::WidgetPower:
		return path + "/power.svg";
	case VideoWidget::WidgetSpeed:
		return path + "/speed.svg";
	case VideoWidget::WidgetTemperature:
		return path + "/temperature.svg";
	case VideoWidget::WidgetText:
		return path + "/text.svg";
	case VideoWidget::WidgetTime:
		return path + "/time.svg";
	case VideoWidget::WidgetTrack:
		return path + "/track.svg";
	case VideoWidget::WidgetVerticalSpeed:
		return path + "/altitude.svg";
	default:
		return "";
	}
}


std::string VideoWidget::getIconFilename(VideoWidget::Theme::Icon icon) {
	log_call();

	std::string path = GPXApplication::assets("icons");

	if (icon == VideoWidget::Theme::IconDefault)
		return VideoWidget::getIconFilename(this->type());
	else if (icon < VideoWidget::Theme::IconDefault)
		return VideoWidget::getIconFilename((VideoWidget::Widget) icon);

	switch (icon) {
	case VideoWidget::Theme::IconRunning:
		return path + "/running.svg";
	default:
		return "";
	}
}


std::string VideoWidget::getFriendlyName(VideoWidget::Shape shape) {
	switch (shape) {
	case VideoWidget::ShapeArc:
		return _("Arc");
	case VideoWidget::ShapeBar:
		return _("Bar");
	case VideoWidget::ShapeText:
		return _("Text");
	default:
		return "";
	}
}

std::string VideoWidget::getFriendlyName(VideoWidget::Widget type) {
	switch (type) {
	case VideoWidget::WidgetAverageSpeed:
		return _("Average speed");
	case VideoWidget::WidgetAverageRideSpeed:
		return _("Average ride speed");
	case VideoWidget::WidgetBatteryLevel:
		return _("Battery level");
	case VideoWidget::WidgetCadence:
		return _("Cadence");
	case VideoWidget::WidgetDate:
		return _("Date");
	case VideoWidget::WidgetDistance:
		return _("Distance");
	case VideoWidget::WidgetDuration:
		return _("Duration");
	case VideoWidget::WidgetElevation:
		return _("Elevation");
	case VideoWidget::WidgetGForce:
		return _("GForce");
	case VideoWidget::WidgetGPX:
		return _("GPX");
	case VideoWidget::WidgetGrade:
		return _("Grade / Slope");
	case VideoWidget::WidgetHeading:
		return _("Heading");
	case VideoWidget::WidgetHeartRate:
		return _("Heart rate");
	case VideoWidget::WidgetHomeDistance:
		return _("Home distance");
	case VideoWidget::WidgetImage:
		return _("Image");
	case VideoWidget::WidgetLap:
		return _("Lap");
	case VideoWidget::WidgetMap:
		return _("Map");
	case VideoWidget::WidgetMaxSpeed:
		return _("Maximum speed");
	case VideoWidget::WidgetPosition:
		return _("Position");
	case VideoWidget::WidgetPower:
		return _("Power");
	case VideoWidget::WidgetSpeed:
		return _("Speed");
	case VideoWidget::WidgetTemperature:
		return _("Temperature");
	case VideoWidget::WidgetText:
		return _("Text");
	case VideoWidget::WidgetTime:
		return _("Time");
	case VideoWidget::WidgetTrack:
		return _("Track");
	case VideoWidget::WidgetVerticalSpeed:
		return _("Vertical speed");
	default:
		return "";
	}
}


std::string VideoWidget::getFriendlyName(TelemetryData::Unit unit) {
	switch (unit) {
	case TelemetryData::UnitMeterPerSec:
		return "m/s";
	case TelemetryData::UnitMilesPerSec:
		return "miles/s";
	case TelemetryData::UnitFeetPerSec:
		return "ft/s";
	case TelemetryData::UnitMeterPerMin:
		return "m/min";
	case TelemetryData::UnitMilesPerMin:
		return "miles/min";
	case TelemetryData::UnitFeetPerMin:
		return "ft/min";
	case TelemetryData::UnitMeterPerHour:
		return "m/h";
	case TelemetryData::UnitMilesPerHour:
		return "miles/h";
	case TelemetryData::UnitKmPerHour:
		return "km/h";
	case TelemetryData::UnitFeetPerHour:
		return "ft/h";
	case TelemetryData::UnitMinPerMile:
		return "min/mile";
	case TelemetryData::UnitMinPerKm:
		return "min/km";
	case TelemetryData::UnitKm:
		return "km";
	case TelemetryData::UnitMeter:
		return "m";
	case TelemetryData::UnitFeet:
		return "ft";
	case TelemetryData::UnitMiles:
		return "miles";
	case TelemetryData::UnitCelsius:
		return "°C";
	case TelemetryData::UnitFarenheit:
		return "F";
	case TelemetryData::UnitG:
		return "g";
	case TelemetryData::UnitMeterPerSec2:
		return "m/s²";
	case TelemetryData::UnitBPM:
		return "bpm";
	case TelemetryData::UnitTrPerMin:
		return "tr/min";
	case TelemetryData::UnitWatt:
		return "Watt";
	case TelemetryData::UnitNone:
	case TelemetryData::UnitUnknown:
	default:
		return "";
	}
}


void VideoWidget::dump(void) {
	log_call();

	printf("[%s] x: %d / y: %d - size: %dx%d\n", 
			name().c_str(), x(), y(), theme().width(), theme().height());
}

