#ifndef __GPX2VIDEO__GTK__WIDGETBASESETTINGSBOX_H__
#define __GPX2VIDEO__GTK__WIDGETBASESETTINGSBOX_H__

#include <gtkmm/builder.h>
#include <gtkmm/box.h>
#include "../log.h"


class GPX2VideoWidgetBaseSettingsBox : public Gtk::Box {
public:
	GPX2VideoWidgetBaseSettingsBox(const std::string &name)
		: Glib::ObjectBase(name)
		, ref_builder_(NULL) {
		loading_ = false;
	}

	GPX2VideoWidgetBaseSettingsBox(BaseObjectType *cobject,
			const Glib::RefPtr<Gtk::Builder> &ref_builder, std::string name, std::string resource_file)
		: Glib::ObjectBase(name)
		, Gtk::Box(cobject)
		, ref_builder_(ref_builder) 
		, resource_file_(resource_file) {
		loading_ = false;
	}

	virtual ~GPX2VideoWidgetBaseSettingsBox() {
		ref_builder_ = NULL;
	}

	void release(void) {
		ref_builder_ = NULL;
	}

	virtual void update_content(void) = 0;

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

	const std::string resource_file_;

	bool loading_;
};

#endif

