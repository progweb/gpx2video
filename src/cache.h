#ifndef __GPX2VIDEO__CACHE_H__
#define __GPX2VIDEO__CACHE_H__

#include <string>

#include "gpx2video.h"


class Cache : public GPX2Video::Task {
public:
	virtual ~Cache();

	static Cache * create(GPX2Video &app);

	const std::string& path(void) const {
		return path_;
	}

	bool run(void);

private:
	GPX2Video &app_;

	Cache(GPX2Video &app);

	void init(void);

	std::string path_;
};

#endif

