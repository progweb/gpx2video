#ifndef __GPX2VIDEO__GTK__WIDGETFRAME_H__
#define __GPX2VIDEO__GTK__WIDGETFRAME_H__

#include <gtkmm/builder.h>
#include <glibmm/dispatcher.h>
#include <gtkmm/frame.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/liststore.h>

#include "renderer.h"
#include "videowidget.h"


class GPX2VideoWidgetFrame : public Gtk::Frame {
public:
	GPX2VideoWidgetFrame();
	GPX2VideoWidgetFrame(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &ref_builder);
	virtual ~GPX2VideoWidgetFrame();

	void set_renderer(GPX2VideoRenderer *renderer);

	void set_visible(bool visible);

	GPX2VideoWidget * widget_selected(void);
	void set_widget_selected(GPX2VideoWidget *widget);

	Glib::Dispatcher& signal_widget_changed(void) {
		return dispatcher_;
	}

protected:
	class Model : public Gtk::TreeModel::ColumnRecord {
	public:
		Model() { 
			add(m_id); 
			add(m_name);
		}

		Gtk::TreeModelColumn<int> m_id;
		Gtk::TreeModelColumn<Glib::ustring> m_name;
	};

	Glib::RefPtr<Gtk::Builder> ref_builder_;

	Glib::Dispatcher dispatcher_;

	Model model_;
	Glib::RefPtr<Gtk::ListStore> align_model_;
	Glib::RefPtr<Gtk::ListStore> position_model_;

	void update_content(void);

	void on_widget_position_value_changed(void);
	void on_widget_align_value_changed(void);
	void on_widget_width_value_changed(void);
	void on_widget_height_value_changed(void);
	void on_widget_margin_value_changed(const VideoWidget::Margin &margin);
	void on_widget_padding_value_changed(const VideoWidget::Padding &padding);
	void on_widget_text_color_set(void);
	void on_widget_border_color_set(void);
	void on_widget_border_value_changed(void);
	void on_widget_background_color_set(void);

private:
	GPX2VideoRenderer *renderer_;

	GPX2VideoWidget *widget_selected_;

	bool loading_;
};

#endif

