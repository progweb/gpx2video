#include <glibmm/i18n.h>

#include "log_i.h"
#include "settings.h"


GPX2VideoSettings * GPX2VideoSettings::instance_ = NULL;


GPX2VideoSettings::GPX2VideoSettings()
	: Glib::Object() {
}


GPX2VideoSettings::~GPX2VideoSettings() {
}


GPX2VideoSettings * GPX2VideoSettings::handle(void) {
	log_call();

	if (instance_ == NULL)
		instance_ = new GPX2VideoSettings();

	return instance_;
}


std::string GPX2VideoSettings::section2string(GPX2VideoSettings::Section section) {
	switch (section) {
	case SectionVideo:
		return _("Video");
	case SectionWidget:
		return _("Widget");
	default:
		return "";
	}
}

