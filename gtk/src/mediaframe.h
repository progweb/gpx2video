#ifndef __GPX2VIDEO__GTK__MEDIAFRAME_H__
#define __GPX2VIDEO__GTK__MEDIAFRAME_H__

#include <glibmm/dispatcher.h>

#include <gtkmm/builder.h>
#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/entry.h>
#include <gtkmm/filter.h>
#include <gtkmm/listbox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/filterlistmodel.h>

#include "media.h"


class GPX2VideoMediaFrame : public Gtk::Frame {
public:
	class Filter : public Gtk::Filter {
	public: 
		Filter();

		void set_media(GPX2VideoMedia::Media media);

	protected:
		bool match_vfunc(const Glib::RefPtr<Glib::ObjectBase> &item);

	private:
		GPX2VideoMedia::Media media_;
	};

	GPX2VideoMediaFrame();
	GPX2VideoMediaFrame(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder, 
			const Glib::RefPtr<GPX2VideoMediaListStore> &media_model);
	virtual ~GPX2VideoMediaFrame();

	void set_visible(bool visible);

	void set_media(GPX2VideoMedia::Media media);

	void update_content(void);

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

	Glib::Dispatcher dispatcher_;

	Glib::RefPtr<GPX2VideoMediaFrame::Filter> filter_;
	Glib::RefPtr<GPX2VideoMediaListStore> media_model_;
	Glib::RefPtr<Gtk::FilterListModel> media_filter_model_;

	void build(void);
	Gtk::Widget * create_row(const Glib::RefPtr<Glib::ObjectBase> &obj);

private:
	bool is_visible_;
};

#endif

