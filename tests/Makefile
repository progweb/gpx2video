

INC_AVUTIL=`pkg-config --cflags libavutil`
LIBS_AVUTIL=`pkg-config --libs libavutil`

INC_AVFORMAT=`pkg-config --cflags libavformat`
LIBS_AVFORMAT=`pkg-config --libs libavformat`

INC_AVCODEC=`pkg-config --cflags libavcodec`
LIBS_AVCODEC=`pkg-config --libs libavcodec`

INC_AVFILTER=`pkg-config --cflags libavfilter`
LIBS_AVFILTER=`pkg-config --libs libavfilter`

INC_SWSCALE=`pkg-config --cflags libswscale`
LIBS_SWSCALE=`pkg-config --libs libswscale`

INC=$(INC_AVUTIL) $(INC_AVFORMAT) $(INC_AVCODEC) $(INC_AVFILTER) $(INC_SWSCALE)
LIBS=$(LIBS_AVUTIL) $(LIBS_AVFORMAT) $(LIBS_AVCODEC) $(LIBS_AVFILTER) $(LIBS_SWSCALE)

all:
	gcc -g -I. $(INC) -o transcoding transcoding.c $(LIBS)
	gcc -g -I. $(INC) -o overlay main4.c $(LIBS)

