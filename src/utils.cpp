#include <algorithm>
#include <string>

#include "utils.h"


std::string replace(
		std::string sHaystack, std::string sNeedle, std::string sReplace,
		size_t nTimes) {
	size_t found = 0, pos = 0, c = 0;
	size_t len = sNeedle.size();
	size_t replen = sReplace.size();

	std::string input(sHaystack);

	do {
		found = input.find(sNeedle, pos);
		if (found == std::string::npos)
			break;

		input.replace(found, len, sReplace);
		pos = found + replen;
		++c;
	} while (!nTimes || c < nTimes);

	return input;
}

