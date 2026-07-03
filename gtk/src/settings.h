#ifndef __GPX2VIDEO__GTK__SETTINGS_H__
#define __GPX2VIDEO__GTK__SETTINGS_H__

#include <string>

#include <glibmm/object.h>

#include "../../src/videowidget.h"


class GPX2VideoSettings : public Glib::Object {
public:
	enum Section {
		SectionNone = -1,
		SectionVideo,
		SectionWidget,
		SectionUnknown
	};

	static GPX2VideoSettings * handle(void);

	VideoWidget::Theme& widget(void) {
		return widget_;
	}

	static std::string section2string(Section section);

private:
	static GPX2VideoSettings *instance_;

	VideoWidget::Theme widget_;

	GPX2VideoSettings();
	~GPX2VideoSettings();
};

#endif
