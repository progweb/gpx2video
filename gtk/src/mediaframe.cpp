#include <iostream>
#include <exception>

#include <gtkmm.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/image.h>
#include <gtkmm/expression.h>
#include <gtkmm/stringsorter.h>

#include "log_i.h"
#include "mediaframe.h"


GPX2VideoMediaFrame::Filter::Filter()
	: Gtk::Filter()
	, media_(GPX2VideoMedia::MediaNone) {
}


void GPX2VideoMediaFrame::Filter::set_media(GPX2VideoMedia::Media media) {
	if (media == media_)
		return;

	media_ = media;

	changed();
}


bool GPX2VideoMediaFrame::Filter::match_vfunc(const Glib::RefPtr<Glib::ObjectBase> &item) {
	const auto media = std::dynamic_pointer_cast<GPX2VideoMedia>(item);

	if (media_ == GPX2VideoMedia::MediaNone)
		return true;

	if (media) {
		return media->media() == media_;
	}

	return false;
}


GPX2VideoMediaFrame::GPX2VideoMediaFrame()
	: Glib::ObjectBase("GPX2VideoMediaFrame")
	, ref_builder_(NULL) 
	, media_model_(NULL) {
	log_call();

	is_visible_ = false;
}


GPX2VideoMediaFrame::GPX2VideoMediaFrame(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder, 
		const Glib::RefPtr<GPX2VideoMediaListStore> &media_model) 
	: Glib::ObjectBase("GPX2VideoMediaFrame")
	, Gtk::Frame(cobject)
	, ref_builder_(ref_builder)
	, filter_(NULL)
	, media_model_(media_model)
	, media_filter_model_(NULL) {
	log_call();

	is_visible_ = false;

	// Populate widgets list
	build();
}


GPX2VideoMediaFrame::~GPX2VideoMediaFrame() {
	log_call();
}


void GPX2VideoMediaFrame::set_visible(bool visible) {
	log_call();

	is_visible_ = visible;

	update_content();
}


void GPX2VideoMediaFrame::set_media(GPX2VideoMedia::Media media) {
	log_call();

	filter_->set_media(media);
}


void GPX2VideoMediaFrame::build(void) {
	log_call();

	// Create model 
//	media_model_ = Gio::ListStore<GPX2VideoMedia>::create();
//	media_model_ = GPX2VideoMediaListStore::create();

	// Create the sort model
	auto expr = Gtk::ClosureExpression<Glib::ustring>::create(
		[] (const Glib::RefPtr<Glib::ObjectBase> &item) -> Glib::ustring {
			const auto media = std::dynamic_pointer_cast<GPX2VideoMedia>(item);
			return media ? media->filename() : "";
		}
	);

	auto sorter = Gtk::StringSorter::create(expr);

	// Wrap the store in a sort list model
	auto sorted_model = Gtk::SortListModel::create(media_model_, sorter);

	// Create the filter model
	filter_ = Glib::make_refptr_for_instance<GPX2VideoMediaFrame::Filter>(new GPX2VideoMediaFrame::Filter());

	auto filter_model = Gtk::FilterListModel::create(sorted_model, filter_);

	// Media list
	auto list = ref_builder_->get_widget<Gtk::ListBox>("media_listbox");
	if (!list)
		throw std::runtime_error("No \"media_listbox\" object in media_frame.ui");

	list->bind_model(filter_model, sigc::mem_fun(*this, &GPX2VideoMediaFrame::create_row));

	// Save model
	media_filter_model_ = filter_model;
}


Gtk::Widget * GPX2VideoMediaFrame::create_row(const Glib::RefPtr<Glib::ObjectBase> &obj) {
	Glib::RefPtr<const GPX2VideoMedia> media = std::dynamic_pointer_cast<const GPX2VideoMedia>(obj);
	if (!media)
		return NULL;

	std::string filename = Glib::path_get_basename(Glib::StdStringView(media->filename()));
	std::string filepath = Glib::path_get_dirname(Glib::StdStringView(media->filename()));

	auto info = Gio::File::create_for_path(media->filename())->query_info();

	// Build & append media item
	auto media_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 0);
	auto label_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 0);

	auto image = Gtk::make_managed<Gtk::Image>();
	auto filename_label = Gtk::make_managed<Gtk::Label>(filename);
	auto filepath_label = Gtk::make_managed<Gtk::Label>(filepath);
	auto filesize_label = Gtk::make_managed<Gtk::Label>(Glib::format_size(info->get_size()));

	label_box->set_valign(Gtk::Align::CENTER);
	label_box->set_spacing(12);

	image->set_halign(Gtk::Align::START);
	image->set_valign(Gtk::Align::CENTER);
	image->set_margin(8);
	image->set_pixel_size(96);
	image->set(media->filename());

	filename_label->set_halign(Gtk::Align::START);
	filename_label->set_hexpand(true);
	filename_label->set_valign(Gtk::Align::CENTER);
	filename_label->set_xalign(0.0);
	filename_label->set_yalign(1.0);

	filepath_label->set_can_focus(false);
	filepath_label->set_halign(Gtk::Align::START);
	filepath_label->set_hexpand(true);
	filepath_label->set_valign(Gtk::Align::CENTER);
	filepath_label->set_xalign(0.0);
	filepath_label->set_yalign(1.0);
	filepath_label->get_style_context()->add_class("dim-label");

	filesize_label->set_halign(Gtk::Align::START);
	filesize_label->set_hexpand(true);
	filesize_label->set_valign(Gtk::Align::CENTER);
	filesize_label->set_xalign(0.0);
	filesize_label->set_yalign(1.0);

	label_box->append(*filename_label);
	label_box->append(*filepath_label);
	label_box->append(*filesize_label);

	media_box->set_hexpand(true);
	media_box->append(*image);
	media_box->append(*label_box);

	return media_box;
}


void GPX2VideoMediaFrame::update_content(void) {
	log_call();

	// Frame is visible
	Gtk::Frame::set_visible(is_visible_);
}

