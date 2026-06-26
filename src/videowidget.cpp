#include <iomanip>
#include <iostream>
#include <limits>
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
	// Init
	value_min_ = std::numeric_limits<int>::min();
	value_max_ = std::numeric_limits<int>::max();

	// Default value
	setSize(240, 50);
	setFlags(VideoWidget::Theme::FlagAll);

	setPadding(VideoWidget::Theme::PaddingAll, 3);

	setBorder(0.5);
	setBorderColor(0.0, 0.0, 0.0, 1.0);
	setBackgroundColor(0.0, 0.0, 0.0, 0.8);

	setRoundCorner(0);

	setIcon(VideoWidget::Theme::IconDefault);
	setIconSize(50.0);
	setIconColor(1.0, 1.0, 1.0, 1.0);

	setCursorWidth(1.0);
	setCursorColor(0.8, 0.0, 0.0, 0.8);

	setLabelFontFamily("Sans");
	setLabelFontSize(30.0);
	setLabelFontStyle(VideoWidget::Theme::FontStyleNormal);
	setLabelFontWeight(VideoWidget::Theme::FontWeightNormal);
	setLabelShadowOpacity(80);
	setLabelShadowDistance(1.0);
	setLabelHorizontalAlign(VideoWidget::Theme::AlignLeft);
	setLabelVerticalAlign(VideoWidget::Theme::AlignTop);
	setLabelColor(1.0, 1.0, 1.0, 1.0);
	setLabelBorderWidth(1.0);
	setLabelBorderColor(0.0, 0.0, 0.0, 1.0);

	setValueMin(0);
	setValueMax(100);
	setValueFontFamily("Sans");
	setValueFontSize(50.0);
	setValueFontStyle(VideoWidget::Theme::FontStyleNormal);
	setValueFontWeight(VideoWidget::Theme::FontWeightNormal);
	setValueShadowOpacity(80);
	setValueShadowDistance(1.0);
	setValueHorizontalAlign(VideoWidget::Theme::AlignLeft);
	setValueVerticalAlign(VideoWidget::Theme::AlignBottom);
	setValueColor(1.0, 1.0, 1.0, 1.0);
	setValueBorderWidth(1.0);
	setValueBorderColor(0.0, 0.0, 0.0, 1.0);

	setUnitFontSize(50.0);

	setLineSpace(20.0);

	setGaugeAngle(300);
	setGaugeRotation(0);
	setGaugeFlip(false);
	setGaugeWidth(20.0);
	setGaugeOffset(0.0);
	setGaugeCap(VideoWidget::Theme::GaugeCapSquare);
	setGaugeOrientation(VideoWidget::OrientationVertical);
	setGaugeBorder(0);
	setGaugeBorderColor(1.0, 1.0, 1.0, 1.0);
	setGaugeBackgroundColor(0.0, 0.0, 0.0, 0.0);
	setGaugePrimaryColor(0.0, 0.8, 0.0, 0.8);
	setGaugeSecondaryColor(1.0, 0.0, 0.0, 1.0);

	setTickSize(20.0);
	setTickAlign(VideoWidget::Theme::AlignCenter);
	setTickColor(1.0, 1.0, 1.0, 1.0);
	setTickLabelDistance(20.0);
	setTickLabelFontSize(20.0);
	setTickLabelColor(1.0, 1.0, 1.0, 1.0);
	setTickLabelBorderColor(0.0, 0.0, 0.0, 1.0);

	setNeedleType(VideoWidget::Theme::NeedleTypeBasic);
	setNeedleDistance(0);
	setNeedleBorder(0);
	setNeedleBorderColor(1.0, 1.0, 1.0, 1.0);
	setNeedleBackgroundColor(0.0, 0.0, 0.0, 0.8);
	setNeedlePrimaryColor(1.0, 1.0, 1.0, 1.0);
	setNeedleSecondaryColor(1.0, 0.0, 0.0, 1.0);

	setAxisThick(1.0);
	setAxisBorder(0.2);
	setAxisColor(1.0, 1.0, 1.0, 1.0);
	setAxisBorderColor(0.0, 0.0, 0.0, 1.0);

	setCurveThick(1.0);
	setCurveBorder(0.2);
	setCurveColor(1.0, 1.0, 1.0, 1.0);
	setCurveBorderColor(0.0, 0.0, 0.0, 1.0);
	setCurveFillColor(0.3, 0.3, 0.3, 1.0);
}


void VideoWidget::Theme::setSize(int width, int height) {
	width_ = width;
	height_ = height;
}

const int& VideoWidget::Theme::width(void) const {
	return width_;
}

const int& VideoWidget::Theme::height(void) const {
	return height_;
}

void VideoWidget::Theme::setFlags(int flags) {
	flags_ = flags;
}

void VideoWidget::Theme::addFlag(VideoWidget::Theme::Flag flag) {
	flags_ |= flag;
}

void VideoWidget::Theme::removeFlag(VideoWidget::Theme::Flag flag) {
	flags_ &= ~flag;
}

bool VideoWidget::Theme::hasFlag(VideoWidget::Theme::Flag flag) {
	return ((flags_ & flag) != 0);
}

const int& VideoWidget::Theme::padding(enum Padding side) const {
	switch (side) {
	case PaddingLeft:
		return padding_left_;
	case PaddingRight:
		return padding_right_;
	case PaddingTop:
		return padding_top_;
	case PaddingBottom:
		return padding_bottom_;
	default:
		return null_;
	}
}

void VideoWidget::Theme::setPadding(enum Padding side, int padding) {
	if (padding < 0)
		return;

	switch (side) {
	case PaddingAll:
		padding_left_ = padding;
		padding_right_ = padding;
		padding_top_ = padding;
		padding_bottom_ = padding;
		break;
	case PaddingLeft:
		padding_left_ = padding;
		break;
	case PaddingRight:
		padding_right_ = padding;
		break;
	case PaddingTop:
		padding_top_ = padding;
		break;
	case PaddingBottom:
		padding_bottom_ = padding;
		break;
	default:
		break;
	}
}

const float * VideoWidget::Theme::backgroundColor(void) const {
	return bg_color_;
}

bool VideoWidget::Theme::setBackgroundColor(std::string color) {
	return hex2color(bg_color_, color);
}

bool VideoWidget::Theme::setBackgroundColor(double r, double g, double b, double a) {
	bg_color_[0] = r;
	bg_color_[1] = g;
	bg_color_[2] = b;
	bg_color_[3] = a;
	return true;
}

double VideoWidget::Theme::border(void) const {
	return border_;
}

void VideoWidget::Theme::setBorder(double border) {
	border_ = border;
}

const float * VideoWidget::Theme::borderColor(void) const {
	return border_color_;
}

bool VideoWidget::Theme::setBorderColor(std::string color) {
	return hex2color(border_color_, color);
}

bool VideoWidget::Theme::setBorderColor(double r, double g, double b, double a) {
	border_color_[0] = r;
	border_color_[1] = g;
	border_color_[2] = b;
	border_color_[3] = a;
	return true;
}

double VideoWidget::Theme::roundCorner(void) const {
	return round_corner_;
}

void VideoWidget::Theme::setRoundCorner(double size) {
	round_corner_ = size;
}

const VideoWidget::Theme::Icon& VideoWidget::Theme::icon(void) const {
	return icon_;
}

void VideoWidget::Theme::setIcon(const VideoWidget::Theme::Icon &icon) {
	icon_ = icon;
}

const std::string& VideoWidget::Theme::iconFile(void) const {
	return icon_file_;
}

void VideoWidget::Theme::setIconFile(const std::string &file) {
	icon_file_ = file;
}

const double& VideoWidget::Theme::iconSize(void) const {
	return icon_size_;
}

void VideoWidget::Theme::setIconSize(const double &size) {
	icon_size_ = size;
}

const float * VideoWidget::Theme::iconColor(void) const {
	return icon_color_;
}

bool VideoWidget::Theme::setIconColor(std::string color) {
	return hex2color(icon_color_, color);
}

bool VideoWidget::Theme::setIconColor(double r, double g, double b, double a) {
	icon_color_[0] = r;
	icon_color_[1] = g;
	icon_color_[2] = b;
	icon_color_[3] = a;
	return true;
}

enum VideoWidget::Theme::Align VideoWidget::Theme::labelHorizontalAlign(void) const {
	return label_horizontal_align_;
}

void VideoWidget::Theme::setLabelHorizontalAlign(VideoWidget::Theme::Align align) {
	label_horizontal_align_ = align;
}

enum VideoWidget::Theme::Align VideoWidget::Theme::labelVerticalAlign(void) const {
	return label_vertical_align_;
}

void VideoWidget::Theme::setLabelVerticalAlign(VideoWidget::Theme::Align align) {
	label_vertical_align_ = align;
}

const std::string& VideoWidget::Theme::labelFontFamily(void) const {
	return label_font_family_;
}

void VideoWidget::Theme::setLabelFontFamily(std::string family) {
	if (family.empty())
		return;

	label_font_family_ = family;
};

double VideoWidget::Theme::labelFontSize(void) const {
	return label_font_size_;
}

void VideoWidget::Theme::setLabelFontSize(double size) {
	if (size < 0)
		return;

	label_font_size_ = size;
}

enum VideoWidget::Theme::FontStyle VideoWidget::Theme::labelFontStyle(void) const {
	return label_font_style_;
}

void VideoWidget::Theme::setLabelFontStyle(VideoWidget::Theme::FontStyle style) {
	label_font_style_ = style;
}

enum VideoWidget::Theme::FontWeight VideoWidget::Theme::labelFontWeight(void) const {
	return label_font_weight_;
}

void VideoWidget::Theme::setLabelFontWeight(VideoWidget::Theme::FontWeight weight) {
	label_font_weight_ = weight;
}

const float * VideoWidget::Theme::labelColor(void) const {
	return label_color_;
}

bool VideoWidget::Theme::setLabelColor(std::string color) {
	return hex2color(label_color_, color);
}

bool VideoWidget::Theme::setLabelColor(double r, double g, double b, double a) {
	label_color_[0] = r;
	label_color_[1] = g;
	label_color_[2] = b;
	label_color_[3] = a;
	return true;
}

double VideoWidget::Theme::labelShadowDistance(void) const {
	return label_shadow_distance_;
}

void VideoWidget::Theme::setLabelShadowDistance(double distance) {
	if (distance < 0)
		return;

	label_shadow_distance_ = distance;
}

int VideoWidget::Theme::labelShadowOpacity(void) const {
	return label_shadow_opacity_;
}

void VideoWidget::Theme::setLabelShadowOpacity(int opacity) {
	if (opacity < 0)
		return;

	label_shadow_opacity_ = opacity;
}

double VideoWidget::Theme::labelBorderWidth(void) const {
	return label_border_width_;
}

void VideoWidget::Theme::setLabelBorderWidth(double width) {
	if (width < 0)
		return;

	label_border_width_ = width;
}

const float * VideoWidget::Theme::labelBorderColor(void) const {
	return label_border_color_;
}

bool VideoWidget::Theme::setLabelBorderColor(std::string color) {
	return hex2color(label_border_color_, color);
}

bool VideoWidget::Theme::setLabelBorderColor(double r, double g, double b, double a) {
	label_border_color_[0] = r;
	label_border_color_[1] = g;
	label_border_color_[2] = b;
	label_border_color_[3] = a;
	return true;
}

const int& VideoWidget::Theme::valueMin(void) const {
	return value_min_;
}

void VideoWidget::Theme::setValueMin(int value) {
	if (value < value_max_)
		value_min_ = value;
}

const int& VideoWidget::Theme::valueMax(void) const {
	return value_max_;
}

void VideoWidget::Theme::setValueMax(int value) {
	if (value > value_min_)
		value_max_ = value;
}

enum VideoWidget::Theme::Align VideoWidget::Theme::valueHorizontalAlign(void) const {
	return value_horizontal_align_;
}

void VideoWidget::Theme::setValueHorizontalAlign(VideoWidget::Theme::Align align) {
	value_horizontal_align_ = align;
}

enum VideoWidget::Theme::Align VideoWidget::Theme::valueVerticalAlign(void) const {
	return value_vertical_align_;
}

void VideoWidget::Theme::setValueVerticalAlign(VideoWidget::Theme::Align align) {
	value_vertical_align_ = align;
}

const std::string& VideoWidget::Theme::valueFontFamily(void) const {
	return value_font_family_;
}

void VideoWidget::Theme::setValueFontFamily(std::string family) {
	if (family.empty())
		return;

	value_font_family_ = family;
};

double VideoWidget::Theme::valueFontSize(void) const {
	return value_font_size_;
}

void VideoWidget::Theme::setValueFontSize(double size) {
	if (size < 0)
		return;

	value_font_size_ = size;
}

enum VideoWidget::Theme::FontStyle VideoWidget::Theme::valueFontStyle(void) const {
	return value_font_style_;
}

void VideoWidget::Theme::setValueFontStyle(VideoWidget::Theme::FontStyle style) {
	value_font_style_ = style;
}

enum VideoWidget::Theme::FontWeight VideoWidget::Theme::valueFontWeight(void) const {
	return value_font_weight_;
}

void VideoWidget::Theme::setValueFontWeight(VideoWidget::Theme::FontWeight weight) {
	value_font_weight_ = weight;
}

const float * VideoWidget::Theme::valueColor(void) const {
	return value_color_;
}

bool VideoWidget::Theme::setValueColor(std::string color) {
	return hex2color(value_color_, color);
}

bool VideoWidget::Theme::setValueColor(double r, double g, double b, double a) {
	value_color_[0] = r;
	value_color_[1] = g;
	value_color_[2] = b;
	value_color_[3] = a;
	return true;
}

double VideoWidget::Theme::valueShadowDistance(void) const {
	return value_shadow_distance_;
}

void VideoWidget::Theme::setValueShadowDistance(double distance) {
	if (distance < 0)
		return;

	value_shadow_distance_ = distance;
}

int VideoWidget::Theme::valueShadowOpacity(void) const {
	return value_shadow_opacity_;
}

void VideoWidget::Theme::setValueShadowOpacity(int opacity) {
	if (opacity < 0)
		return;

	value_shadow_opacity_ = opacity;
}

const double& VideoWidget::Theme::valueBorderWidth(void) const {
	return value_border_width_;
}

void VideoWidget::Theme::setValueBorderWidth(double width) {
	if (width < 0)
		return;

	value_border_width_ = width;
}

const float * VideoWidget::Theme::valueBorderColor(void) const {
	return value_border_color_;
}

bool VideoWidget::Theme::setValueBorderColor(std::string color) {
	return hex2color(value_border_color_, color);
}

bool VideoWidget::Theme::setValueBorderColor(double r, double g, double b, double a) {
	value_border_color_[0] = r;
	value_border_color_[1] = g;
	value_border_color_[2] = b;
	value_border_color_[3] = a;
	return true;
}

double VideoWidget::Theme::unitFontSize(void) const {
	return unit_font_size_;
}

void VideoWidget::Theme::setUnitFontSize(double size) {
	if (size < 0)
		return;

	unit_font_size_ = size;
}

double VideoWidget::Theme::lineSpace(void) const {
	return line_space_;
}

void VideoWidget::Theme::setLineSpace(double size) {
	if (size < 0)
		return;

	line_space_ = size;
}

const int& VideoWidget::Theme::gaugeAngle(void) const {
	return gauge_angle_;
}

void VideoWidget::Theme::setGaugeAngle(int angle) {
	if (angle < 0)
		return;

	gauge_angle_ = angle;
}

const int& VideoWidget::Theme::gaugeRotation(void) const {
	return gauge_rotation_;
}

void VideoWidget::Theme::setGaugeRotation(int rotation) {
	if (rotation < 0)
		return;

	gauge_rotation_ = rotation;
}

const VideoWidget::Orientation& VideoWidget::Theme::gaugeOrientation(void) const {
	return gauge_orientation_;
}

void VideoWidget::Theme::setGaugeOrientation(VideoWidget::Orientation orientation) {
	gauge_orientation_ = orientation;
}

const bool& VideoWidget::Theme::gaugeFlip(void) const {
	return gauge_flip_;
}

void VideoWidget::Theme::setGaugeFlip(bool flip) {
	gauge_flip_ = flip;
}

const double& VideoWidget::Theme::gaugeWidth(void) const {
	return gauge_width_;
}

void VideoWidget::Theme::setGaugeWidth(double width) {
	if (width < 0)
		return;

	gauge_width_ = width;
}

const double& VideoWidget::Theme::gaugeOffset(void) const {
	return gauge_offset_;
}

void VideoWidget::Theme::setGaugeOffset(double offset) {
	gauge_offset_ = offset;
}

VideoWidget::Theme::GaugeCap VideoWidget::Theme::gaugeCap(void) const {
	return gauge_cap_;
}

void VideoWidget::Theme::setGaugeCap(VideoWidget::Theme::GaugeCap cap) {
	gauge_cap_ = cap;
}

const double& VideoWidget::Theme::gaugeBorder(void) const {
	return gauge_border_;
}

void VideoWidget::Theme::setGaugeBorder(double border) {
	gauge_border_ = border;
}

const float * VideoWidget::Theme::gaugeBorderColor(void) const {
	return gauge_border_color_;
}

bool VideoWidget::Theme::setGaugeBorderColor(std::string color) {
	return hex2color(gauge_border_color_, color);
}

bool VideoWidget::Theme::setGaugeBorderColor(double r, double g, double b, double a) {
	gauge_border_color_[0] = r;
	gauge_border_color_[1] = g;
	gauge_border_color_[2] = b;
	gauge_border_color_[3] = a;
	return true;
}

const float * VideoWidget::Theme::gaugePrimaryColor() const {
	return gauge_primary_color_;
}

bool VideoWidget::Theme::setGaugePrimaryColor(std::string color) {
	return hex2color(gauge_primary_color_, color);
}

bool VideoWidget::Theme::setGaugePrimaryColor(double r, double g, double b, double a) {
	gauge_primary_color_[0] = r;
	gauge_primary_color_[1] = g;
	gauge_primary_color_[2] = b;
	gauge_primary_color_[3] = a;
	return true;
}

const float * VideoWidget::Theme::gaugeSecondaryColor() const {
	return gauge_secondary_color_;
}

bool VideoWidget::Theme::setGaugeSecondaryColor(std::string color) {
	return hex2color(gauge_secondary_color_, color);
}

bool VideoWidget::Theme::setGaugeSecondaryColor(double r, double g, double b, double a) {
	gauge_secondary_color_[0] = r;
	gauge_secondary_color_[1] = g;
	gauge_secondary_color_[2] = b;
	gauge_secondary_color_[3] = a;
	return true;
}

const float * VideoWidget::Theme::gaugeBackgroundColor(void) const {
	return gauge_bg_color_;
}

bool VideoWidget::Theme::setGaugeBackgroundColor(std::string color) {
	return hex2color(gauge_bg_color_, color);
}

bool VideoWidget::Theme::setGaugeBackgroundColor(double r, double g, double b, double a) {
	gauge_bg_color_[0] = r;
	gauge_bg_color_[1] = g;
	gauge_bg_color_[2] = b;
	gauge_bg_color_[3] = a;
	return true;
}

const double& VideoWidget::Theme::tickSize(void) const {
	return tick_size_;
}

void VideoWidget::Theme::setTickSize(double size) {
	if (size < 0)
		return;

	tick_size_ = size;
}

const VideoWidget::Theme::Align& VideoWidget::Theme::tickAlign(void) const {
	return tick_align_;
}

void VideoWidget::Theme::setTickAlign(VideoWidget::Theme::Align align) {
	tick_align_ = align;
}

const float * VideoWidget::Theme::tickColor(void) const {
	return tick_color_;
}

bool VideoWidget::Theme::setTickColor(std::string color) {
	return hex2color(tick_color_, color);
}

bool VideoWidget::Theme::setTickColor(double r, double g, double b, double a) {
	tick_color_[0] = r;
	tick_color_[1] = g;
	tick_color_[2] = b;
	tick_color_[3] = a;
	return true;
}

const double& VideoWidget::Theme::tickLabelDistance(void) const {
	return tick_label_distance_;
}

void VideoWidget::Theme::setTickLabelDistance(double distance) {
	if (distance < 0)
		return;

	tick_label_distance_ = distance;
}

double VideoWidget::Theme::tickLabelFontSize(void) const {
	return tick_label_font_size_;
}

void VideoWidget::Theme::setTickLabelFontSize(double size) {
	if (size < 0)
		return;

	tick_label_font_size_ = size;
}

const float * VideoWidget::Theme::tickLabelColor(void) const {
	return tick_label_color_;
}

bool VideoWidget::Theme::setTickLabelColor(std::string color) {
	return hex2color(tick_label_color_, color);
}

bool VideoWidget::Theme::setTickLabelColor(double r, double g, double b, double a) {
	tick_label_color_[0] = r;
	tick_label_color_[1] = g;
	tick_label_color_[2] = b;
	tick_label_color_[3] = a;
	return true;
}

const float * VideoWidget::Theme::tickLabelBorderColor(void) const {
	return tick_label_border_color_;
}

bool VideoWidget::Theme::setTickLabelBorderColor(std::string color) {
	return hex2color(tick_label_border_color_, color);
}

bool VideoWidget::Theme::setTickLabelBorderColor(double r, double g, double b, double a) {
	tick_label_border_color_[0] = r;
	tick_label_border_color_[1] = g;
	tick_label_border_color_[2] = b;
	tick_label_border_color_[3] = a;
	return true;
}

VideoWidget::Theme::NeedleType VideoWidget::Theme::needleType(void) const {
	return needle_type_;
}

void VideoWidget::Theme::setNeedleType(VideoWidget::Theme::NeedleType type) {
	needle_type_ = type;
}

const double& VideoWidget::Theme::needleDistance(void) const {
	return needle_distance_;
}

void VideoWidget::Theme::setNeedleDistance(double distance) {
	if (distance < 0)
		return;

	needle_distance_ = distance;
}

const double& VideoWidget::Theme::needleBorder(void) const {
	return needle_border_;
}

void VideoWidget::Theme::setNeedleBorder(double border) {
	needle_border_ = border;
}

const float * VideoWidget::Theme::needleBorderColor(void) const {
	return needle_border_color_;
}

bool VideoWidget::Theme::setNeedleBorderColor(std::string color) {
	return hex2color(needle_border_color_, color);
}

bool VideoWidget::Theme::setNeedleBorderColor(double r, double g, double b, double a) {
	needle_border_color_[0] = r;
	needle_border_color_[1] = g;
	needle_border_color_[2] = b;
	needle_border_color_[3] = a;
	return true;
}

const float * VideoWidget::Theme::needleBackgroundColor(void) const {
	return needle_background_color_;
}

bool VideoWidget::Theme::setNeedleBackgroundColor(std::string color) {
	return hex2color(needle_background_color_, color);
}

bool VideoWidget::Theme::setNeedleBackgroundColor(double r, double g, double b, double a) {
	needle_background_color_[0] = r;
	needle_background_color_[1] = g;
	needle_background_color_[2] = b;
	needle_background_color_[3] = a;
	return true;
}

const float * VideoWidget::Theme::needlePrimaryColor(void) const {
	return needle_primary_color_;
}

bool VideoWidget::Theme::setNeedlePrimaryColor(std::string color) {
	return hex2color(needle_primary_color_, color);
}

bool VideoWidget::Theme::setNeedlePrimaryColor(double r, double g, double b, double a) {
	needle_primary_color_[0] = r;
	needle_primary_color_[1] = g;
	needle_primary_color_[2] = b;
	needle_primary_color_[3] = a;
	return true;
}

const float * VideoWidget::Theme::needleSecondaryColor(void) const {
	return needle_secondary_color_;
}

bool VideoWidget::Theme::setNeedleSecondaryColor(std::string color) {
	return hex2color(needle_secondary_color_, color);
}

bool VideoWidget::Theme::setNeedleSecondaryColor(double r, double g, double b, double a) {
	needle_secondary_color_[0] = r;
	needle_secondary_color_[1] = g;
	needle_secondary_color_[2] = b;
	needle_secondary_color_[3] = a;
	return true;
}


const double& VideoWidget::Theme::cursorWidth(void) const {
	return cursor_width_;
}

void VideoWidget::Theme::setCursorWidth(double width) {
	cursor_width_ = width;
}

const float * VideoWidget::Theme::cursorColor(void) const {
	return cursor_color_;
}

bool VideoWidget::Theme::setCursorColor(std::string color) {
	return hex2color(cursor_color_, color);
}

bool VideoWidget::Theme::setCursorColor(double r, double g, double b, double a) {
	cursor_color_[0] = r;
	cursor_color_[1] = g;
	cursor_color_[2] = b;
	cursor_color_[3] = a;
	return true;
}


const double& VideoWidget::Theme::axisThick(void) const {
	return axis_thick_;
}


void VideoWidget::Theme::setAxisThick(double thick) {
	axis_thick_ = thick;
}

const double& VideoWidget::Theme::axisBorder(void) const {
	return axis_border_;
}


void VideoWidget::Theme::setAxisBorder(double border) {
	axis_border_ = border;
}


const float * VideoWidget::Theme::axisColor(void) const {
	return axis_color_;
}


bool VideoWidget::Theme::setAxisColor(std::string color) {
	return hex2color(axis_color_, color);
}


bool VideoWidget::Theme::setAxisColor(double r, double g, double b, double a) {
	axis_color_[0] = r;
	axis_color_[1] = g;
	axis_color_[2] = b;
	axis_color_[3] = a;
	return true;
}


const float * VideoWidget::Theme::axisBorderColor(void) const {
	return axis_border_color_;
}


bool VideoWidget::Theme::setAxisBorderColor(std::string color) {
	return hex2color(axis_border_color_, color);
}


bool VideoWidget::Theme::setAxisBorderColor(double r, double g, double b, double a) {
	axis_border_color_[0] = r;
	axis_border_color_[1] = g;
	axis_border_color_[2] = b;
	axis_border_color_[3] = a;
	return true;
}


const double& VideoWidget::Theme::curveThick(void) const {
	return curve_thick_;
}


void VideoWidget::Theme::setCurveThick(double thick) {
	curve_thick_ = thick;
}

const double& VideoWidget::Theme::curveBorder(void) const {
	return curve_border_;
}


void VideoWidget::Theme::setCurveBorder(double border) {
	curve_border_ = border;
}


const float * VideoWidget::Theme::curveColor(void) const {
	return curve_color_;
}


bool VideoWidget::Theme::setCurveColor(std::string color) {
	return hex2color(curve_color_, color);
}


bool VideoWidget::Theme::setCurveColor(double r, double g, double b, double a) {
	curve_color_[0] = r;
	curve_color_[1] = g;
	curve_color_[2] = b;
	curve_color_[3] = a;
	return true;
}


const float * VideoWidget::Theme::curveBorderColor(void) const {
	return curve_border_color_;
}


bool VideoWidget::Theme::setCurveBorderColor(std::string color) {
	return hex2color(curve_border_color_, color);
}


bool VideoWidget::Theme::setCurveBorderColor(double r, double g, double b, double a) {
	curve_border_color_[0] = r;
	curve_border_color_[1] = g;
	curve_border_color_[2] = b;
	curve_border_color_[3] = a;
	return true;
}


const float * VideoWidget::Theme::curveFillColor(void) const {
	return curve_fill_color_;
}


bool VideoWidget::Theme::setCurveFillColor(std::string color) {
	return hex2color(curve_fill_color_, color);
}


bool VideoWidget::Theme::setCurveFillColor(double r, double g, double b, double a) {
	curve_fill_color_[0] = r;
	curve_fill_color_[1] = g;
	curve_fill_color_[2] = b;
	curve_fill_color_[3] = a;
	return true;
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
	else if (s == "course")
		type = VideoWidget::WidgetCourse;
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
	else if (s == "chart")
		shape = VideoWidget::ShapeChart;
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

	if (s.empty() || (s == "none") || (s == "basic"))
		type = VideoWidget::Theme::NeedleTypeBasic;
	else if (s == "thin")
		type = VideoWidget::Theme::NeedleTypeThin;
	else if (s == "light")
		type = VideoWidget::Theme::NeedleTypeLight;
	else if (s == "design")
		type = VideoWidget::Theme::NeedleTypeDesign;
	else if (s == "icon")
		type = VideoWidget::Theme::NeedleTypeIcon;
	else if (s == "value")
		type = VideoWidget::Theme::NeedleTypeValue;
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
	else if (s == "internal:avgspeed")
		icon = VideoWidget::Theme::IconAverageSpeed;
	else if (s == "internal:avgridespeed")
		icon = VideoWidget::Theme::IconAverageRideSpeed;
	else if (s == "internal:batterylevel")
		icon = VideoWidget::Theme::IconBatteryLevel;
	else if (s == "internal:cadence")
		icon = VideoWidget::Theme::IconCadence;
	else if (s == "internal:date")
		icon = VideoWidget::Theme::IconDate;
	else if (s == "internal:distance")
		icon = VideoWidget::Theme::IconDistance;
	else if (s == "internal:duration")
		icon = VideoWidget::Theme::IconDuration;
	else if (s == "internal:elevation")
		icon = VideoWidget::Theme::IconElevation;
	else if (s == "internal:gforce")
		icon = VideoWidget::Theme::IconGForce;
	else if (s == "internal:gpx")
		icon = VideoWidget::Theme::IconGPX;
	else if (s == "internal:grade")
		icon = VideoWidget::Theme::IconGrade;
	else if (s == "internal:course")
		icon = VideoWidget::Theme::IconCourse;
	else if (s == "internal:heading")
		icon = VideoWidget::Theme::IconHeading;
	else if (s == "internal:heartrate")
		icon = VideoWidget::Theme::IconHeartRate;
	else if (s == "internal:homedistance")
		icon = VideoWidget::Theme::IconHomeDistance;
	else if (s == "internal:image")
		icon = VideoWidget::Theme::IconImage;
	else if (s == "internal:lap")
		icon = VideoWidget::Theme::IconLap;
	else if (s == "internal:map")
		icon = VideoWidget::Theme::IconMap;
	else if (s == "internal:maxspeed")
		icon = VideoWidget::Theme::IconMaxSpeed;
	else if (s == "internal:position")
		icon = VideoWidget::Theme::IconPosition;
	else if (s == "internal:power")
		icon = VideoWidget::Theme::IconPower;
	else if (s == "internal:speed")
		icon = VideoWidget::Theme::IconSpeed;
	else if (s == "internal:temperature")
		icon = VideoWidget::Theme::IconTemperature;
	else if (s == "internal:text")
		icon = VideoWidget::Theme::IconText;
	else if (s == "internal:time")
		icon = VideoWidget::Theme::IconTime;
	else if (s == "internal:track")
		icon = VideoWidget::Theme::IconTrack;
	else if (s == "internal:verticalspeed")
		icon = VideoWidget::Theme::IconVerticalSpeed;
	else if (s == "internal:spot")
		icon = VideoWidget::Theme::IconSpot;
	else if (s == "internal:running")
		icon = VideoWidget::Theme::IconRunning;
	else if (s == "internal:bike")
		icon = VideoWidget::Theme::IconBike;
	else if (Utils::starts_with(s, "file:"))
		icon = VideoWidget::Theme::IconUserFile;
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
	case VideoWidget::ShapeChart:
		return "chart";
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
	case VideoWidget::WidgetCourse:
		return "course";
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
	case Theme::NeedleTypeIcon:
		return "icon";
	case Theme::NeedleTypeValue:
		return "value";
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
	case VideoWidget::Theme::IconDefault:
		return "default";

	case VideoWidget::Theme::IconAverageSpeed:
		return "internal:avgspeed";
	case VideoWidget::Theme::IconAverageRideSpeed:
		return "internal:avgridespeed";
	case VideoWidget::Theme::IconBatteryLevel:
		return "internal:batterylevel";
	case VideoWidget::Theme::IconCadence:
		return "internal:cadence";
	case VideoWidget::Theme::IconDate:
		return "internal:date";
	case VideoWidget::Theme::IconDistance:
		return "internal:distance";
	case VideoWidget::Theme::IconDuration:
		return "internal:duration";
	case VideoWidget::Theme::IconElevation:
		return "internal:elevation";
	case VideoWidget::Theme::IconGForce:
		return "internal:gforce";
	case VideoWidget::Theme::IconGPX:
		return "internal:gpx";
	case VideoWidget::Theme::IconGrade:
		return "internal:grade";
	case VideoWidget::Theme::IconCourse:
		return "internal:course";
	case VideoWidget::Theme::IconHeading:
		return "internal:heading";
	case VideoWidget::Theme::IconHeartRate:
		return "internal:heartrate";
	case VideoWidget::Theme::IconHomeDistance:
		return "internal:homedistance";
	case VideoWidget::Theme::IconImage:
		return "internal:image";
	case VideoWidget::Theme::IconLap:
		return "internal:lap";
	case VideoWidget::Theme::IconMap:
		return "internal:map";
	case VideoWidget::Theme::IconMaxSpeed:
		return "internal:maxspeed";
	case VideoWidget::Theme::IconPosition:
		return "internal:position";
	case VideoWidget::Theme::IconPower:
		return "internal:power";
	case VideoWidget::Theme::IconSpeed:
		return "internal:speed";
	case VideoWidget::Theme::IconTemperature:
		return "internal:temperature";
	case VideoWidget::Theme::IconText:
		return "internal:text";
	case VideoWidget::Theme::IconTime:
		return "internal:time";
	case VideoWidget::Theme::IconTrack:
		return "internal:track";
	case VideoWidget::Theme::IconVerticalSpeed:
		return "internal:verticalspeed";

	case VideoWidget::Theme::IconSpot:
		return "internal:spot";
	case VideoWidget::Theme::IconRunning:
		return "internal:running";
	case VideoWidget::Theme::IconBike:
		return "internal:bike";

	case VideoWidget::Theme::IconUserFile:
		return "file:";

	default:
		return "";
	}
}


std::string VideoWidget::zoom2string(VideoWidget::Zoom zoom) {
	switch (zoom) {
	case VideoWidget::ZoomNone:
		return "none";
	case VideoWidget::ZoomFit:
		return "fit";
	case VideoWidget::ZoomFill:
		return "fill";
	case VideoWidget::ZoomCrop:
		return "crop";
	case VideoWidget::ZoomStretch:
		return "stretch";
	default:
		return "";
	}
}


void VideoWidget::save(std::ostream &os) {
	log_call();

	os.imbue(std::locale::classic());

	xmlopen(os);

	{
		Utils::IndentingOStreambuf indent(os, 4);

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
	os <<   " display=\"" << bool2string(visible()) << "\"";
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
	case VideoWidget::WidgetCourse:
		return path + "/compass.svg";
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
	else if ((VideoWidget::Widget) icon < VideoWidget::WidgetUnknown)
		return VideoWidget::getIconFilename((VideoWidget::Widget) icon);
	else if (icon == VideoWidget::Theme::IconUserFile)
		return theme().iconFile();

	switch (icon) {
	case VideoWidget::Theme::IconSpot:
		return path + "/spot.svg";
	case VideoWidget::Theme::IconRunning:
		return path + "/running.svg";
	case VideoWidget::Theme::IconBike:
		return path + "/bike.svg";
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
	case VideoWidget::ShapeChart:
		return _("Chart");
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
	case VideoWidget::WidgetCourse:
		return _("Course");
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

