#ifndef __GPX2VIDEO__AUDIOPARAMS_H__
#define __GPX2VIDEO__AUDIOPARAMS_H__

#include <iostream>
#include <memory>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}


class AudioParams {
public:
	AudioParams();
	virtual ~AudioParams();
};

#endif

