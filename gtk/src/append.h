#ifndef __GPX2VIDEO__GTK__APPEND_H__
#define __GPX2VIDEO__GTK__APPEND_H__

#include <gtkmm/builder.h>
#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/filter.h>
#include <gtkmm/listbox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/filterlistmodel.h>

#include "../src/widgets.h"
#include "renderer.h"


class GPX2VideoAppend : public Gtk::Dialog {
private:
	class Filter : public Gtk::Filter {
	public: 
		Filter();

		void set_needle(const std::string &needle);

	protected:
		bool match_vfunc(const Glib::RefPtr<Glib::ObjectBase> &item);

	private:
		std::string needle_;
	};

	class Widget : public Glib::Object {
	public:
		VideoWidget::Widget id_;
		std::string icon_;
		std::string label_;

		Widget(VideoWidget::Widget id, const Glib::ustring &icon, const Glib::ustring &label) 
			: Glib::Object()
			, id_(id)
			, icon_(icon)
			, label_(label) {
		}

		static Glib::RefPtr<Widget> create(VideoWidget::Widget id, const Glib::ustring &icon, const Glib::ustring &label) {
			return Glib::make_refptr_for_instance<Widget>(new Widget(id, icon, label));
		}

		VideoWidget::Widget type(void) const {
			return id_;
		}

		const std::string& icon(void) const {
			return icon_;
		}

		const std::string& label(void) const {
			return label_;
		}
	};

public:
	GPX2VideoAppend();
	GPX2VideoAppend(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder>& ref_builder, GPX2VideoRenderer *renderer);
	virtual ~GPX2VideoAppend();

	static GPX2VideoAppend * create(Gtk::Window &parent, GPX2VideoRenderer *renderer);

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

	GPX2VideoRenderer *renderer_;

	Glib::RefPtr<GPX2VideoAppend::Filter> filter_;
	Glib::RefPtr<Gtk::FilterListModel> widget_model_;

	void build(void);
	Gtk::Widget * create_row(const Glib::RefPtr<Glib::ObjectBase> &obj);

	void on_search_entry_changed(Gtk::Entry *entry, std::function<void(const Glib::ustring&)> set);
	void on_selected(Gtk::ListBoxRow *row);
	void on_ok_clicked(void);
	void on_cancel_clicked(void);

private:
	Gtk::Button *ok_button_;
	Gtk::Button *cancel_button_;
};

#endif

