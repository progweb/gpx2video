#ifndef __GPX2VIDEO__GTK__WIDGETFRAME_H__
#define __GPX2VIDEO__GTK__WIDGETFRAME_H__

#include <gtkmm/builder.h>
#include <glibmm/dispatcher.h>
#include <gtkmm/frame.h>

#include "renderer.h"
#include "videowidget.h"


class GPX2VideoWidgetFrame : public Gtk::Frame {
public:
	GPX2VideoWidgetFrame();
	GPX2VideoWidgetFrame(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder);
	virtual ~GPX2VideoWidgetFrame();

	void set_renderer(GPX2VideoRenderer *renderer);

	void set_visible(bool visible);

	void set_widget_selected(GPX2VideoWidget *widget);

	Glib::Dispatcher& signal_widget_changed(void) {
		return dispatcher_;
	}

protected:
	Glib::RefPtr<Gtk::Builder> ref_builder_;

	Glib::Dispatcher dispatcher_;

	void update_content(void);

	void on_widget_background_color_set(void);
	void on_widget_border_color_set(void);
	void on_widget_border_value_changed(void);

private:
	GPX2VideoRenderer *renderer_;

	GPX2VideoWidget *widget_selected_;
};

#endif

