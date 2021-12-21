#ifndef __GPX2VIDEO__UTILS_H__
#define __GPX2VIDEO__UTILS_H__

#include <string>


std::string replace(
		std::string sHaystack, std::string sNeedle, std::string sReplace,
		size_t nTimes=0);

int mkpath(std::string &path, mode_t mode);

void rmpath(std:: string path);

#endif

