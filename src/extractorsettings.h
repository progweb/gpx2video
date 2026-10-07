#ifndef __GPX2VIDEO__EXTRACTORSETTINGS_H__
#define __GPX2VIDEO__EXTRACTORSETTINGS_H__

#include <iostream>
#include <string>


class ExtractorSettings {
public:
	enum Format {
		FormatNone = 0,

		FormatRAW, // Binary stream
		FormatDump, // Parse GPMF stream
		FormatText, // Parse and interpret GPMF stream
		FormatGPX, // Parse and build GPX

		FormatCount
	};

	ExtractorSettings();
	virtual ~ExtractorSettings();

	const Format& format(void) const;
	void setFormat(const Format &format);

	static const std::string getFriendlyName(const Format &format);

private:
	enum Format format_;
};


#endif

