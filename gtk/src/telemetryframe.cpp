#include <glibmm/i18n.h>

#include <gtkmm/combobox.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>

#include "log_i.h"
#include "../../src/telemetrysettings.h"
#include "telemetryframe.h"



GPX2VideoTelemetryFrame::GPX2VideoTelemetryFrame()
	: Glib::ObjectBase("GPX2VideoTelemetryFrame") 
	, dispatcher_()
	, source_(NULL) {
	log_call();

	loading_ = false;
	is_visible_ = false;
}


GPX2VideoTelemetryFrame::GPX2VideoTelemetryFrame(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder) 
	: Glib::ObjectBase("GPX2VideoTelemetryFrame")
	, Gtk::Frame(cobject)
	, ref_builder_(ref_builder)
	, dispatcher_()
	, source_(NULL) {
	log_call();

	loading_ = false;
	is_visible_ = false;

	// Populate models
	load_models();

	// Binding
	bind_content();

	// Update ui
	update_content();
	update_boundaries();
}


GPX2VideoTelemetryFrame::~GPX2VideoTelemetryFrame() {
	log_call();
}


void GPX2VideoTelemetryFrame::load_models(void) {
	log_call();

	Glib::RefPtr<Gtk::ListStore> smooth_method_model;

	prediction_method_model_ = Gtk::ListStore::create(model_);

	{
		auto iter = prediction_method_model_->append();
		auto row = *iter;
		row[model_.m_id] = TelemetrySettings::MethodNone;
		row[model_.m_name] = _("None");
		row[model_.m_enable] = true;

		row = *(prediction_method_model_->append());
		row[model_.m_id] = TelemetrySettings::MethodSample;
		row[model_.m_name] = _("Sample");
		row[model_.m_enable] = true;

		row = *(prediction_method_model_->append());
		row[model_.m_id] = TelemetrySettings::MethodLinear;
		row[model_.m_name] = _("Linear");
		row[model_.m_enable] = true;

		row = *(prediction_method_model_->append());
		row[model_.m_id] = TelemetrySettings::MethodInterpolate;
		row[model_.m_name] = _("Interpolate");
		row[model_.m_enable] = true;
	}

	smooth_method_model = Gtk::ListStore::create(model_);

	{
		auto iter = smooth_method_model->append();
		auto row = *iter;
		row[model_.m_id] = TelemetrySettings::SmoothNone;
		row[model_.m_name] = _("None");
		row[model_.m_enable] = true;

		row = *(smooth_method_model->append());
		row[model_.m_id] = TelemetrySettings::SmoothWindowedMovingAverage;
		row[model_.m_name] = _("Windowed moving average");
		row[model_.m_enable] = true;

		row = *(smooth_method_model->append());
		row[model_.m_id] = TelemetrySettings::SmoothSavitzkyGolay;
		row[model_.m_name] = _("Savitzky & Golay");
		row[model_.m_enable] = true;

		row = *(smooth_method_model->append());
		row[model_.m_id] = TelemetrySettings::SmoothButterworth;
		row[model_.m_name] = _("Butterworth");
		row[model_.m_enable] = true;
	}

	position_smooth_method_model_ = duplicate_liststore(smooth_method_model, model_);
	grade_smooth_method_model_ = duplicate_liststore(smooth_method_model, model_);
	course_smooth_method_model_ = duplicate_liststore(smooth_method_model, model_);
	heading_smooth_method_model_ = duplicate_liststore(smooth_method_model, model_);
	elevation_smooth_method_model_ = duplicate_liststore(smooth_method_model, model_);
	acceleration_smooth_method_model_ = duplicate_liststore(smooth_method_model, model_);
	speed_smooth_method_model_ = duplicate_liststore(smooth_method_model, model_);
	verticalspeed_smooth_method_model_ = duplicate_liststore(smooth_method_model, model_);
}


void GPX2VideoTelemetryFrame::set_telemetry(TelemetrySource *source) {
	log_call();

	// Save telemetry settings handle
	source_ = source;

	// Update ui content
	update_content();
	update_boundaries();
}


void GPX2VideoTelemetryFrame::set_visible(bool visible) {
	log_call();

	is_visible_ = visible;

	update_content();
	update_boundaries();
}


Glib::RefPtr<Gtk::ListStore> GPX2VideoTelemetryFrame::duplicate_liststore(const Glib::RefPtr<Gtk::ListStore> &source, class Model &columns) {
	log_call();

	Glib::RefPtr<Gtk::ListStore> destination = Gtk::ListStore::create(columns);

	for (const auto& item : source->children()) {
		auto row = *(destination->append());

		row[columns.m_id] = item[columns.m_id];
		row[columns.m_name] = item[columns.m_name];
		row[columns.m_enable] = item[columns.m_enable];
	}
	
	return destination;
}


bool GPX2VideoTelemetryFrame::find_in_listtore(const Glib::RefPtr<Gtk::ListStore> &store, const int &value, Gtk::TreeModel::iterator &result) {
	log_call();

	for (auto iter = store->children().begin(); iter != store->children().end(); iter++) {
		if (iter->get_value(model_.m_id) != value)
			continue;

		result = iter;

		return true;
	}

	return false;
}


void GPX2VideoTelemetryFrame::bind_content(void) {
	log_call();

	Gtk::ComboBox *combobox;
	Gtk::SpinButton *spinbutton;
	Gtk::CheckButton *checkbutton;

	auto renderer = Gtk::make_managed<Gtk::CellRendererText>();

	// Connect widgets button
	//------------------------

	// Time offset
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("timeoffset_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"timeoffset_spinbutton\" object in telemetry_frame.ui");

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Telemetry time offset changed to '%d' ms", value);

						source_->settings().setTelemetryOffset(value);
					}
			));

	// Drop invalid point
	checkbutton = ref_builder_->get_widget<Gtk::CheckButton>("dropinvalid_checkbutton");
	if (!checkbutton)
		throw std::runtime_error("No \"dropinvalid_checkbutton\" object in telemetry_frame.ui");

	checkbutton->signal_toggled().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_checkbutton_toggled), checkbutton, 
					[this](const bool active) {
						log_notice("Telemetry skip bad point toggled to '%s' ms", std::to_string(active).c_str());

						source_->settings().setTelemetryCheck(active);
					}
			));

	// Pause detection
	checkbutton = ref_builder_->get_widget<Gtk::CheckButton>("pausedetection_checkbutton");
	if (!checkbutton)
		throw std::runtime_error("No \"pausedetection_checkbutton\" object in telemetry_frame.ui");

	checkbutton->signal_toggled().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_checkbutton_toggled), checkbutton, 
					[this](const bool active) {
						log_notice("Telemetry pause detection toggled to '%s' ms", std::to_string(active).c_str());

						source_->settings().setTelemetryPauseDetection(active);
					}
			));

	// 'Iglewicz & Hoaglin' modified z-scores filter
	checkbutton = ref_builder_->get_widget<Gtk::CheckButton>("zscorefilter_checkbutton");
	if (!checkbutton)
		throw std::runtime_error("No \"zscorefilter_checkbutton\" object in telemetry_frame.ui");

	checkbutton->signal_toggled().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_checkbutton_toggled), checkbutton, 
					[this](const bool active) {
						log_notice("Telemetry filter toggled to '%s' ms", std::to_string(active).c_str());

						source_->settings().setTelemetryFilter(
								active ? TelemetrySettings::FilterOutlier : TelemetrySettings::FilterNone);
					}
			));

	// Prediction method
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("prediction_method_combobox");
	if (!combobox)
		throw std::runtime_error("No \"prediction_method_combobox\" object in telemetry_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						int rate = source_->settings().telemetryRate();

						log_notice("Telemetry prediction method changed to '%s'", 
								iter->get_value(model_.m_name).c_str());

						source_->settings().setTelemetryMethod((TelemetrySettings::Method) value, rate);

						// Update
						update_boundaries();
					}
			));

	// Rate
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("rate_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"rate_spinbutton\" object in telemetry_frame.ui");

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_spin_changed), spinbutton, 
					[this](const int &value) {
						TelemetrySettings::Method method = source_->settings().telemetryMethod();

						log_notice("Telemetry rate value changed to '%d' ms", value);

						source_->settings().setTelemetryMethod(method, value);
					}
			));

	// Position smooth method
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("position_method_combobox");
	if (!combobox)
		throw std::runtime_error("No \"position_method_combobox\" object in telemetry_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Telemetry smooth method changed to '%s' for position data", 
								iter->get_value(model_.m_name).c_str());

						source_->settings().setTelemetrySmoothMethod(TelemetryData::DataPosition, (TelemetrySettings::Smooth) value);

						// Update
						update_boundaries();
					}
			));

	// Position window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("position_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"position_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Telemetry window size value changed to '%d' for position data", value);

						source_->settings().setTelemetrySmoothPoints(TelemetryData::DataPosition, value);

						// Update
						update_boundaries();
					}
			));

	// Position order
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("position_order_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"position_order_spinbutton\" object in telemetry_frame.ui");

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Telemetry order value changed to '%d' for position data", value);

						source_->settings().setTelemetrySmoothOrder(TelemetryData::DataPosition, value);

						// Update
						update_boundaries();
					}
			));

	// Grade smooth method
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("grade_method_combobox");
	if (!combobox)
		throw std::runtime_error("No \"grade_method_combobox\" object in telemetry_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Telemetry smooth method changed to '%s' for grade data", 
								iter->get_value(model_.m_name).c_str());

						source_->settings().setTelemetrySmoothMethod(TelemetryData::DataGrade, (TelemetrySettings::Smooth) value);

						// Update
						update_boundaries();
					}
			));

	// Grade window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("grade_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"grade_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Telemetry window size value changed to '%d' for grade data", value);

						source_->settings().setTelemetrySmoothPoints(TelemetryData::DataGrade, value);

						// Update
						update_boundaries();
					}
			));

	// Grade order
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("grade_order_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"grade_order_spinbutton\" object in telemetry_frame.ui");

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Telemetry order value changed to '%d' for grade data", value);

						source_->settings().setTelemetrySmoothOrder(TelemetryData::DataGrade, value);
					}
			));

	// Course smooth method
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("course_method_combobox");
	if (!combobox)
		throw std::runtime_error("No \"course_method_combobox\" object in telemetry_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Telemetry smooth method changed to '%s' for course data", 
								iter->get_value(model_.m_name).c_str());

						source_->settings().setTelemetrySmoothMethod(TelemetryData::DataCourse, (TelemetrySettings::Smooth) value);

						// Update
						update_boundaries();
					}
			));

	// Course window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("course_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"course_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Telemetry window size value changed to '%d' for course data", value);

						source_->settings().setTelemetrySmoothPoints(TelemetryData::DataCourse, value);

						// Update
						update_boundaries();
					}
			));

	// Heading smooth method
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("heading_method_combobox");
	if (!combobox)
		throw std::runtime_error("No \"heading_method_combobox\" object in telemetry_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Telemetry smooth method changed to '%s' for heading data", 
								iter->get_value(model_.m_name).c_str());

						source_->settings().setTelemetrySmoothMethod(TelemetryData::DataHeading, (TelemetrySettings::Smooth) value);

						// Update
						update_boundaries();
					}
			));

	// Heading window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("heading_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"heading_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Telemetry window size value changed to '%d' for heading data", value);

						source_->settings().setTelemetrySmoothPoints(TelemetryData::DataHeading, value);

						// Update
						update_boundaries();
					}
			));

	// Elevation smooth method
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("elevation_method_combobox");
	if (!combobox)
		throw std::runtime_error("No \"elevation_method_combobox\" object in telemetry_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Telemetry smooth method changed to '%s' for elevation data", 
								iter->get_value(model_.m_name).c_str());

						source_->settings().setTelemetrySmoothMethod(TelemetryData::DataElevation, (TelemetrySettings::Smooth) value);

						// Update
						update_boundaries();
					}
			));

	// Elevation window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("elevation_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"elevation_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Telemetry window size value changed to '%d' for elevation data", value);

						source_->settings().setTelemetrySmoothPoints(TelemetryData::DataElevation, value);

						// Update
						update_boundaries();
					}
			));

	// Elevation order
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("elevation_order_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"elevation_order_spinbutton\" object in telemetry_frame.ui");

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Telemetry order value changed to '%d' for elevation data", value);

						source_->settings().setTelemetrySmoothOrder(TelemetryData::DataElevation, value);

						// Update
						update_boundaries();
					}
			));

	// Acceleration smooth method
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("acceleration_method_combobox");
	if (!combobox)
		throw std::runtime_error("No \"acceleration_method_combobox\" object in telemetry_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Telemetry smooth method changed to '%s' for acceleration data", 
								iter->get_value(model_.m_name).c_str());

						source_->settings().setTelemetrySmoothMethod(TelemetryData::DataAcceleration, (TelemetrySettings::Smooth) value);

						// Update
						update_boundaries();
					}
			));

	// Acceleration window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("acceleration_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"acceleration_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Telemetry window size value changed to '%d' for acceleration data", value);

						source_->settings().setTelemetrySmoothPoints(TelemetryData::DataAcceleration, value);

						// Update
						update_boundaries();
					}
			));

	// Acceleration order
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("acceleration_order_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"acceleration_order_spinbutton\" object in telemetry_frame.ui");

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Telemetry order value changed to '%d' for acceleration data", value);

						source_->settings().setTelemetrySmoothOrder(TelemetryData::DataAcceleration, value);

						// Update
						update_boundaries();
					}
			));

	// Speed smooth method
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("speed_method_combobox");
	if (!combobox)
		throw std::runtime_error("No \"speed_method_combobox\" object in telemetry_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Telemetry smooth method changed to '%s' for speed data", 
								iter->get_value(model_.m_name).c_str());

						source_->settings().setTelemetrySmoothMethod(TelemetryData::DataSpeed, (TelemetrySettings::Smooth) value);

						// Update
						update_boundaries();
					}
			));

	// Speed window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("speed_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"speed_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Telemetry window size value changed to '%d' for speed data", value);

						source_->settings().setTelemetrySmoothPoints(TelemetryData::DataSpeed, value);

						// Update
						update_boundaries();
					}
			));

	// Speed order
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("speed_order_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"speed_order_spinbutton\" object in telemetry_frame.ui");

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Telemetry order value changed to '%d' for speed data", value);

						source_->settings().setTelemetrySmoothOrder(TelemetryData::DataSpeed, value);

						// Update
						update_boundaries();
					}
			));

	// Vertical speed smooth method
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("verticalspeed_method_combobox");
	if (!combobox)
		throw std::runtime_error("No \"verticalspeed_method_combobox\" object in telemetry_frame.ui");

	combobox->pack_start(*renderer, true);
	combobox->add_attribute(renderer->property_text(), model_.m_name);
	combobox->add_attribute(renderer->property_sensitive(), model_.m_enable);

	combobox->signal_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_combobox_changed), combobox, 
					[this](const Gtk::TreeModel::const_iterator &iter) {
						int value = iter->get_value(model_.m_id);

						log_notice("Telemetry smooth method changed to '%s' for vertical speed data", 
								iter->get_value(model_.m_name).c_str());

						source_->settings().setTelemetrySmoothMethod(TelemetryData::DataVerticalSpeed, (TelemetrySettings::Smooth) value);

						// Update
						update_boundaries();
					}
			));

	// Vertical speed window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("verticalspeed_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"verticalspeed_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Telemetry window size value changed to '%d' for vertical speed data", value);

						source_->settings().setTelemetrySmoothPoints(TelemetryData::DataVerticalSpeed, value);

						// Update
						update_boundaries();
					}
			));

	// Vertical speed order
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("verticalspeed_order_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"verticalspeed_order_spinbutton\" object in telemetry_frame.ui");

	spinbutton->signal_value_changed().connect(sigc::bind(
				sigc::mem_fun(*this, &GPX2VideoTelemetryFrame::on_telemetry_spin_changed), spinbutton, 
					[this](const int &value) {
						log_notice("Telemetry order value changed to '%d' for vertical speed data", value);

						source_->settings().setTelemetrySmoothOrder(TelemetryData::DataVerticalSpeed, value);

						// Update
						update_boundaries();
					}
			));
}


void GPX2VideoTelemetryFrame::update_content(void) {
	log_call();

	Gtk::ComboBox *combobox;
	Gtk::SpinButton *spinbutton;
	Gtk::CheckButton *checkbutton;

	Gtk::TreeModel::iterator iter;

	// Frame is visible only as telemetry source is defined
	Gtk::Frame::set_visible(is_visible_ && (source_ != NULL));

	// No telemetry source
	if (source_ == NULL)
		return;

	log_info("Load telemetry settings...");

	// Mask value changed
	loading_ = true;

	// Time offset
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("timeoffset_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"timeoffset_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_value(source_->settings().telemetryOffset());

	// Drop invalid point
	checkbutton = ref_builder_->get_widget<Gtk::CheckButton>("dropinvalid_checkbutton");
	if (!checkbutton)
		throw std::runtime_error("No \"dropinvalid_checkbutton\" object in telemetry_frame.ui");

	checkbutton->set_active(source_->settings().telemetryCheck());

	// Pause detection
	checkbutton = ref_builder_->get_widget<Gtk::CheckButton>("pausedetection_checkbutton");
	if (!checkbutton)
		throw std::runtime_error("No \"pausedetection_checkbutton\" object in telemetry_frame.ui");

	checkbutton->set_active(source_->settings().telemetryPauseDetection());

	// 'Iglewicz & Hoaglin' modified z-scores filter
	checkbutton = ref_builder_->get_widget<Gtk::CheckButton>("zscorefilter_checkbutton");
	if (!checkbutton)
		throw std::runtime_error("No \"zscorefilter_checkbutton\" object in telemetry_frame.ui");

	checkbutton->set_active((source_->settings().telemetryFilter() == TelemetrySettings::FilterOutlier));

	// Prediction method
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("prediction_method_combobox");
	if (!combobox)
		throw std::runtime_error("No \"prediction_method_combobox\" object in telemetry_frame.ui");

	combobox->set_model(prediction_method_model_);

	if (find_in_listtore(prediction_method_model_, source_->settings().telemetryMethod(), iter))
		combobox->set_active(iter);

	// Rate
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("rate_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"rate_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_value(source_->settings().telemetryRate());

	// Position smooth method
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("position_method_combobox");
	if (!combobox)
		throw std::runtime_error("No \"position_method_combobox\" object in telemetry_frame.ui");

	combobox->set_model(position_smooth_method_model_);

	if (find_in_listtore(position_smooth_method_model_, source_->settings().telemetrySmoothMethod(TelemetryData::DataPosition), iter))
		combobox->set_active(iter);

	// Position window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("position_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"position_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_value(source_->settings().telemetrySmoothPoints(TelemetryData::DataPosition));

	// Position order
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("position_order_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"position_order_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_value(source_->settings().telemetrySmoothOrder(TelemetryData::DataPosition));

	// Grade smooth method
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("grade_method_combobox");
	if (!combobox)
		throw std::runtime_error("No \"grade_method_combobox\" object in telemetry_frame.ui");

	combobox->set_model(grade_smooth_method_model_);

	if (find_in_listtore(grade_smooth_method_model_, source_->settings().telemetrySmoothMethod(TelemetryData::DataGrade), iter))
		combobox->set_active(iter);

	// Grade window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("grade_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"grade_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_value(source_->settings().telemetrySmoothPoints(TelemetryData::DataGrade));

	// Grade order
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("grade_order_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"grade_order_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_value(source_->settings().telemetrySmoothOrder(TelemetryData::DataGrade));

	// Course smooth method
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("course_method_combobox");
	if (!combobox)
		throw std::runtime_error("No \"course_method_combobox\" object in telemetry_frame.ui");

	combobox->set_model(course_smooth_method_model_);

	if (find_in_listtore(course_smooth_method_model_, source_->settings().telemetrySmoothMethod(TelemetryData::DataCourse), iter))
		combobox->set_active(iter);

	// Course window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("course_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"course_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_value(source_->settings().telemetrySmoothPoints(TelemetryData::DataCourse));

	// Heading smooth method
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("heading_method_combobox");
	if (!combobox)
		throw std::runtime_error("No \"heading_method_combobox\" object in telemetry_frame.ui");

	combobox->set_model(heading_smooth_method_model_);

	if (find_in_listtore(heading_smooth_method_model_, source_->settings().telemetrySmoothMethod(TelemetryData::DataHeading), iter))
		combobox->set_active(iter);

	// Heading window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("heading_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"heading_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_value(source_->settings().telemetrySmoothPoints(TelemetryData::DataHeading));

	// Elevation smooth method
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("elevation_method_combobox");
	if (!combobox)
		throw std::runtime_error("No \"elevation_method_combobox\" object in telemetry_frame.ui");

	combobox->set_model(elevation_smooth_method_model_);

	if (find_in_listtore(elevation_smooth_method_model_, source_->settings().telemetrySmoothMethod(TelemetryData::DataElevation), iter))
		combobox->set_active(iter);

	// Elevation window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("elevation_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"elevation_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_value(source_->settings().telemetrySmoothPoints(TelemetryData::DataElevation));

	// Elevation order
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("elevation_order_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"elevation_order_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_value(source_->settings().telemetrySmoothOrder(TelemetryData::DataElevation));

	// Acceleration smooth method
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("acceleration_method_combobox");
	if (!combobox)
		throw std::runtime_error("No \"acceleration_method_combobox\" object in telemetry_frame.ui");

	combobox->set_model(acceleration_smooth_method_model_);

	if (find_in_listtore(acceleration_smooth_method_model_, source_->settings().telemetrySmoothMethod(TelemetryData::DataAcceleration), iter))
		combobox->set_active(iter);

	// Acceleration window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("acceleration_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"acceleration_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_value(source_->settings().telemetrySmoothPoints(TelemetryData::DataAcceleration));

	// Acceleration order
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("acceleration_order_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"acceleration_order_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_value(source_->settings().telemetrySmoothOrder(TelemetryData::DataAcceleration));

	// Speed smooth method
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("speed_method_combobox");
	if (!combobox)
		throw std::runtime_error("No \"speed_method_combobox\" object in telemetry_frame.ui");

	combobox->set_model(speed_smooth_method_model_);

	if (find_in_listtore(speed_smooth_method_model_, source_->settings().telemetrySmoothMethod(TelemetryData::DataSpeed), iter))
		combobox->set_active(iter);

	// Speed window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("speed_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"speed_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_value(source_->settings().telemetrySmoothPoints(TelemetryData::DataSpeed));

	// Speed order
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("speed_order_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"speed_order_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_value(source_->settings().telemetrySmoothOrder(TelemetryData::DataSpeed));

	// Vertical speed smooth method
	combobox = ref_builder_->get_widget<Gtk::ComboBox>("verticalspeed_method_combobox");
	if (!combobox)
		throw std::runtime_error("No \"verticalspeed_method_combobox\" object in telemetry_frame.ui");

	combobox->set_model(verticalspeed_smooth_method_model_);

	if (find_in_listtore(verticalspeed_smooth_method_model_, source_->settings().telemetrySmoothMethod(TelemetryData::DataVerticalSpeed), iter))
		combobox->set_active(iter);

	// Vertical speed window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("verticalspeed_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"verticalspeed_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_value(source_->settings().telemetrySmoothPoints(TelemetryData::DataVerticalSpeed));

	// Vertical speed order
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("verticalspeed_order_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"verticalspeed_order_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_value(source_->settings().telemetrySmoothOrder(TelemetryData::DataVerticalSpeed));

	log_info("Telemetry settings loaded");

	// Unmask value changed
	loading_ = false;
}


void GPX2VideoTelemetryFrame::update_boundaries(void) {
	log_call();

	int nbr_points;

	enum TelemetrySettings::Smooth method;

	Gtk::SpinButton *spinbutton;

	Gtk::TreeModel::iterator iter;

	// No telemetry source
	if (source_ == NULL)
		return;

	// Number of points
	nbr_points = source_->numberOfPoints();

	// Position smooth method
	method = source_->settings().telemetrySmoothMethod(TelemetryData::DataPosition);

	// Position window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("position_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"position_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_range(0, nbr_points);
	spinbutton->set_sensitive((method == TelemetrySettings::SmoothWindowedMovingAverage) || (method == TelemetrySettings::SmoothSavitzkyGolay));

	// Position order
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("position_order_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"position_order_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_range(0, source_->settings().telemetrySmoothPoints(TelemetryData::DataPosition));
	spinbutton->set_sensitive(method == TelemetrySettings::SmoothSavitzkyGolay);

	// Grade smooth method
	method = source_->settings().telemetrySmoothMethod(TelemetryData::DataGrade);

	// Grade window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("grade_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"grade_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_range(0, nbr_points);
	spinbutton->set_sensitive((method == TelemetrySettings::SmoothWindowedMovingAverage) || (method == TelemetrySettings::SmoothSavitzkyGolay));

	// Grade order
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("grade_order_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"grade_order_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_range(0, source_->settings().telemetrySmoothPoints(TelemetryData::DataGrade));
	spinbutton->set_sensitive(method == TelemetrySettings::SmoothSavitzkyGolay);

	// Course smooth method
	method = source_->settings().telemetrySmoothMethod(TelemetryData::DataCourse);

	for (iter = course_smooth_method_model_->children().begin(); iter != course_smooth_method_model_->children().end(); iter++) {
		bool enable = (iter->get_value(model_.m_id) != TelemetrySettings::SmoothSavitzkyGolay);
		iter->set_value(model_.m_enable, enable);
	}

	// Course window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("course_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"course_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_range(0, nbr_points);
	spinbutton->set_sensitive((method == TelemetrySettings::SmoothWindowedMovingAverage) || (method == TelemetrySettings::SmoothSavitzkyGolay));

	// Course smooth method
	method = source_->settings().telemetrySmoothMethod(TelemetryData::DataHeading);

	for (iter = heading_smooth_method_model_->children().begin(); iter != heading_smooth_method_model_->children().end(); iter++) {
		bool enable = (iter->get_value(model_.m_id) != TelemetrySettings::SmoothSavitzkyGolay);
		iter->set_value(model_.m_enable, enable);
	}

	// Heading window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("heading_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"heading_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_range(0, nbr_points);
	spinbutton->set_sensitive((method == TelemetrySettings::SmoothWindowedMovingAverage) || (method == TelemetrySettings::SmoothSavitzkyGolay));

	// Elevation smooth method
	method = source_->settings().telemetrySmoothMethod(TelemetryData::DataElevation);

	// Elevation window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("elevation_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"elevation_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_range(0, nbr_points);
	spinbutton->set_sensitive((method == TelemetrySettings::SmoothWindowedMovingAverage) || (method == TelemetrySettings::SmoothSavitzkyGolay));

	// Elevation order
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("elevation_order_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"elevation_order_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_range(0, source_->settings().telemetrySmoothPoints(TelemetryData::DataElevation));
	spinbutton->set_sensitive(method == TelemetrySettings::SmoothSavitzkyGolay);

	// Acceleration smooth method
	method = source_->settings().telemetrySmoothMethod(TelemetryData::DataAcceleration);

	// Acceleration window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("acceleration_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"acceleration_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_range(0, nbr_points);
	spinbutton->set_sensitive((method == TelemetrySettings::SmoothWindowedMovingAverage) || (method == TelemetrySettings::SmoothSavitzkyGolay));

	// Acceleration order
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("acceleration_order_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"acceleration_order_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_range(0, source_->settings().telemetrySmoothPoints(TelemetryData::DataAcceleration));
	spinbutton->set_sensitive(method == TelemetrySettings::SmoothSavitzkyGolay);

	// Speed smooth method
	method = source_->settings().telemetrySmoothMethod(TelemetryData::DataSpeed);

	// Speed window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("speed_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"speed_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_range(0, nbr_points);
	spinbutton->set_sensitive((method == TelemetrySettings::SmoothWindowedMovingAverage) || (method == TelemetrySettings::SmoothSavitzkyGolay));

	// Speed order
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("speed_order_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"speed_order_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_range(0, source_->settings().telemetrySmoothPoints(TelemetryData::DataSpeed));
	spinbutton->set_sensitive(method == TelemetrySettings::SmoothSavitzkyGolay);

	// Vertical speed smooth method
	method = source_->settings().telemetrySmoothMethod(TelemetryData::DataVerticalSpeed);

	// Vertical speed window size
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("verticalspeed_winsize_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"verticalspeed_winsize_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_range(0, nbr_points);
	spinbutton->set_sensitive((method == TelemetrySettings::SmoothWindowedMovingAverage) || (method == TelemetrySettings::SmoothSavitzkyGolay));

	// Vertical speed order
	spinbutton = ref_builder_->get_widget<Gtk::SpinButton>("verticalspeed_order_spinbutton");
	if (!spinbutton)
		throw std::runtime_error("No \"verticalspeed_order_spinbutton\" object in telemetry_frame.ui");

	spinbutton->set_range(0, source_->settings().telemetrySmoothPoints(TelemetryData::DataVerticalSpeed));
	spinbutton->set_sensitive(method == TelemetrySettings::SmoothSavitzkyGolay);
}


void GPX2VideoTelemetryFrame::on_telemetry_spin_changed(Gtk::SpinButton *button, std::function<void(const int&)> set) {
	log_call();

	int value;

	if (loading_)
		return;

	value = button->get_value_as_int();

	// Set value
	set(value);

	// Compute
	source_->loadData(true);

	// Telemetry compute reset
	dispatcher_.emit();
}


void GPX2VideoTelemetryFrame::on_telemetry_combobox_changed(Gtk::ComboBox *combobox, std::function<void(const Gtk::TreeModel::const_iterator&)> set) {
	log_call();

	if (loading_)
		return;

	// Set combobox
	set(combobox->get_active());

	// Compute
	source_->loadData(true);

	// Telemetry compute reset
	dispatcher_.emit();
}


void GPX2VideoTelemetryFrame::on_telemetry_checkbutton_toggled(Gtk::CheckButton *button, std::function<void(const int&)> set) {
	log_call();

	bool active;

	if (loading_)
		return;

	active = button->get_active();

	// Set active
	set(active);

	// Compute
	source_->loadData(true);

	// Telemetry compute reset
	dispatcher_.emit();
}


