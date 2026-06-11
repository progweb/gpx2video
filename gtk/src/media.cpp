#include <glibmm/i18n.h>

#include "log_i.h"
#include "media.h"


std::string GPX2VideoMedia::media2string(GPX2VideoMedia::Media media) {
	switch (media) {
	case MediaAudio:
		return _("Audio");
	case MediaIcon:
		return _("Icon");
	case MediaImage:
		return _("Image");
	default:
		return "";
	}
}

