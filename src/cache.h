#ifndef __GPX2VIDEO__CACHE_H__
#define __GPX2VIDEO__CACHE_H__

#include <string>

#include "application.h"


class Cache : public GPXApplication::Task {
public:
	virtual ~Cache();

	static Cache * create(GPXApplication &app);

	const std::string& path(void) const {
		return path_;
	}

	bool run(void);

private:
	GPXApplication &app_;

	Cache(GPXApplication &app);

	void init(void);

	std::string path_;
};

#endif

