#include <iostream>
#include <memory>

#include <OpenImageIO/fmath.h>
#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include "layoutlib/Parser.h"
#include "layoutlib/ReportCerr.h"

#include "log_i.h"
#include "oiioutils.h"
#include "decoder.h"
#include "audioparams.h"
#include "videoparams.h"
#include "encoder.h"
#include "widgets.h"
#include "renderer.h"


Renderer::Renderer(GPXApplication &app, 
		RendererSettings &renderer_settings, TelemetrySettings &telemetry_settings)
	: Task(app, "renderer") 
	, app_(app)
	, renderer_settings_(renderer_settings)
	, telemetry_settings_(telemetry_settings) {
	container_ = NULL;

	source_ = NULL;
}


Renderer::~Renderer() {
}


bool Renderer::init(MediaContainer *container) {
	log_call();

	if (container == NULL)
		return false;

	// Media
	container_ = container;

	// Compute telemetry range
	computeTelemetryRange();

	return true;
}


bool Renderer::load(void) {
	std::ifstream stream;

	layout::Layout *root;
//	layout::ReportCerr report;
	layout::Parser parser(NULL); //&report);

	std::list<layout::Node *> nodes;

	std::list<layout::Map *> maps;
	std::list<layout::Track *> tracks;
	std::list<layout::Widget *> widgets;

	std::string filename = rendererSettings().layoutfile();

	log_call();

	if (filename.empty()) {
		log_warn("None layout file");
		goto done;
	}

    stream = std::ifstream(filename);

	if (!stream.is_open()) {
		log_error("Open '%s' layout file failure, please check that file is readable", filename.c_str());
		goto failure;
	}

	root = parser.parse(stream);

	if (root == NULL) {
		log_error("Parsing of '%s' failed due to %s on line %d and column %d", 
			filename.c_str(), parser.errorText().c_str(),
			parser.errorLineNumber(), parser.errorColumnNumber());
		goto failure;
	}

	std::cout << "Parsing '" << filename << "' layout file" << std::endl;

	// Load telemetry data
	source_ = TelemetryMedia::open(app_.settings().inputfile(), telemetrySettings(), false);

	// For each layout elements
	nodes = root->getElements();

	for (std::list<layout::Node *>::iterator item = nodes.begin(); item != nodes.end(); ++item) {
		layout::Node *node = (*item);

		if (node == nullptr)
 			continue;

		const std::string name = node->getName();

		if (name == "map")
			loadMap((layout::Map *) node);
		else if (name == "track")
			loadTrack((layout::Track *) node);
		else if (name == "widget")
			loadWidget((layout::Widget *) node);
	}

done:
	return true;

failure:
	return false;
}


bool Renderer::loadMap(layout::Map *m) {
	int width, height;

	std::string s;

	Map *map = NULL;

	std::string icon_end_file;
	std::string icon_start_file;
	std::string icon_position_file;

	unsigned int mapsource;

	int flags = VideoWidget::Theme::FlagNone;

	MapSettings::View view;
	MapSettings::Follow follow;

	MapSettings mapSettings;

	VideoStreamPtr video_stream;

	VideoWidget::Position position = VideoWidget::PositionNone;
	VideoWidget::Orientation orientation = VideoWidget::OrientationNone;

	MapSettings::Icon icon_end = MapSettings::IconDefault;
	MapSettings::Icon icon_start = MapSettings::IconDefault;
	MapSettings::Icon icon_position = MapSettings::IconDefault;

	log_call();

	// Map source
	mapsource = m->source();

	if ((MapSettings::Source) mapsource == MapSettings::SourceNull) {
		log_warn("Map source undefined, skip map widget");
		return false;
	}

	video_stream = container_->getVideoStream();

	// Default size
	//   2704x1520 => 800x500
	//   1920x1080 =>   ?x?
	width = (m->width() > 0) ? m->width() : 800 * video_stream->width() / 2704;
	height = (m->height() > 0) ? m->height() : 500 * video_stream->height() / 1520;

	// View
	s = (const char *) m->view();
	view = MapSettings::string2view(s);

	if (view == MapSettings::ViewUnknown)
		log_error("Map view value '%s' unknown", s.c_str());

	// Follow
	s = (const char *) m->follow();
	follow = MapSettings::string2follow(s);

	if (follow == MapSettings::FollowUnknown)
		log_error("Map follow value '%s' unknown", s.c_str());

	// Position
	s = (const char *) m->position();
	position = VideoWidget::string2position(s);

	if (position == VideoWidget::PositionUnknown)
		log_error("Map position value '%s' unknown", s.c_str());

	// Orientation
	s = (const char *) m->orientation();
	orientation = VideoWidget::string2orientation(s);

	if (orientation == VideoWidget::OrientationUnknown)
		log_error("Map orientation value '%s' unknown", s.c_str());

	// Icon start
	s = (const char *) m->iconStart();
	icon_start = MapSettings::string2icon(s);

	if (icon_start == MapSettings::IconUnknown) {
		log_error("Map loading error, icon start '%s' unknown", s.c_str());
		goto error;
	}
	else if (icon_start == MapSettings::IconUserFile) {
		icon_start_file = Utils::replace(s, "file:", "");
	}
	else
		icon_start_file = "";

	// Icon end
	s = (const char *) m->iconEnd();
	icon_end = MapSettings::string2icon(s);

	if (icon_end == MapSettings::IconUnknown) {
		log_error("Map loading error, icon end '%s' unknown", s.c_str());
		goto error;
	}
	else if (icon_end == MapSettings::IconUserFile) {
		icon_end_file = Utils::replace(s, "file:", "");
	}
	else
		icon_end_file = "";

	// Icon position
	s = (const char *) m->iconPosition();
	icon_position = MapSettings::string2icon(s);

	if (icon_position == MapSettings::IconUnknown) {
		log_error("Map loading error, icon position '%s' unknown", s.c_str());
		goto error;
	}
	else if (icon_position == MapSettings::IconUserFile) {
		icon_position_file = Utils::replace(s, "file:", "");
	}
	else
		icon_position_file = "";

	// Flags
	if (m->withIconEnd())
		flags |= VideoWidget::Theme::FlagIconEnd;
	if (m->withIconStart())
		flags |= VideoWidget::Theme::FlagIconStart;
	if (m->withIconPosition())
		flags |= VideoWidget::Theme::FlagIconPosition;

	// Map settings
	mapSettings.setSize(width, height);
	mapSettings.setSource((MapSettings::Source) mapsource);
	mapSettings.setZoom(m->zoom());
	mapSettings.setView((MapSettings::View) view);
	mapSettings.setDivider(m->factor());
	mapSettings.setPathSmooth((int) m->pathSmooth());
	mapSettings.setPathThick((double) m->pathThick());
	mapSettings.setPathBorder((double) m->pathBorder());
	mapSettings.setPathBorderColor((const char *) m->pathBorderColor());
	mapSettings.setPathPrimaryColor((const char *) m->pathPrimaryColor());
	mapSettings.setPathSecondaryColor((const char *) m->pathSecondaryColor());
	mapSettings.setFollow((MapSettings::Follow) follow);

	// Map icon settings
	mapSettings.setIcon(MapSettings::IconStart, icon_start);
	mapSettings.setIconFile(MapSettings::IconStart, icon_start_file);
	mapSettings.setIconColor(MapSettings::IconStart, (const char *) m->iconStartColor());
	mapSettings.setIconSize(MapSettings::IconStart, (double) m->iconStartSize());
	mapSettings.setIcon(MapSettings::IconEnd, icon_end);
	mapSettings.setIconFile(MapSettings::IconEnd, icon_end_file);
	mapSettings.setIconColor(MapSettings::IconEnd, (const char *) m->iconEndColor());
	mapSettings.setIconSize(MapSettings::IconEnd, (double) m->iconEndSize());
	mapSettings.setIcon(MapSettings::IconPosition, icon_position);
	mapSettings.setIconFile(MapSettings::IconPosition, icon_position_file);
	mapSettings.setIconColor(MapSettings::IconPosition, (const char *) m->iconPositionColor());
	mapSettings.setIconSize(MapSettings::IconPosition, (double) m->iconPositionSize());

	map = Map::create(app_, mapSettings, source_);

	log_info("Load map widget at %dx%d", (int) m->x(), (int) m->y());

	// Widget settings
	map->setVisible(m->display());
	map->setPosition(position);
	map->setOrientation(orientation);
	map->setPosition(m->x(), m->y());
	map->setAtTime(m->at(), m->at() + m->duration());
	map->setSize(mapSettings.width(), mapSettings.height());
	map->setMargin(VideoWidget::MarginAll, m->margin());
	map->setMargin(VideoWidget::MarginLeft, m->marginLeft());
	map->setMargin(VideoWidget::MarginRight, m->marginRight());
	map->setMargin(VideoWidget::MarginTop, m->marginTop());
	map->setMargin(VideoWidget::MarginBottom, m->marginBottom());

	// Widget theme
	map->theme().setFlags(flags);
	map->theme().setPadding(VideoWidget::Theme::PaddingAll, m->padding());
	map->theme().setPadding(VideoWidget::Theme::PaddingLeft, m->paddingLeft());
	map->theme().setPadding(VideoWidget::Theme::PaddingRight, m->paddingRight());
	map->theme().setPadding(VideoWidget::Theme::PaddingTop, m->paddingTop());
	map->theme().setPadding(VideoWidget::Theme::PaddingBottom, m->paddingBottom());
	map->theme().setBorder(m->border());
	map->theme().setBorderColor((const char *) m->borderColor());
	map->theme().setRoundCorner(m->roundCorner());
	map->theme().setBackgroundColor((const char *) m->backgroundColor());

	// Append
	app_.append(map);

	this->append(map);

//skip:
	return true;

error:
	if (map)
		delete map;

	return false;
}


bool Renderer::loadTrack(layout::Track *t) {
	int width, height;

	std::string s;

	Track *track = NULL;

	std::string icon_end_file;
	std::string icon_start_file;
	std::string icon_position_file;

	int flags = VideoWidget::Theme::FlagNone;

	TrackSettings::View view;
	TrackSettings::Follow follow;

	TrackSettings trackSettings;

	VideoStreamPtr video_stream;

	VideoWidget::Position position = VideoWidget::PositionNone;
	VideoWidget::Orientation orientation = VideoWidget::OrientationNone;

	TrackSettings::Icon icon_end = TrackSettings::IconDefault;
	TrackSettings::Icon icon_start = TrackSettings::IconDefault;
	TrackSettings::Icon icon_position = TrackSettings::IconDefault;

	log_call();

	video_stream = container_->getVideoStream();

	// Default size
	//   2704x1520 => 800x500
	//   1920x1080 => 560x350
	width = (t->width() > 0) ? t->width() : 800 * video_stream->width() / 2704;
	height = (t->height() > 0) ? t->height() : 500 * video_stream->height() / 1520;

	// View
	s = (const char *) t->view();
	view = TrackSettings::string2view(s);

	if (view == TrackSettings::ViewUnknown)
		log_error("Track view value '%s' unknown", s.c_str());

	// Follow
	s = (const char *) t->follow();
	follow = TrackSettings::string2follow(s);

	if (follow == TrackSettings::FollowUnknown)
		log_error("Track follow value '%s' unknown", s.c_str());

	// Position
	s = (const char *) t->position();
	position = VideoWidget::string2position(s);

	if (position == VideoWidget::PositionUnknown)
		log_error("Track position value '%s' unknown", s.c_str());

	// Orientation
	s = (const char *) t->orientation();
	orientation = VideoWidget::string2orientation(s);

	if (orientation == VideoWidget::OrientationUnknown)
		log_error("Track orientation value '%s' unknown", s.c_str());

	// Icon start
	s = (const char *) t->iconStart();
	icon_start = TrackSettings::string2icon(s);

	if (icon_start == TrackSettings::IconUnknown) {
		log_error("Track loading error, icon start '%s' unknown", s.c_str());
		goto error;
	}
	else if (icon_start == TrackSettings::IconUserFile) {
		icon_start_file = Utils::replace(s, "file:", "");
	}
	else
		icon_start_file = "";

	// Icon end
	s = (const char *) t->iconEnd();
	icon_end = TrackSettings::string2icon(s);

	if (icon_end == TrackSettings::IconUnknown) {
		log_error("Track loading error, icon end '%s' unknown", s.c_str());
		goto error;
	}
	else if (icon_end == TrackSettings::IconUserFile) {
		icon_end_file = Utils::replace(s, "file:", "");
	}
	else
		icon_end_file = "";

	// Icon position
	s = (const char *) t->iconPosition();
	icon_position = TrackSettings::string2icon(s);

	if (icon_position == TrackSettings::IconUnknown) {
		log_error("Track loading error, icon position '%s' unknown", s.c_str());
		goto error;
	}
	else if (icon_position == TrackSettings::IconUserFile) {
		icon_position_file = Utils::replace(s, "file:", "");
	}
	else
		icon_position_file = "";

	// Flags
	if (t->withIconEnd())
		flags |= VideoWidget::Theme::FlagIconEnd;
	if (t->withIconStart())
		flags |= VideoWidget::Theme::FlagIconStart;
	if (t->withIconPosition())
		flags |= VideoWidget::Theme::FlagIconPosition;

	// Track settings
	trackSettings.setSize(width, height);
	trackSettings.setView((TrackSettings::View) view);
	trackSettings.setDivider(t->factor());
	trackSettings.setPathSmooth((int) t->pathSmooth());
	trackSettings.setPathThick((double) t->pathThick());
	trackSettings.setPathBorder((double) t->pathBorder());
	trackSettings.setPathBorder((double) t->pathBorder());
	trackSettings.setPathBorderColor((const char *) t->pathBorderColor());
	trackSettings.setPathPrimaryColor((const char *) t->pathPrimaryColor());
	trackSettings.setPathSecondaryColor((const char *) t->pathSecondaryColor());
	trackSettings.setFollow((TrackSettings::Follow) follow);

	// Track icon settings
	trackSettings.setIcon(TrackSettings::IconStart, icon_start);
	trackSettings.setIconFile(TrackSettings::IconStart, icon_start_file);
	trackSettings.setIconColor(TrackSettings::IconStart, (const char *) t->iconStartColor());
	trackSettings.setIconSize(TrackSettings::IconStart, (double) t->iconStartSize());
	trackSettings.setIcon(TrackSettings::IconEnd, icon_end);
	trackSettings.setIconFile(TrackSettings::IconEnd, icon_end_file);
	trackSettings.setIconColor(TrackSettings::IconEnd, (const char *) t->iconEndColor());
	trackSettings.setIconSize(TrackSettings::IconEnd, (double) t->iconEndSize());
	trackSettings.setIcon(TrackSettings::IconPosition, icon_position);
	trackSettings.setIconFile(TrackSettings::IconPosition, icon_position_file);
	trackSettings.setIconColor(TrackSettings::IconPosition, (const char *) t->iconPositionColor());
	trackSettings.setIconSize(TrackSettings::IconPosition, (double) t->iconPositionSize());

	track = Track::create(app_, trackSettings, source_);

	log_info("Load track widget at %dx%d", (int) t->x(), (int) t->y());

	// Widget settings
	track->setVisible(t->display());
	track->setPosition(position);
	track->setOrientation(orientation);
	track->setPosition(t->x(), t->y());
	track->setAtTime(t->at(), t->at() + t->duration());
	track->setSize(trackSettings.width(), trackSettings.height());
	track->setMargin(VideoWidget::MarginAll, t->margin());
	track->setMargin(VideoWidget::MarginLeft, t->marginLeft());
	track->setMargin(VideoWidget::MarginRight, t->marginRight());
	track->setMargin(VideoWidget::MarginTop, t->marginTop());
	track->setMargin(VideoWidget::MarginBottom, t->marginBottom());

	// Widget theme
	track->theme().setFlags(flags);
	track->theme().setPadding(VideoWidget::Theme::PaddingAll, t->padding());
	track->theme().setPadding(VideoWidget::Theme::PaddingLeft, t->paddingLeft());
	track->theme().setPadding(VideoWidget::Theme::PaddingRight, t->paddingRight());
	track->theme().setPadding(VideoWidget::Theme::PaddingTop, t->paddingTop());
	track->theme().setPadding(VideoWidget::Theme::PaddingBottom, t->paddingBottom());
	track->theme().setBorder(t->border());
	track->theme().setBorderColor((const char *) t->borderColor());
	track->theme().setRoundCorner(t->roundCorner());
	track->theme().setBackgroundColor((const char *) t->backgroundColor());

	// Append
	app_.append(track);

	this->append(track);

//skip:
	return true;

error:
	if (track)
		delete track;

	return false;
}


bool Renderer::loadWidget(layout::Widget *w) {
	std::string s;

	VideoWidget *widget = NULL;

	std::string format;
	std::string icon_file;

	int flags = VideoWidget::Theme::FlagNone;

	VideoWidget::Widget type = VideoWidget::WidgetUnknown;

	VideoWidget::Shape shape = VideoWidget::ShapeNone;

	TelemetryData::Unit unit = TelemetryData::UnitNone;

	VideoWidget::Zoom zoom = VideoWidget::ZoomNone;

	VideoWidget::Position position = VideoWidget::PositionNone;
	VideoWidget::Orientation orientation = VideoWidget::OrientationNone;
	VideoWidget::Orientation text_orientation = VideoWidget::OrientationNone;
	VideoWidget::Orientation gauge_orientation = VideoWidget::OrientationNone;

	VideoWidget::Theme::Align align;
	VideoWidget::Theme::Align label_horizontal_align = VideoWidget::Theme::AlignLeft;
	VideoWidget::Theme::Align label_vertical_align = VideoWidget::Theme::AlignTop;
	VideoWidget::Theme::Align value_horizontal_align = VideoWidget::Theme::AlignLeft;
	VideoWidget::Theme::Align value_vertical_align = VideoWidget::Theme::AlignBottom;

	VideoWidget::Theme::FontStyle label_font_style = VideoWidget::Theme::FontStyleNormal;
	VideoWidget::Theme::FontStyle value_font_style = VideoWidget::Theme::FontStyleNormal;

	VideoWidget::Theme::FontWeight label_font_weight = VideoWidget::Theme::FontWeightNormal;
	VideoWidget::Theme::FontWeight value_font_weight = VideoWidget::Theme::FontWeightNormal;

	VideoWidget::Theme::Icon icon = VideoWidget::Theme::IconDefault;
	VideoWidget::Theme::GaugeCap gauge_cap = VideoWidget::Theme::GaugeCapSquare;
	VideoWidget::Theme::NeedleType needle_type = VideoWidget::Theme::NeedleTypeBasic;

	// Type
	s = (const char *) w->type();
	type = VideoWidget::string2widget(s);

	if (type == VideoWidget::WidgetUnknown) {
		log_info("Widget '%s' unknown type", (const char *) w->type());
		goto skip;
	}

	// Shape 
	s = (const char *) w->shape();
	shape = VideoWidget::string2shape(s);

	if (shape == VideoWidget::ShapeUnknown) {
		log_error("Widget loading error, shape value '%s' unknown", s.c_str());
		goto error;
	}

	// Position
	s = (const char *) w->position();
	position = VideoWidget::string2position(s);

	if (position == VideoWidget::PositionUnknown) {
		log_error("Widget position value '%s' unknown", s.c_str());
		goto error;
	}

	// Orientation
	s = (const char *) w->orientation();
	orientation = VideoWidget::string2orientation(s);

	if (orientation == VideoWidget::OrientationUnknown) {
		log_error("Widget loading error, orientation value '%s' unknown", s.c_str());
		goto error;
	}

	// Unit
	s = (const char *) w->valueUnit();
	unit = VideoWidget::string2unit(s);

	if (unit == TelemetryData::UnitUnknown) {
		log_error("Widget loading error, unit value '%s' unknown", s.c_str());
		goto error;
	}

	// Icon
	s = (const char *) w->icon();
	icon = VideoWidget::string2icon(s);

	if (icon == VideoWidget::Theme::IconUnknown) {
		log_error("Widget loading error, icon '%s' unknown", s.c_str());
		goto error;
	}
	else if (icon == VideoWidget::Theme::IconUserFile) {
		icon_file = Utils::replace(s, "file:", "");
	}
	else
		icon_file = "";

	// Format
	format = (const char *) w->valueFormat();

	// Zoom
	s = (const char *) w->zoom();
	zoom = VideoWidget::string2zoom(s);

	log_info("Load widget '%s' (shape: %s) at %dx%d", 
			(const char *) w->type(), (const char *) w->shape(), (int) w->x(), (int) w->y());

	// Font style for label
	s = (const char *) w->labelFontStyle();
	label_font_style = VideoWidget::string2fontstyle(s);

	if (label_font_style == VideoWidget::Theme::FontStyleUnknown) {
		log_error("Widget loading error, label font style value '%s' unknown", s.c_str());
		goto error;
	}

	// Font weight for label
	s = (const char *) w->labelFontWeight();
	label_font_weight = VideoWidget::string2fontweight(s);

	if (label_font_weight == VideoWidget::Theme::FontWeightUnknown) {
		log_error("Widget loading error, label font weight value '%s' unknown", s.c_str());
		goto error;
	}

	// Text horizontal alignment for label
	s = (const char *) w->labelHorizontalAlign();
	align = VideoWidget::string2align(s);

	if (align == VideoWidget::Theme::AlignUnknown) {
		log_error("Widget loading error, label horizontal align value '%s' unknown", s.c_str());
		goto error;
	}
	else if (align != VideoWidget::Theme::AlignNone)
		label_horizontal_align = align;

	// Text vertical alignment for label
	s = (const char *) w->labelVerticalAlign();
	align = VideoWidget::string2align(s);

	if (align == VideoWidget::Theme::AlignUnknown) {
		log_error("Widget loading error, label vertical align value '%s' unknown", s.c_str());
		goto error;
	}
	else if (align != VideoWidget::Theme::AlignNone)
		label_vertical_align = align;

	// Font style for value
	s = (const char *) w->valueFontStyle();
	value_font_style = VideoWidget::string2fontstyle(s);

	if (value_font_style == VideoWidget::Theme::FontStyleUnknown) {
		log_error("Widget loading error, value font style value '%s' unknown", s.c_str());
		goto error;
	}

	// Font weight for value
	s = (const char *) w->valueFontWeight();
	value_font_weight = VideoWidget::string2fontweight(s);

	if (value_font_weight == VideoWidget::Theme::FontWeightUnknown) {
		log_error("Widget loading error, value font weight value '%s' unknown", s.c_str());
		goto error;
	}

	// Text horizontal alignment for value
	s = (const char *) w->valueHorizontalAlign();
	align = VideoWidget::string2align(s);

	if (align == VideoWidget::Theme::AlignUnknown) {
		log_error("Widget loading error, value horizontal align value '%s' unknown", s.c_str());
		goto error;
	}
	else if (align != VideoWidget::Theme::AlignNone)
		value_horizontal_align = align;

	// Text vertical alignment for value
	s = (const char *) w->valueVerticalAlign();
	align = VideoWidget::string2align(s);

	if (align == VideoWidget::Theme::AlignUnknown) {
		log_error("Widget loading error, value vertical align value '%s' unknown", s.c_str());
		goto error;
	}
	else if (align != VideoWidget::Theme::AlignNone)
		value_vertical_align = align;

	// Text orientation
	s = (const char *) w->textOrientation();
	text_orientation = VideoWidget::string2orientation(s);

	if (text_orientation == VideoWidget::OrientationUnknown) {
		log_error("Widget loading error, text orientation value '%s' unknown", s.c_str());
		goto error;
	}
	else if (text_orientation == VideoWidget::OrientationNone)
		text_orientation = VideoWidget::OrientationHorizontal;

	// Gauge cap
	s = (const char *) w->gaugeCap();
	gauge_cap = VideoWidget::string2gaugecap(s);

	if (gauge_cap == VideoWidget::Theme::GaugeCapUnknown) {
		log_error("Widget loading error, gauge cap value '%s' unknown", s.c_str());
		goto error;
	}

	// Gauge orientation
	s = (const char *) w->gaugeOrientation();
	gauge_orientation = VideoWidget::string2orientation(s);

	if (gauge_orientation == VideoWidget::OrientationUnknown) {
		log_error("Widget loading error, gauge orientation value '%s' unknown", s.c_str());
		goto error;
	}
	else if (gauge_orientation == VideoWidget::OrientationNone)
		gauge_orientation = VideoWidget::OrientationVertical;

	// Needle type
	s = (const char *) w->needleType();
	needle_type = VideoWidget::string2needletype(s);

	if (needle_type == VideoWidget::Theme::NeedleTypeUnknown) {
		log_error("Widget loading error, needle type value '%s' unknown", s.c_str());
		goto error;
	}

	// Lap
	if (type == VideoWidget::WidgetLap) {
		LapWidget *lap = (LapWidget *) widget;
		lap->setTargetLap(w->nbrLap());
	}

	// Flags
	if (w->withLabel())
		flags |= VideoWidget::Theme::FlagLabel;
	if (w->withValue())
		flags |= VideoWidget::Theme::FlagValue;
	if (w->withIcon())
		flags |= VideoWidget::Theme::FlagIcon;
	if (w->withUnit())
		flags |= VideoWidget::Theme::FlagUnit;
	if (w->withTick())
		flags |= VideoWidget::Theme::FlagTick;
	if (w->withTickLabel())
		flags |= VideoWidget::Theme::FlagTickLabel;
	if (w->withGauge())
		flags |= VideoWidget::Theme::FlagGauge;
	if (w->withNeedle())
		flags |= VideoWidget::Theme::FlagNeedle;

	// TODO
	flags |= VideoWidget::Theme::FlagCursor;

	// Create widget
	if ((widget = this->create(type, source_)) == NULL)
		goto skip;

	// Use default values if undefined
	if (unit == TelemetryData::UnitNone)
		unit = widget->valueUnit();
	if (format == "")
		format = widget->valueFormat();

	// Widget common settings
	widget->setVisible(w->display());
	widget->setShape(shape);
	widget->setPosition(position);
	widget->setOrientation(orientation);
	widget->setPosition(w->x(), w->y());
	widget->setAtTime(w->at(), w->at() + w->duration());
	widget->setSize(w->width(), w->height());
	widget->setMargin(VideoWidget::MarginAll, w->margin());
	widget->setMargin(VideoWidget::MarginLeft, w->marginLeft());
	widget->setMargin(VideoWidget::MarginRight, w->marginRight());
	widget->setMargin(VideoWidget::MarginTop, w->marginTop());
	widget->setMargin(VideoWidget::MarginBottom, w->marginBottom());
	
	// Widget misc. settings
	widget->setLabel((const char *) w->name());
	widget->setValue((const char *) w->text());
	widget->setValueUnit(unit);
	widget->setValueFormat(format);
	widget->setZoom(zoom);
	widget->setSource((const char *) w->source());

	// Widget theme
	widget->theme().setFlags(flags);
	widget->theme().setPadding(VideoWidget::Theme::PaddingAll, w->padding());
	widget->theme().setPadding(VideoWidget::Theme::PaddingLeft, w->paddingLeft());
	widget->theme().setPadding(VideoWidget::Theme::PaddingRight, w->paddingRight());
	widget->theme().setPadding(VideoWidget::Theme::PaddingTop, w->paddingTop());
	widget->theme().setPadding(VideoWidget::Theme::PaddingBottom, w->paddingBottom());
	widget->theme().setBorder(w->border());
	widget->theme().setBorderColor((const char *) w->borderColor());
	widget->theme().setRoundCorner(w->roundCorner());
	widget->theme().setBackgroundColor((const char *) w->backgroundColor());

	// Widget icon settings
	widget->theme().setIcon(icon);
	widget->theme().setIconFile(icon_file);
	widget->theme().setIconSize(w->iconSize());
	widget->theme().setIconColor((const char *) w->iconColor());

	// Widget misc. settings
	widget->theme().setLineSpace(w->lineSpace());
	widget->theme().setTextOrientation(text_orientation);

	// Widget label settings
	widget->theme().setLabelFontFamily((const char *) w->labelFontFamily());
	widget->theme().setLabelFontSize(w->labelFontSize());
	widget->theme().setLabelFontStyle(label_font_style);
	widget->theme().setLabelFontWeight(label_font_weight);
	widget->theme().setLabelHorizontalAlign(label_horizontal_align);
	widget->theme().setLabelVerticalAlign(label_vertical_align);
	widget->theme().setLabelColor((const char *) w->labelColor());
	widget->theme().setLabelShadowOpacity(w->labelShadowOpacity());
	widget->theme().setLabelShadowDistance(w->labelShadowDistance());
	widget->theme().setLabelBorderWidth(w->labelBorderWidth());
	widget->theme().setLabelBorderColor((const char *) w->labelBorderColor());

	// Widget value settings
	widget->theme().setValueFontFamily((const char *) w->valueFontFamily());
	widget->theme().setValueFontSize(w->valueFontSize());
	widget->theme().setValueFontStyle(value_font_style);
	widget->theme().setValueFontWeight(value_font_weight);
	widget->theme().setValueHorizontalAlign(value_horizontal_align);
	widget->theme().setValueVerticalAlign(value_vertical_align);
	widget->theme().setValueColor((const char *) w->valueColor());
	widget->theme().setValueShadowOpacity(w->valueShadowOpacity());
	widget->theme().setValueShadowDistance(w->valueShadowDistance());
	widget->theme().setValueBorderWidth(w->valueBorderWidth());
	widget->theme().setValueBorderColor((const char *) w->valueBorderColor());
	widget->theme().setValueMin(w->valueMin());
	widget->theme().setValueMax(w->valueMax());

	// Widget unit settings
	widget->theme().setUnitFontSize(w->unitFontSize());
	widget->theme().setUnitDistance(w->unitDistance());

	// Width gauge settings
	widget->theme().setGaugeAngle(w->gaugeAngle());
	widget->theme().setGaugeRotation(w->gaugeRotation());
	widget->theme().setGaugeOrientation(gauge_orientation);
	widget->theme().setGaugeFlip(w->gaugeFlip());
	widget->theme().setGaugeWidth(w->gaugeWidth());
	widget->theme().setGaugeOffset(w->gaugeOffset());
	widget->theme().setGaugeCap(gauge_cap);
	widget->theme().setGaugeBorder(w->gaugeBorder());
	widget->theme().setGaugeBorderColor((const char *) w->gaugeBorderColor());
	widget->theme().setGaugeBackgroundColor((const char *) w->gaugeBackgroundColor());
	widget->theme().setGaugePrimaryColor((const char *) w->gaugePrimaryColor());
	widget->theme().setGaugeSecondaryColor((const char *) w->gaugeSecondaryColor());

	// Widget tick settings
	widget->theme().setTickSize(w->tickSize());
	widget->theme().setTickColor((const char *) w->tickColor());
	widget->theme().setTickLabelDistance(w->tickLabelDistance());
	widget->theme().setTickLabelFontSize(w->tickLabelFontSize());
	widget->theme().setTickLabelColor((const char *) w->tickLabelColor());

	// Widget needle settings
	widget->theme().setNeedleType(needle_type);
	widget->theme().setNeedleDistance(w->needleDistance());
	widget->theme().setNeedleBorder(w->needleBorder());
	widget->theme().setNeedleBorderColor((const char *) w->needleBorderColor());
	widget->theme().setNeedleBackgroundColor((const char *) w->needleBackgroundColor());
	widget->theme().setNeedlePrimaryColor((const char *) w->needlePrimaryColor());
	widget->theme().setNeedleSecondaryColor((const char *) w->needleSecondaryColor());

skip:
	return true;

error:
	if (widget)
		delete widget;

	return false;
}


void Renderer::append(VideoWidget *widget) {
	log_info("Initialize %s widget", widget->name().c_str());

	widgets_.push_back(widget);
}


void Renderer::remove(VideoWidget *widget) {
	widgets_.remove(widget);

	delete widget;
}


void Renderer::drop(void) {
	while (!widgets_.empty()) {
		VideoWidget *widget = widgets_.front();

		widgets_.pop_front();

		delete widget;
	}
}


void Renderer::computeTelemetryRange(void) {
	log_call();

	uint64_t ts;
	uint64_t duration;

	if (!container_)
		return;

	ts = container_->startTime();
	duration = container_->duration();

	telemetrySettings().setViewRange(ts, ts + duration);
}


void Renderer::computeWidgetsPosition(void) {
	bool is_first;

	int n;
	int width, height;

	int x, y;
	int space, offset;
	int offset_x, offset_y;
	
	int margintop, marginbottom;
	int marginleft, marginright;

	VideoStreamPtr video_stream = container_->getVideoStream();

	log_call();

	// TopLeft
	//-----------------------------------------------------------
	offset_x = 0;
	offset_y = 0;

	is_first = true;

	// Get position (left, top, bottom, right)
	for (VideoWidget *widget : widgets_) {
		if (!widget->visible())
			continue;

		if (widget->position() != VideoWidget::PositionTopLeft)
			continue;

		x = widget->margin(VideoWidget::MarginLeft);
		y = widget->margin(VideoWidget::MarginTop);

		if (widget->orientation() == VideoWidget::OrientationHorizontal)
			x += offset_x;

		if (widget->orientation() == VideoWidget::OrientationVertical)
			y += offset_y;

		widget->setPosition(x, y);

		if (is_first || (widget->orientation() == VideoWidget::OrientationHorizontal)) {
			offset_x += widget->theme().width();
			offset_x += widget->margin(VideoWidget::MarginLeft);
			offset_x += widget->margin(VideoWidget::MarginRight);
		}

		if (is_first || (widget->orientation() == VideoWidget::OrientationVertical)) {
			offset_y += widget->theme().height();
			offset_y += widget->margin(VideoWidget::MarginTop);
			offset_y += widget->margin(VideoWidget::MarginBottom);
		}

		is_first = false;
	}

	// TopRight
	//-----------------------------------------------------------
	offset_x = 0;
	offset_y = 0;

	is_first = true;

	// Get position (left, top, bottom, right)
	for (VideoWidget *widget : widgets_) {
		if (!widget->visible())
			continue;

		if (widget->position() != VideoWidget::PositionTopRight)
			continue;

		x = layout_width_ - widget->margin(VideoWidget::MarginRight) - widget->theme().width();
		y = widget->margin(VideoWidget::MarginTop);

		if (widget->orientation() == VideoWidget::OrientationHorizontal)
			x -= offset_x;

		if (widget->orientation() == VideoWidget::OrientationVertical)
			y += offset_y;

		widget->setPosition(x, y);

		if (is_first || (widget->orientation() == VideoWidget::OrientationHorizontal)) {
			offset_x += widget->theme().width();
			offset_x += widget->margin(VideoWidget::MarginLeft);
			offset_x += widget->margin(VideoWidget::MarginRight);
		}

		if (is_first || (widget->orientation() == VideoWidget::OrientationVertical)) {
			offset_y += widget->theme().height();
			offset_y += widget->margin(VideoWidget::MarginTop);
			offset_y += widget->margin(VideoWidget::MarginBottom);
		}

		is_first = false;
	}
	
	// BottomLeft
	//-----------------------------------------------------------
	offset_x = 0;
	offset_y = 0;

	is_first = true;

	// Get position (left, top, bottom, right)
	for (VideoWidget *widget : widgets_) {
		if (!widget->visible())
			continue;

		if (widget->position() != VideoWidget::PositionBottomLeft)
			continue;

		x = widget->margin(VideoWidget::MarginLeft);
		y = layout_height_ - widget->margin(VideoWidget::MarginBottom) - widget->theme().height();

		if (widget->orientation() == VideoWidget::OrientationHorizontal)
			x += offset_x;

		if (widget->orientation() == VideoWidget::OrientationVertical)
			y -= offset_y;

		widget->setPosition(x, y);

		if (is_first || (widget->orientation() == VideoWidget::OrientationHorizontal)) {
			offset_x += widget->theme().width();
			offset_x += widget->margin(VideoWidget::MarginLeft);
			offset_x += widget->margin(VideoWidget::MarginRight);
		}

		if (is_first || (widget->orientation() == VideoWidget::OrientationVertical)) {
			offset_y += widget->theme().height();
			offset_y += widget->margin(VideoWidget::MarginTop);
			offset_y += widget->margin(VideoWidget::MarginBottom);
		}

		is_first = false;
	}

	// BottomRight
	//-----------------------------------------------------------
	offset_x = 0;
	offset_y = 0;

	is_first = true;

	// Get position (left, top, bottom, right)
	for (VideoWidget *widget : widgets_) {
		if (!widget->visible())
			continue;

		if (widget->position() != VideoWidget::PositionBottomRight)
			continue;

		x = layout_width_ - widget->margin(VideoWidget::MarginRight) - widget->theme().width();
		y = layout_height_ - widget->margin(VideoWidget::MarginBottom) - widget->theme().height();

		if (widget->orientation() == VideoWidget::OrientationHorizontal)
			x -= offset_x;

		if (widget->orientation() == VideoWidget::OrientationVertical)
			y -= offset_y;

		widget->setPosition(x, y);

		if (is_first || (widget->orientation() == VideoWidget::OrientationHorizontal)) {
			offset_x += widget->theme().width();
			offset_x += widget->margin(VideoWidget::MarginLeft);
			offset_x += widget->margin(VideoWidget::MarginRight);
		}

		if (is_first || (widget->orientation() == VideoWidget::OrientationVertical)) {
			offset_y += widget->theme().height();
			offset_y += widget->margin(VideoWidget::MarginTop);
			offset_y += widget->margin(VideoWidget::MarginBottom);
		}

		is_first = false;
	}

	// Left side
	//-----------------------------------------------------------
	n = 0;
	height = 0;
	margintop = 0;
	marginbottom = 0;

	// Compute available height on the left side
	for (VideoWidget *widget : widgets_) {
		if (!widget->visible())
			continue;

		if (widget->position() == VideoWidget::PositionTopLeft) {
			if ((margintop == 0) || (widget->orientation() == VideoWidget::OrientationVertical)) {
				margintop = MAX(widget->theme().height() + widget->margin(VideoWidget::MarginTop) + widget->margin(VideoWidget::MarginBottom), margintop);
				continue;
			}
		}

		if (widget->position() == VideoWidget::PositionBottomLeft) {
			if ((marginbottom == 0) || (widget->orientation() == VideoWidget::OrientationVertical)) {
				marginbottom = MAX(widget->theme().height() + widget->margin(VideoWidget::MarginTop) + widget->margin(VideoWidget::MarginBottom), marginbottom);
				continue;
			}
		}

		if (widget->position() != VideoWidget::PositionLeft)
			continue;

		if ((n == 0) || (widget->orientation() == VideoWidget::OrientationVertical)) {
			height += widget->theme().height();
			height += widget->margin(VideoWidget::MarginTop);
			height += widget->margin(VideoWidget::MarginBottom);
			n++;
		}
	}

	// Compute position for each widget
	space = layout_height_ - (height + margintop + marginbottom);
	space = MAX(0, space);

	// Set position (for 'left' align)
	offset = space / 2;
	offset_x = 0;
	offset_y = 0;

	is_first = true;

	for (VideoWidget *widget : widgets_) {
		if (!widget->visible())
			continue;

		if (widget->position() != VideoWidget::PositionLeft)
			continue;

		x = widget->margin(VideoWidget::MarginLeft);
		y = margintop + offset + widget->margin(VideoWidget::MarginTop);

		if (widget->orientation() == VideoWidget::OrientationHorizontal)
			x += offset_x;

		if (widget->orientation() == VideoWidget::OrientationVertical)
			y += offset_y;

		widget->setPosition(x, y);

		if (is_first || (widget->orientation() == VideoWidget::OrientationHorizontal)) {
			offset_x += widget->theme().width();
			offset_x += widget->margin(VideoWidget::MarginLeft);
			offset_x += widget->margin(VideoWidget::MarginRight);
		}

		if (is_first || (widget->orientation() == VideoWidget::OrientationVertical)) {
			offset_y += widget->theme().height();
			offset_y += widget->margin(VideoWidget::MarginTop);
			offset_y += widget->margin(VideoWidget::MarginBottom);
		}

		is_first = false;
	}

	// Right side
	//-----------------------------------------------------------
	n = 0;
	height = 0;
	margintop = 0;
	marginbottom = 0;

	// Compute available height on the right side
	for (VideoWidget *widget : widgets_) {
		if (!widget->visible())
			continue;

		if (widget->position() == VideoWidget::PositionTopRight) {
			if ((margintop == 0) || (widget->orientation() == VideoWidget::OrientationVertical)) {
				margintop = MAX(widget->theme().height() + widget->margin(VideoWidget::MarginTop) + widget->margin(VideoWidget::MarginBottom), margintop);
				continue;
			}
		}

		if (widget->position() == VideoWidget::PositionBottomRight) {
			if ((marginbottom == 0) || (widget->orientation() == VideoWidget::OrientationVertical)) {
				marginbottom = MAX(widget->theme().height() + widget->margin(VideoWidget::MarginTop) + widget->margin(VideoWidget::MarginBottom), marginbottom);
				continue;
			}
		}

		if (widget->position() != VideoWidget::PositionRight)
			continue;

		if ((n == 0) || (widget->orientation() == VideoWidget::OrientationVertical)) {
			height += widget->theme().height();
			height += widget->margin(VideoWidget::MarginTop);
			height += widget->margin(VideoWidget::MarginBottom);
			n++;
		}
	}

	// Compute position for each widget
	space = layout_height_ - (height + margintop + marginbottom);
	space = MAX(0, space);

	// Set position (for 'right' align)
	offset = space / 2;
	offset_x = 0;
	offset_y = 0;

	is_first = true;

	for (VideoWidget *widget : widgets_) {
		if (!widget->visible())
			continue;

		if (widget->position() != VideoWidget::PositionRight)
			continue;

		x = layout_width_ - widget->margin(VideoWidget::MarginRight) - widget->theme().width();
		y = margintop + offset + widget->margin(VideoWidget::MarginTop);

		if (widget->orientation() == VideoWidget::OrientationHorizontal)
			x -= offset_x;

		if (widget->orientation() == VideoWidget::OrientationVertical)
			y += offset_y;

		widget->setPosition(x, y);

		if (is_first || (widget->orientation() == VideoWidget::OrientationHorizontal)) {
			offset_x += widget->theme().width();
			offset_x += widget->margin(VideoWidget::MarginLeft);
			offset_x += widget->margin(VideoWidget::MarginRight);
		}

		if (is_first || (widget->orientation() == VideoWidget::OrientationVertical)) {
			offset_y += widget->theme().height();
			offset_y += widget->margin(VideoWidget::MarginTop);
			offset_y += widget->margin(VideoWidget::MarginBottom);
		}

		is_first = false;
	}

	// Top side
	//-----------------------------------------------------------
	n = 0;
	width = 0;
	marginleft = 0;
	marginright = 0;

	// Compute available width on the top side
	for (VideoWidget *widget : widgets_) {
		if (!widget->visible())
			continue;

		if (widget->position() == VideoWidget::PositionTopLeft) {
			if ((marginleft == 0) || (widget->orientation() == VideoWidget::OrientationHorizontal)) {
				marginleft = MAX(widget->theme().width() + widget->margin(VideoWidget::MarginLeft) + widget->margin(VideoWidget::MarginRight), marginleft);
				continue;
			}
		}

		if (widget->position() == VideoWidget::PositionTopRight) {
			if ((marginright == 0) || (widget->orientation() == VideoWidget::OrientationHorizontal)) {
				marginright = MAX(widget->theme().width() + widget->margin(VideoWidget::MarginLeft) + widget->margin(VideoWidget::MarginRight), marginright);
				continue;
			}
		}

		if (widget->position() != VideoWidget::PositionTop)
			continue;

		if ((n == 0) || (widget->orientation() == VideoWidget::OrientationHorizontal)) {
			width += widget->theme().width();
			width += widget->margin(VideoWidget::MarginLeft);
			width += widget->margin(VideoWidget::MarginRight);
			n++;
		}
	}

	// Compute position for each widget
	space = layout_width_ - (width + marginleft + marginright);
	space = MAX(0, space);

	// Set position (for 'top' align)
	offset = space / 2;
	offset_x = 0;
	offset_y = 0;

	is_first = true;

	for (VideoWidget *widget : widgets_) {
		if (!widget->visible())
			continue;

		if (widget->position() != VideoWidget::PositionTop)
			continue;

		x = marginleft + offset + widget->margin(VideoWidget::MarginLeft);
		y = widget->margin(VideoWidget::MarginTop);

		if (widget->orientation() == VideoWidget::OrientationHorizontal)
			x += offset_x;

		if (widget->orientation() == VideoWidget::OrientationVertical)
			y += offset_y;

		widget->setPosition(x, y);

		if (is_first || (widget->orientation() == VideoWidget::OrientationHorizontal)) {
			offset_x += widget->theme().width();
			offset_x += widget->margin(VideoWidget::MarginLeft);
			offset_x += widget->margin(VideoWidget::MarginRight);
		}

		if (is_first || (widget->orientation() == VideoWidget::OrientationVertical)) {
			offset_y += widget->theme().height();
			offset_y += widget->margin(VideoWidget::MarginTop);
			offset_y += widget->margin(VideoWidget::MarginBottom);
		}

		is_first = false;
	}

	// Bottom side
	//-----------------------------------------------------------
	n = 0;
	width = 0;
	marginleft = 0;
	marginright = 0;

	// Compute available width on the bottom side
	for (VideoWidget *widget : widgets_) {
		if (!widget->visible())
			continue;

		if (widget->position() == VideoWidget::PositionBottomLeft) {
			if ((marginleft == 0) || (widget->orientation() == VideoWidget::OrientationHorizontal)) {
				marginleft = MAX(widget->theme().width() + widget->margin(VideoWidget::MarginLeft) + widget->margin(VideoWidget::MarginRight), marginleft);
				continue;
			}
		}

		if (widget->position() == VideoWidget::PositionBottomRight) {
			if ((marginright == 0) || (widget->orientation() == VideoWidget::OrientationHorizontal)) {
				marginright = MAX(widget->theme().width() + widget->margin(VideoWidget::MarginLeft) + widget->margin(VideoWidget::MarginRight), marginright);
				continue;
			}
		}

		if (widget->position() != VideoWidget::PositionBottom)
			continue;

		if ((n == 0) || (widget->orientation() == VideoWidget::OrientationHorizontal)) {
			width += widget->theme().width();
			width += widget->margin(VideoWidget::MarginLeft);
			width += widget->margin(VideoWidget::MarginRight);
			n++;
		}
	}

	// Compute position for each widget
	space = layout_width_ - (width + marginleft + marginright);
	space = MAX(0, space);

	// Set position (for 'bottom' align)
	offset = space / 2;
	offset_x = 0;
	offset_y = 0;

	is_first = true;

	for (VideoWidget *widget : widgets_) {
		if (!widget->visible())
			continue;

		if (widget->position() != VideoWidget::PositionBottom)
			continue;

		x = marginleft + offset + widget->margin(VideoWidget::MarginLeft);
		y = layout_height_ - widget->margin(VideoWidget::MarginBottom) - widget->theme().height();

		if (widget->orientation() == VideoWidget::OrientationHorizontal)
			x += offset_x;

		if (widget->orientation() == VideoWidget::OrientationVertical)
			y -= offset_y;

		widget->setPosition(x, y);

		if (is_first || (widget->orientation() == VideoWidget::OrientationHorizontal)) {
			offset_x += widget->theme().width();
			offset_x += widget->margin(VideoWidget::MarginLeft);
			offset_x += widget->margin(VideoWidget::MarginRight);
		}

		if (is_first || (widget->orientation() == VideoWidget::OrientationVertical)) {
			offset_y += widget->theme().height();
			offset_y += widget->margin(VideoWidget::MarginTop);
			offset_y += widget->margin(VideoWidget::MarginBottom);
		}

		is_first = false;
	}
}


VideoWidget * Renderer::create(VideoWidget::Widget type, TelemetrySource *source) {
	VideoWidget *widget = NULL;

	switch (type) {
	case VideoWidget::WidgetAverageSpeed: 
		widget = AvgSpeedWidget::create(app_, source);
		break;
	case VideoWidget::WidgetAverageRideSpeed: 
		widget = AvgRideSpeedWidget::create(app_, source);
		break;
	case VideoWidget::WidgetBatteryLevel:
		widget = BatteryLevelWidget::create(app_, source);
		break;
	case VideoWidget::WidgetCadence:
		widget = CadenceWidget::create(app_, source);
		break;
	case VideoWidget::WidgetDate:
		widget = DateWidget::create(app_, source);
		break;
	case VideoWidget::WidgetDistance:
		widget = DistanceWidget::create(app_, source);
		break;
	case VideoWidget::WidgetDuration: 
		widget = DurationWidget::create(app_, source);
		break;
	case VideoWidget::WidgetElevation:
		widget = ElevationWidget::create(app_, source);
		break;
	case VideoWidget::WidgetGForce:
		widget = GForceWidget::create(app_, source);
		break;
	case VideoWidget::WidgetGPX:
		widget = GPXWidget::create(app_, source);
		break;
	case VideoWidget::WidgetGrade:
		widget = GradeWidget::create(app_, source);
		break;
	case VideoWidget::WidgetCourse:
		widget = CourseWidget::create(app_, source);
		break;
	case VideoWidget::WidgetHeading:
		widget = HeadingWidget::create(app_, source);
		break;
	case VideoWidget::WidgetHeartRate:
		widget = HeartRateWidget::create(app_, source);
		break;
	case VideoWidget::WidgetHomeDistance:
		widget = HomeDistanceWidget::create(app_, source);
		break;
	case VideoWidget::WidgetImage:
		widget = ImageWidget::create(app_, source);
		break;
	case VideoWidget::WidgetLap:
		widget = LapWidget::create(app_, source);
		break;
	case VideoWidget::WidgetMaxSpeed: 
		widget = MaxSpeedWidget::create(app_, source);
		break;
	case VideoWidget::WidgetPosition: 
		widget = PositionWidget::create(app_, source);
		break;
	case VideoWidget::WidgetPower: 
		widget = PowerWidget::create(app_, source);
		break;
	case VideoWidget::WidgetSpeed: 
		widget = SpeedWidget::create(app_, source);
		break;
	case VideoWidget::WidgetTemperature:
		widget = TemperatureWidget::create(app_, source);
		break;
	case VideoWidget::WidgetText:
		widget = TextWidget::create(app_, source);
		break;
	case VideoWidget::WidgetTime:
		widget = TimeWidget::create(app_, source);
		break;
	case VideoWidget::WidgetVerticalSpeed:
		widget = VerticalSpeedWidget::create(app_, source);
		break;
	case VideoWidget::WidgetMap: {
			MapSettings mapSettings;
			widget = Map::create(app_, mapSettings, source);
		}
		break;
	case VideoWidget::WidgetTrack: {
			TrackSettings trackSettings;
			widget = Track::create(app_, trackSettings, source);
		}
		break;
	default:
		log_error("Widget unknown type, '%s' type unknown", VideoWidget::widget2string(type).c_str());
		break;
	}

	// Append
	if (widget != NULL) {
		app_.append(widget);

		this->append(widget);
	}

	return widget;
}


void Renderer::rotate(OIIO::ImageBuf *buf, int orientation) {
	switch (orientation) {
	case 180:
	case -180:
		OIIO::ImageBufAlgo::rotate180(*buf, *buf);
		break;

	case 90:
	case -270:
		OIIO::ImageBufAlgo::rotate270(*buf, *buf);
		break;

	case -90:
	case 270:
		OIIO::ImageBufAlgo::rotate90(*buf, *buf);
		break;

	default:
		break;
	}
}


void Renderer::resize(OIIO::ImageBuf *buf, int width, int height) {
	const OIIO::ImageSpec& spec = buf->spec();
	OIIO::TypeDesc::BASETYPE type = (OIIO::TypeDesc::BASETYPE) spec.format.basetype;

	// If no change, skip
	if ((width == spec.width) && (height == spec.height))
		return;

	OIIO::ImageBuf out(OIIO::ImageSpec(width, height, spec.nchannels, type));
	OIIO::ImageBufAlgo::resize(out, *buf);

	*buf = out;
}


void Renderer::add(OIIO::ImageBuf *frame, int x, int y, const char *picto, const char *label, const char *value, double divider) {
	int w, h;

	// Open picto
	auto img = OIIO::ImageInput::open(picto);
	const OIIO::ImageSpec& spec = img->spec();
	VideoParams::Format img_fmt = OIIOUtils::getFormatFromOIIOBaseType((OIIO::TypeDesc::BASETYPE) spec.format.basetype);
	OIIO::TypeDesc::BASETYPE type = OIIOUtils::getOIIOBaseTypeFromFormat(img_fmt);

	OIIO::ImageBuf *buf = new OIIO::ImageBuf(OIIO::ImageSpec(spec.width, spec.height, spec.nchannels, type)); //, OIIO::InitializePixels::No);
	img->read_image(img->current_subimage(), img->current_miplevel(), 0, -1, type, buf->localpixels());

	// Resize picto
	OIIO::ImageBuf dst(OIIO::ImageSpec(spec.width * divider, spec.height * divider, spec.nchannels, type)); //, OIIO::InitializePixels::No);
	OIIO::ImageBufAlgo::resize(dst, *buf);

	w = spec.width * divider;
	h = spec.height * divider;

	// Image over
	dst.specmod().x = x;
	dst.specmod().y = y;
	OIIO::ImageBufAlgo::over(*frame, dst, *frame, OIIO::ROI());

	delete buf;


	// Add label
	int pt;
	int space = h / 10;
	pt = (h - 3 * space) / 3;
	float white[] = { 1.0, 1.0, 1.0, 1.0 };
	if (OIIO::ImageBufAlgo::render_text(*frame, x + w + (w/10), y + space + pt, label, pt, "./assets/fonts/Helvetica.ttf", white) == false)
		fprintf(stderr, "render text error\n");
	pt *= 2;
	if (OIIO::ImageBufAlgo::render_text(*frame, x + w + (w/10), y + h - (pt/2), value, pt, "./assets/fonts/Helvetica.ttf", white) == false)
		fprintf(stderr, "render text error\n");
}

