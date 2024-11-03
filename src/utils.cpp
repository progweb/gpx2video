#include <algorithm>
#include <string>

#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

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


int mkpath(std::string &path, mode_t mode) {
	char *p; 
	char path_[PATH_MAX];

	const size_t len = path.size();

	errno = 0;

	if (len > sizeof(path_)-1) {
		errno = ENAMETOOLONG;
		return -1; 
	}

	strncpy(path_, path.c_str(), sizeof(path_));
	path_[sizeof(path_) - 1] = '\0';

	// Iterate the string
	for (p = path_ + 1; *p; p++) {
		if (*p == '/') {
			// Temporarily truncate
			*p = '\0';

			if (::mkdir(path_, mode) != 0) {
				if (errno != EEXIST)
					return -1; 
			}

			*p = '/';
		}
	}   

	if (::mkdir(path_, mode) != 0) {
		if (errno != EEXIST)
			return -1; 
	}   

	return 0;
}


void rmpath(std::string path) {
	DIR *dir = NULL;

	struct dirent *entry = NULL;

	dir = ::opendir(path.c_str());

	while ((entry = ::readdir(dir)) != NULL) {
		FILE *file = NULL;
		DIR *sub_dir = NULL;

		char path_[PATH_MAX];

		if ((*(entry->d_name) != '.') || ((strlen(entry->d_name) > 1) && (entry->d_name[1] != '.'))) {   
			snprintf(path_, sizeof(path_), "%s/%s", path.c_str(), entry->d_name);
			path_[sizeof(path_) - 1] = '\0';

			if ((sub_dir = ::opendir(path_)) != NULL) {   
				::closedir(sub_dir);
				rmpath(path_);
			}
			else {   
				if ((file = ::fopen(path_, "r")) != NULL) {
					::fclose(file);
					remove(path_);
				}
			}   
		}
	}   

	::remove(path.c_str());
}

