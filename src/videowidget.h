#ifndef __GPX2VIDEO__VIDEOWIDGET_H__
#define __GPX2VIDEO__VIDEOWIDGET_H__

#include <iostream>
#include <memory>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <list>

#include <cairo.h>
#include <OpenImageIO/imagebuf.h>

#include "telemetrymedia.h"
#include "application.h"


class ShapeBase;

class VideoWidget : public GPXApplication::Task {
public:
	enum Margin {
		MarginAll,
		MarginLeft,
		MarginRight,
		MarginBottom,
		MarginTop
	};

	enum Orientation {
		OrientationNone = 0,
		OrientationHorizontal,
		OrientationVertical,
		OrientationUnknown
	};

	enum Position {
		PositionNone = 0,
		PositionLeft,
		PositionRight,
		PositionBottom,
		PositionTop,
		PositionBottomLeft,
		PositionBottomRight,
		PositionTopLeft,
		PositionTopRight,
		PositionUnknown
	};

	enum Shape {
		ShapeNone = 0,
		ShapeText,
		ShapeArc,
		ShapeBar,
		ShapeChart,
		ShapeUnknown
	};

	enum Widget {
		WidgetAverageSpeed,
		WidgetAverageRideSpeed,
		WidgetBatteryLevel,
		WidgetCadence,
		WidgetDate,
		WidgetDistance,
		WidgetDuration,
		WidgetElevation,
		WidgetGForce,
		WidgetGPX,
		WidgetGrade,
		WidgetCourse,
		WidgetHeading,
		WidgetHeartRate,
		WidgetHomeDistance,
		WidgetImage,
		WidgetLap,
		WidgetMap,
		WidgetMaxSpeed,
		WidgetPosition,
		WidgetPower,
		WidgetSpeed,
		WidgetTemperature,
		WidgetText,
		WidgetTime,
		WidgetTrack,
		WidgetVerticalSpeed,
		WidgetUnknown
	};

	enum Zoom {
		ZoomNone,
		ZoomFit,
		ZoomFill,
		ZoomCrop,
		ZoomStretch,
		ZoomUnknown
	};

	struct ListItem {
		int id;
		std::string name;
		std::string fmt;
	};

	virtual ~VideoWidget() {
	}

	class Theme {
	public:
		enum Align {
			AlignNone = -1,
			AlignLeft = 0, // PANGO_ALIGN_LEFT
			AlignCenter = 1, // PANGO_ALIGN_CENTER
			AlignRight = 2, // PANGO_ALIGN_RIGHT
			AlignTop = 3,
			AlignBottom = 4,
			AlignUnknown = 5
		};

		enum Padding {
			PaddingAll,
			PaddingLeft,
			PaddingRight,
			PaddingBottom,
			PaddingTop
		};

		enum Flag {
			FlagNone = 0,
			FlagLabel = (1 << 0),
			FlagValue = (1 << 1),
			FlagIcon = (1 << 2),
			FlagUnit = (1 << 3),
			FlagNeedle = (1 << 4),
			FlagTick = (1 << 5),
			FlagTickLabel = (1 << 6),
			FlagCursor = (1 << 7),
			FlagGauge = (1 << 8),
			FlagIconStart = (1 << 9),
			FlagIconEnd = (1 << 10),
			FlagIconPosition = (1 << 11),
			FlagAxis = (1 << 12),
			FlagUnknown = (1 << 13),
			FlagAll = ~0x0
		};

		enum FontStyle {
			FontStyleNormal,
			FontStyleOblique,
			FontStyleItalic,
			FontStyleUnknown
		};

		enum FontWeight {
			FontWeightThin = 100,
			FontWeightUltraLight = 200,
			FontWeightLight = 300,
			FontWeightSemiLight = 350,
			FontWeightBook = 380,
			FontWeightNormal = 400,
			FontWeightMedium = 500,
			FontWeightSemiBold = 600,
			FontWeightBold = 700,
			FontWeightUltraBold = 800,
			FontWeightHeavy = 900,
			FontWeightUltraHeavy = 1000,
			FontWeightUnknown
		};

		enum Icon {
			IconDefault = -1,

			// Begin match VideoWidget::Type
			IconAverageSpeed = 0,
			IconAverageRideSpeed,
			IconBatteryLevel,
			IconCadence,
			IconDate,
			IconDistance,
			IconDuration,
			IconElevation,
			IconGForce,
			IconGPX,
			IconGrade,
			IconCourse,
			IconHeading,
			IconHeartRate,
			IconHomeDistance,
			IconImage,
			IconLap,
			IconMap,
			IconMaxSpeed,
			IconPosition,
			IconPower,
			IconSpeed,
			IconTemperature,
			IconText,
			IconTime,
			IconTrack,
			IconVerticalSpeed,
			// End match VideoWidget::Type

			// Other internal icons
			IconSpot,
			IconRunning,
			IconBike,

			// At last user icons
			IconUserFile,

			IconUnknown
		};

		enum GaugeCap {
			GaugeCapSquare,
			GaugeCapRound,
			GaugeCapUnknown
		};

		enum NeedleType {
			NeedleTypeThin,
			NeedleTypeLight,
			NeedleTypeBasic,
			NeedleTypeDesign,
			NeedleTypeIcon,
			NeedleTypeValue,
			NeedleTypeUnknown
		};

		Theme();

		void setSize(int width, int height);
		const int& width(void) const;
		const int& height(void) const;

		void setFlags(int flags);
		void addFlag(VideoWidget::Theme::Flag flag);
		void removeFlag(VideoWidget::Theme::Flag flag);
		bool hasFlag(VideoWidget::Theme::Flag flag);

		const int& padding(enum Padding side) const;
		virtual void setPadding(enum Padding side, int padding);

		const float * backgroundColor(void) const;
		bool setBackgroundColor(std::string color);
		bool setBackgroundColor(double r, double g, double b, double a);

		double border(void) const;
		void setBorder(double border);

		const float * borderColor(void) const;
		bool setBorderColor(std::string color);
		bool setBorderColor(double r, double g, double b, double a);

		double roundCorner(void) const;
		void setRoundCorner(double size);

		const Icon& icon(void) const;
		void setIcon(const Icon &icon);

		const std::string& iconFile(void) const;
		void setIconFile(const std::string &file);

		const double& iconSize(void) const;
		void setIconSize(const double &size);

		const float * iconColor(void) const;
		bool setIconColor(std::string color);
		bool setIconColor(double r, double g, double b, double a);

		enum Align labelHorizontalAlign(void) const;
		virtual void setLabelHorizontalAlign(Align align);

		enum Align labelVerticalAlign(void) const;
		virtual void setLabelVerticalAlign(Align align);

		const std::string& labelFontFamily(void) const;
		virtual void setLabelFontFamily(std::string family);

		double labelFontSize(void) const;
		void setLabelFontSize(double size);

		enum FontStyle labelFontStyle(void) const;
		void setLabelFontStyle(FontStyle style);

		enum FontWeight labelFontWeight(void) const;
		void setLabelFontWeight(FontWeight weight);

		const float * labelColor(void) const;
		bool setLabelColor(std::string color);
		bool setLabelColor(double r, double g, double b, double a);

		double labelShadowDistance(void) const;
		void setLabelShadowDistance(double distance);

		int labelShadowOpacity(void) const;
		void setLabelShadowOpacity(int opacity);

		double labelBorderWidth(void) const;
		void setLabelBorderWidth(double width);

		const float * labelBorderColor(void) const;
		bool setLabelBorderColor(std::string color);
		bool setLabelBorderColor(double r, double g, double b, double a);

		const int& valueMin(void) const;
		void setValueMin(int value);

		const int& valueMax(void) const;
		void setValueMax(int value);

		enum Align valueHorizontalAlign(void) const;
		virtual void setValueHorizontalAlign(Align align);

		enum Align valueVerticalAlign(void) const;
		virtual void setValueVerticalAlign(Align align);

		const std::string& valueFontFamily(void) const;
		virtual void setValueFontFamily(std::string family);

		double valueFontSize(void) const;
		void setValueFontSize(double size);

		enum FontStyle valueFontStyle(void) const;
		void setValueFontStyle(FontStyle style);

		enum FontWeight valueFontWeight(void) const;
		void setValueFontWeight(FontWeight weight);

		const float * valueColor(void) const;
		bool setValueColor(std::string color);
		bool setValueColor(double r, double g, double b, double a);

		double valueShadowDistance(void) const;
		void setValueShadowDistance(double distance);

		int valueShadowOpacity(void) const;
		void setValueShadowOpacity(int opacity);

		const double& valueBorderWidth(void) const;
		void setValueBorderWidth(double width);

		const float * valueBorderColor(void) const;
		bool setValueBorderColor(std::string color);
		bool setValueBorderColor(double r, double g, double b, double a);

		double unitFontSize(void) const;
		void setUnitFontSize(double size);

		double unitDistance(void) const;
		void setUnitDistance(double size);

		double lineSpace(void) const;
		void setLineSpace(double size);

		const VideoWidget::Orientation& textOrientation(void) const;
		void setTextOrientation(VideoWidget::Orientation orientation);

		const int& gaugeAngle(void) const;
		void setGaugeAngle(int angle);

		const int& gaugeRotation(void) const;
		void setGaugeRotation(int rotation);

		const VideoWidget::Orientation& gaugeOrientation(void) const;
		void setGaugeOrientation(VideoWidget::Orientation orientation);

		const bool& gaugeFlip(void) const;
		void setGaugeFlip(bool flip);

		const double& gaugeWidth(void) const;
		void setGaugeWidth(double width);

		const double& gaugeOffset(void) const;
		void setGaugeOffset(double offset);

		GaugeCap gaugeCap(void) const;
		void setGaugeCap(GaugeCap cap);

		const double& gaugeBorder(void) const;
		void setGaugeBorder(double border);

		const float * gaugeBorderColor(void) const;
		bool setGaugeBorderColor(std::string color);
		bool setGaugeBorderColor(double r, double g, double b, double a);

		const float * gaugePrimaryColor() const;
		bool setGaugePrimaryColor(std::string color);
		bool setGaugePrimaryColor(double r, double g, double b, double a);

		const float * gaugeSecondaryColor() const;
		bool setGaugeSecondaryColor(std::string color);
		bool setGaugeSecondaryColor(double r, double g, double b, double a);

		const float * gaugeBackgroundColor(void) const;
		bool setGaugeBackgroundColor(std::string color);
		bool setGaugeBackgroundColor(double r, double g, double b, double a);

		const double& tickSize(void) const;
		void setTickSize(double size);

		const Align& tickAlign(void) const;
		void setTickAlign(Align align);

		const float * tickColor(void) const;
		bool setTickColor(std::string color);
		bool setTickColor(double r, double g, double b, double a);

		const double& tickLabelDistance(void) const;
		void setTickLabelDistance(double distance);

		double tickLabelFontSize(void) const;
		void setTickLabelFontSize(double size);

		const float * tickLabelColor(void) const;
		bool setTickLabelColor(std::string color);
		bool setTickLabelColor(double r, double g, double b, double a);

		const float * tickLabelBorderColor(void) const;
		bool setTickLabelBorderColor(std::string color);
		bool setTickLabelBorderColor(double r, double g, double b, double a);

		NeedleType needleType(void) const;
		void setNeedleType(NeedleType type);

		const double& needleDistance(void) const;
		void setNeedleDistance(double distance);

		const double& needleBorder(void) const;
		void setNeedleBorder(double border);

		const float * needleBorderColor(void) const;
		bool setNeedleBorderColor(std::string color);
		bool setNeedleBorderColor(double r, double g, double b, double a);

		const float * needleBackgroundColor(void) const;
		bool setNeedleBackgroundColor(std::string color);
		bool setNeedleBackgroundColor(double r, double g, double b, double a);

		const float * needlePrimaryColor(void) const;
		bool setNeedlePrimaryColor(std::string color);
		bool setNeedlePrimaryColor(double r, double g, double b, double a);

		const float * needleSecondaryColor(void) const;
		bool setNeedleSecondaryColor(std::string color);
		bool setNeedleSecondaryColor(double r, double g, double b, double a);

		const double &cursorWidth(void) const;
		void setCursorWidth(double width);

		const float * cursorColor(void) const;
		bool setCursorColor(std::string color);
		bool setCursorColor(double r, double g, double b, double a);

		const double& axisThick(void) const;
		void setAxisThick(double thick);

		const double& axisBorder(void) const;
		void setAxisBorder(double border);

		const float * axisColor(void) const;
		bool setAxisColor(std::string color);
		bool setAxisColor(double r, double g, double b, double a);

		const float * axisBorderColor(void) const;
		bool setAxisBorderColor(std::string color);
		bool setAxisBorderColor(double r, double g, double b, double a);

		const double& curveThick(void) const;
		void setCurveThick(double thick);

		const double& curveBorder(void) const;
		void setCurveBorder(double border);

		const float * curveColor(void) const;
		bool setCurveColor(std::string color);
		bool setCurveColor(double r, double g, double b, double a);

		const float * curveBorderColor(void) const;
		bool setCurveBorderColor(std::string color);
		bool setCurveBorderColor(double r, double g, double b, double a);

		const float * curveFillColor(void) const;
		bool setCurveFillColor(std::string color);
		bool setCurveFillColor(double r, double g, double b, double a);

		static bool hex2color(float color[4], std::string html);
		static std::string color2hex(const float color[4]);

	private:
		const int null_ = 0;

		int width_;
		int height_;

		int flags_;

		int padding_;
		int padding_top_;
		int padding_bottom_;
		int padding_left_;
		int padding_right_;

		float bg_color_[4];

		double border_;
		float border_color_[4];

		double round_corner_;

		double cursor_width_;
		float cursor_color_[4];

		Icon icon_;
		std::string icon_file_;
		double icon_size_;
		float icon_color_[4];

		Align label_horizontal_align_;
		Align label_vertical_align_;
		std::string label_font_family_;
		double label_font_size_;
		FontStyle label_font_style_;
		FontWeight label_font_weight_;
		double label_border_width_;
		int label_shadow_opacity_;
		double label_shadow_distance_;
		float label_color_[4];
		float label_border_color_[4];

		NeedleType needle_type_;
		double needle_distance_;
		double needle_border_;
		float needle_border_color_[4];
		float needle_background_color_[4];
		float needle_primary_color_[4];
		float needle_secondary_color_[4];

		int value_min_;
		int value_max_;
		Align value_horizontal_align_;
		Align value_vertical_align_;
		std::string value_font_family_;
		double value_font_size_;
		FontStyle value_font_style_;
		FontWeight value_font_weight_;
		double value_border_width_;
		int value_shadow_opacity_;
		double value_shadow_distance_;
		float value_color_[4];
		float value_border_color_[4];

		double unit_font_size_;
		double unit_distance_;

		double line_space_;
		VideoWidget::Orientation text_orientation_;

		int gauge_angle_;
		int gauge_rotation_;
		VideoWidget::Orientation gauge_orientation_;
		bool gauge_flip_;
		double gauge_width_;
		double gauge_offset_;
		GaugeCap gauge_cap_;
		double gauge_border_;
		float gauge_border_color_[4];
		float gauge_bg_color_[4];
		float gauge_primary_color_[4];
		float gauge_secondary_color_[4];

		double tick_size_;
		Align tick_align_;
		float tick_color_[4];
		double tick_label_distance_;
		double tick_label_font_size_;
		float tick_label_color_[4];
		float tick_label_border_color_[4];

		double axis_thick_;
		double axis_border_;
		float axis_color_[4];
		float axis_border_color_[4];

		double curve_thick_;
		double curve_border_;
		float curve_color_[4];
		float curve_border_color_[4];
		float curve_fill_color_[4];
	};

	virtual ShapeBase * shape(void) = 0;

	Theme& theme(void) {
		return theme_;
	}

	virtual const std::list<ListItem>& shapes(void) const {
		return shapes_supported_;
	}

	virtual void setShape(Shape shape) {
		shape_ = shape;
	}

	TelemetrySource * telemetrySource(void) const {
		return telemetry_source_;
	}

	void setTelemetrySource(TelemetrySource *source) {
		telemetry_source_ = source;
	}

	const bool& visible(void) const {
		return visible_;
	}

	void setVisible(bool visible) {
		visible_ = visible;
	}

	const uint64_t& timestamp(void) const {
		return timestamp_;
	}

	void setTimestamp(uint64_t timestmap) {
		timestamp_  = timestmap;
	}

	uint64_t& atBeginTime(void) {
		return at_begin_time_;
	}

	uint64_t& atEndTime(void) {
		return at_end_time_;
	}

	virtual void setAtTime(const uint64_t begintime, const uint64_t endtime) {
		at_begin_time_ = begintime;

		if (begintime != endtime)
			at_end_time_ = endtime;
	}

	virtual void setSize(int width, int height) {
		theme().setSize(width, height);
	}

	Position& position(void) {
		return position_;
	}

	virtual void setPosition(Position position) {
		position_ = position;
	}

	Orientation& orientation(void) {
		return orientation_;
	}

	virtual void setOrientation(Orientation orientation) {
		orientation_ = orientation;
	}

	Zoom& zoom(void) {
		return zoom_;
	}

	virtual void setZoom(Zoom zoom) {
		zoom_ = zoom;
	}

	virtual const std::list<ListItem>& units(void) const {
		return units_supported_;
	}

	TelemetryData::Unit& valueUnit(void) {
		return value_unit_;
	}

	virtual void setValueUnit(TelemetryData::Unit unit) {
		value_unit_ = unit;
	}

	virtual const std::list<ListItem>& formats(void) const {
		return formats_supported_;
	}

	const std::string& valueFormat(void) {
		return value_format_;
	}

	virtual void setValueFormat(std::string format) {
		value_format_ = format;
	}

	const std::string& source(void) {
		return source_;
	}

	virtual void setSource(std::string source) {
		source_ = source;
	}

	virtual void setPosition(int x, int y) {
		x_ = x;
		y_ = y;
	}

	const int& x(void) const {
		return x_;
	}

	const int& y(void) const {
		return y_;
	}

	const int& margin(enum Margin side) const {
		switch (side) {
		case MarginLeft:
			return margin_left_;
		case MarginRight:
			return margin_right_;
		case MarginTop:
			return margin_top_;
		case MarginBottom:
			return margin_bottom_;
		default:
			return null_;
		}
	}

	virtual void setMargin(enum Margin side, int margin) {
		if (margin < 0)
			return;

		switch (side) {
		case MarginAll:
			margin_left_ = margin;
			margin_right_ = margin;
			margin_top_ = margin;
			margin_bottom_ = margin;
			break;
		case MarginLeft:
			margin_left_ = margin;
			break;
		case MarginRight:
			margin_right_ = margin;
			break;
		case MarginTop:
			margin_top_ = margin;
			break;
		case MarginBottom:
			margin_bottom_ = margin;
			break;
		default:
			break;
		}
	}

	const Widget& type(void) const {
		return type_;
	}

	const std::string& name(void) {
		return name_;
	}

	const std::string& label(void) const {
		return label_;
	}

	virtual void setLabel(std::string label) {
		label_ = label;
	}

	const std::string& value(void) const {
		return text_;
	}

	virtual void setValue(std::string text) {
		text_ = text;
	}

	virtual bool isStatic(void) {
		return false;
	}

	virtual bool run(void) {
		complete();

		return true;
	}

	void dump(void);

	virtual OIIO::ImageBuf * render(const TelemetryData &data, bool &is_update) = 0;

	virtual bool updated(const TelemetryData &data) const = 0;

	virtual void draw(cairo_t *cairo, const TelemetryData &data) = 0;

	virtual void clear(void) = 0;

	virtual void save(std::ostream &os);

	std::string getIconFilename(Theme::Icon icon);

	static Widget string2widget(std::string &s);
	static Shape string2shape(std::string &s);
	static Position string2position(std::string &s);
	static Orientation string2orientation(std::string &s);
	static Theme::Align string2align(std::string &s);
	static Theme::FontStyle string2fontstyle(std::string &s);
	static Theme::FontWeight string2fontweight(std::string &s);
	static Theme::GaugeCap string2gaugecap(std::string &);
	static Theme::NeedleType string2needletype(std::string &);
	static TelemetryData::Unit string2unit(std::string &s);
	static Theme::Icon string2icon(std::string &s);
	static Zoom string2zoom(std::string &s);

	static std::string bool2string(bool value);
	static std::string shape2string(Shape type);
	static std::string widget2string(Widget type);
	static std::string position2string(Position position);
	static std::string orientation2string(Orientation orientation);
	static std::string align2string(Theme::Align align);
	static std::string fontstyle2string(Theme::FontStyle style);
	static std::string fontweight2string(Theme::FontWeight weight);
	static std::string gaugecap2string(Theme::GaugeCap cap);
	static std::string needletype2string(Theme::NeedleType type);
	static std::string unit2string(TelemetryData::Unit unit);
	static std::string icon2string(Theme::Icon icon);
	static std::string zoom2string(VideoWidget::Zoom zoom);

	static std::string getIconFilename(Widget type);
	static std::string getFriendlyName(Shape shape);
	static std::string getFriendlyName(Widget type);
	static std::string getFriendlyName(TelemetryData::Unit unit);

protected:
	VideoWidget(GPXApplication &app, Widget type, TelemetrySource *source = NULL)  
		: GPXApplication::Task(app, widget2string(type))
		, app_(app) 
		, telemetry_source_(source)
		, at_begin_time_(0)
		, at_end_time_(0)
		, name_(widget2string(type))
		, type_(type) {
		setVisible(true);
		setTimestamp(0);
		setShape(ShapeNone);
		setPosition(PositionNone);
		setOrientation(OrientationNone);
		setAtTime(0, 0);
		setPosition(0, 0);
		setMargin(MarginAll, 10);
		setLabel(widget2string(type));
		setValueUnit(TelemetryData::UnitNone);
		setZoom(ZoomNone);
	}

	virtual void xmlopen(std::ostream &os);
	virtual void xmlwrite(std::ostream &os);
	virtual void xmlclose(std::ostream &os);

	GPXApplication &app_;
	TelemetrySource *telemetry_source_;
	Shape shape_;
	Position position_;
	Orientation orientation_;
	Zoom zoom_;
	TelemetryData::Unit value_unit_;
	std::string value_format_;
	std::string source_;

	const int null_ = 0;

	bool visible_;

	// Used only to store video timestamp for profiling
	uint64_t timestamp_;

	uint64_t at_begin_time_;
	uint64_t at_end_time_;

	int x_;
	int y_;
	int margin_top_;
	int margin_bottom_;
	int margin_left_;
	int margin_right_;

	std::string text_;

	std::string name_;
	std::string label_;

	Theme theme_;

	std::list<ListItem> shapes_supported_;
	std::list<ListItem> units_supported_;
	std::list<ListItem> formats_supported_;

private:
	Widget type_;
};

#endif

