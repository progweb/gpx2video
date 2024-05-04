#ifndef __GPX2VIDEO__UTILS_H__
#define __GPX2VIDEO__UTILS_H__

#include <iostream>
#include <string>


std::string replace(
		std::string sHaystack, std::string sNeedle, std::string sReplace,
		size_t nTimes=0);

int mkpath(std::string &path, mode_t mode);

void rmpath(std:: string path);


class IndentingOStreambuf : public std::streambuf {
	std::streambuf *myDest;
	bool myIsAtStartOfLine;
	std::string myIndent;
	std::ostream *myOwner;

protected:
	virtual int overflow(int ch) {
		if (myIsAtStartOfLine && ch != '\n') {
			myDest->sputn( myIndent.data(), myIndent.size() );
		}

		myIsAtStartOfLine = ch == '\n';

		return myDest->sputc(ch);
	}

public:
	explicit IndentingOStreambuf(std::streambuf *dest, int indent = 4)
		: myDest(dest)
		, myIsAtStartOfLine(true)
		, myIndent(indent, ' ')
		, myOwner(NULL) {
	}

	explicit IndentingOStreambuf(std::ostream& dest, int indent = 4)
		: myDest(dest.rdbuf())
		, myIsAtStartOfLine(true)
		, myIndent(indent, ' ')
		, myOwner(&dest) {
		myOwner->rdbuf(this);
	}

	virtual ~IndentingOStreambuf() {
		if (myOwner != NULL)
			myOwner->rdbuf(myDest);
	}
};

#endif

