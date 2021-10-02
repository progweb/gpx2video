extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
#include <libavutil/timestamp.h>
#include <libswscale/swscale.h>
}


#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>



const int kHSVChannelCount = 3;
const int kRGBChannelCount = 3;
const int kRGBAChannelCount = 4;

enum video_format_e {
	kFormatInvalid = -1,
	kFormatUnsigned8,
	kFormatUnsigned16,
	kFormatFloat16,
	kFormatFloat32,
	kFormatCount
};

enum Interlacing {
	kInterlaceNone,
	kInterlacedTop,
	kInterlacedBottomFirst
};



int get_bytes_per_channel(enum video_format_e format) {
	switch (format) {
	case kFormatInvalid:
	case kFormatCount:
		break;
	case kFormatUnsigned8:
		return 1;
	case kFormatUnsigned16:
	case kFormatFloat16:
		return 2;
	case kFormatFloat32:
		return 4;
	}

	return  0;
}

int get_bytes_per_pixel(enum video_format_e format, int channels) {
	return get_bytes_per_channel(format) * channels;
}

size_t get_buffer_size(int width, int height, enum video_format_e format, int channels) {
	return width * height * get_bytes_per_pixel(format, channels);
}

int generate_linesize_bytes(int width, enum video_format_e format, int channels) {
	return get_bytes_per_pixel(format, channels) * ((width + 31) & ~31);
}

enum video_format_e get_compatible_pixel_format(enum video_format_e format) {
	switch (format) {
	case kFormatUnsigned8:
		return kFormatUnsigned8;
	case kFormatUnsigned16:
	case kFormatFloat16:
	case kFormatFloat32:
		return kFormatUnsigned16;
	case kFormatInvalid:
	case kFormatCount:
	default:
		break;
	}

	return kFormatInvalid;
}
enum video_format_e get_native_pixel_format(enum AVPixelFormat format) {
	switch (format) {
	case AV_PIX_FMT_RGB24:
	case AV_PIX_FMT_RGBA:
		return kFormatUnsigned8;
	case AV_PIX_FMT_RGB48:
	case AV_PIX_FMT_RGBA64:
		return kFormatUnsigned16;
	default:
		return kFormatInvalid;
	}
}

int get_native_nb_channels(enum AVPixelFormat format) {
	switch (format) {
	case AV_PIX_FMT_RGB24:
	case AV_PIX_FMT_RGB48:
		return kRGBChannelCount;
	case AV_PIX_FMT_RGBA:
	case AV_PIX_FMT_RGBA64:
		return kRGBAChannelCount;
	default:
		return 0;
	}
}

enum video_format_e get_format_from_oiio_base_type(OIIO::TypeDesc::BASETYPE type) {
	switch (type) {
	case OIIO::TypeDesc::UNKNOWN:
	case OIIO::TypeDesc::NONE:
		break;

	case OIIO::TypeDesc::INT8:
	case OIIO::TypeDesc::INT16:
	case OIIO::TypeDesc::INT32:
	case OIIO::TypeDesc::UINT32:
	case OIIO::TypeDesc::INT64:
	case OIIO::TypeDesc::UINT64:
	case OIIO::TypeDesc::STRING:
	case OIIO::TypeDesc::PTR:
	case OIIO::TypeDesc::LASTBASE:
	case OIIO::TypeDesc::DOUBLE:
		fprintf(stderr, "Tried to use unknown OIIO base type\n");
		break;

	case OIIO::TypeDesc::UINT8:
		return kFormatUnsigned8;
	case OIIO::TypeDesc::UINT16:
		return kFormatUnsigned16;
	case OIIO::TypeDesc::HALF:
		return kFormatFloat16;
	case OIIO::TypeDesc::FLOAT:
		return kFormatFloat32;
	}

	return kFormatInvalid;
}


OIIO::TypeDesc::BASETYPE get_oiio_base_type_from_format(enum video_format_e format) {
	switch (format) {
	case kFormatUnsigned8:
		return OIIO::TypeDesc::UINT8;
	case kFormatUnsigned16:
		return OIIO::TypeDesc::UINT16;
	case kFormatFloat16:
		return OIIO::TypeDesc::HALF;
	case kFormatFloat32:
		return OIIO::TypeDesc::FLOAT;
	case kFormatInvalid:
	case kFormatCount:
		break;
	}

	return OIIO::TypeDesc::UNKNOWN;
}


class Color {
public:
	double data_[4];

	Color() {
		data_[0] = 0.0;
		data_[1] = 0.0;
		data_[2] = 0.0;
		data_[3] = 0.0;
	}
	
	Color(const double &r, const double &g, const double &b, const double &a = 1.0) {
		data_[0] = r;
		data_[1] = g;
		data_[2] = b;
		data_[3] = a;
	}

	Color(const char *data, enum video_format_e format, const int n_layouts) {
		*this = fromData(data, format, n_layouts);
	}

	const double& red() { return data_[0]; }
	const double& green() { return data_[1]; }
	const double& blue() { return data_[2]; }
	const double& alpha() { return data_[3]; }

	void set_red(const double &red) { data_[0] = red; }
	void set_green(const double &green) { data_[1] = green; }
	void set_blue(const double &blue) { data_[2] = blue; }
	void set_alpha(const double &alpha) { data_[3] = alpha; }

	void toData(char *data, enum video_format_e format, int n_layout) {
		OIIO::convert_types(OIIO::TypeDesc::DOUBLE, data_, get_oiio_base_type_from_format(format), data, n_layout);
	}

	static Color fromData(const char *data, enum video_format_e format, int n_layout) {
		Color c;

		OIIO::convert_types(get_oiio_base_type_from_format(format), data, OIIO::TypeDesc::DOUBLE, c.data_, n_layout);

		return c;
	}
};


Color get_pixel(uint8_t *buffer, int x, int y, 
		int linesize, enum video_format_e format, int channels) {
	int bytes_offset = y * linesize + x * get_bytes_per_pixel(format, channels);

	return Color(buffer + bytes_offset, format, channels);
}

void set_pixel(uint8_t *buffer, int x, int y, Color &color,
		int linesize, enum video_format_e format, int channels) {
	int bytes_offset = y * linesize + x * get_bytes_per_pixel(format, channels);

	color.toData(buffer + bytes_offset, format, channels);
}





static int video_stream_index = -1;
static int audio_stream_index = -1;

static AVFormatContext *ifmt_ctx;
static AVFormatContext *ofmt_ctx;
 
typedef struct StreamContext {
    AVCodecContext *dec_ctx;
    AVCodecContext *enc_ctx;
 
    AVFrame *dec_frame;
    AVFrame *enc_frame;
    AVPacket *enc_pkt;
} StreamContext;
static StreamContext *stream_ctx;
 

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag) {
	char buf[1024];

	AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

	printf("%s: %f pts: %s pts_time: %s dts: %s dts_time: %s duration: %s duration_time: %s stream_index: %d\n", 
		tag, 
		pkt->pts * av_q2d(*time_base),
		av_ts_make_string(buf, pkt->pts), av_ts_make_time_string(buf, pkt->pts, time_base), 
		av_ts_make_string(buf, pkt->dts), av_ts_make_time_string(buf, pkt->dts, time_base), 
		av_ts_make_string(buf, pkt->duration), av_ts_make_time_string(buf, pkt->duration, time_base), 
		pkt->stream_index);
}


static int open_input_file(const char *filename)
{
    int ret;
    unsigned int i;
 
    ifmt_ctx = NULL;

  // Open file in a format context
    if ((ret = avformat_open_input(&ifmt_ctx, filename, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
 
  // Get stream information from format
    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }
 
    stream_ctx = (StreamContext *) av_mallocz_array(ifmt_ctx->nb_streams, sizeof(*stream_ctx));
    if (!stream_ctx)
        return AVERROR(ENOMEM);
 
	// For each stream
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
  		// Get reference to correct AVStream
        AVStream *stream = ifmt_ctx->streams[i];

		  // Find decoder
        const AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
        AVCodecContext *codec_ctx;
        if (!dec) {
            av_log(NULL, AV_LOG_ERROR, "Failed to find decoder for stream #%u\n", i);
//            return AVERROR_DECODER_NOT_FOUND;
			stream->discard = AVDISCARD_ALL;           
        }

  		// Allocate context for the decoder
        codec_ctx = avcodec_alloc_context3(dec);
        if (!codec_ctx) {
            av_log(NULL, AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u\n", i);
            return AVERROR(ENOMEM);
        }

		  // Copy parameters from the AVStream to the AVCodecContext
        ret = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context "
                   "for stream #%u\n", i);
            return ret;
        }

        // Reencode video & audio and remux subtitles etc.
        if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
                || codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
				video_stream_index = i;
//                codec_ctx->framerate = av_guess_frame_rate(ifmt_ctx, stream, NULL);
			}
			else
				audio_stream_index = i;

            // Open decoder
            ret = avcodec_open2(codec_ctx, dec, NULL);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
                return ret;
            }
        }

        stream_ctx[i].dec_ctx = codec_ctx;

        stream_ctx[i].dec_frame = av_frame_alloc();

        if (!stream_ctx[i].dec_frame)
            return AVERROR(ENOMEM);

        stream_ctx[i].enc_pkt = av_packet_alloc();
        if (!stream_ctx[i].enc_pkt)
            return AVERROR(ENOMEM);
    }
 
    av_dump_format(ifmt_ctx, 0, filename, 0);
    return 0;
}
 
static int open_output_file(const char *filename)
{
    AVStream *out_stream;
    AVStream *in_stream;
    AVCodecContext *dec_ctx, *enc_ctx;
    const AVCodec *encoder;
    int ret;
    unsigned int i;
 
    ofmt_ctx = NULL;
  // Create output format context
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
    if (!ofmt_ctx) {
        av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
        return AVERROR_UNKNOWN;
    }
 
 



    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        in_stream = ifmt_ctx->streams[i];
        dec_ctx = stream_ctx[i].dec_ctx;
 
//        if (stream_ctx[i].dec_ctx == NULL) {
//			continue;
//		}

		if (ifmt_ctx->streams[i]->discard == AVDISCARD_ALL) {
//			out_stream = avformat_new_stream(ofmt_ctx, NULL);
//			if (!out_stream) {
//				av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
//				return AVERROR_UNKNOWN;
//			}
//
//            enc_ctx = avcodec_alloc_context3(NULL);
//
//			avcodec_parameters_to_context(enc_ctx, in_stream->codecpar);

			continue;
		}

        out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
            return AVERROR_UNKNOWN;
        }

		if ((ifmt_ctx->streams[i]->discard != AVDISCARD_ALL) &&
				( dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
                || dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO)) {
            // in this example, we choose transcoding to same codec
  			
			// Find encoder with this name
            encoder = avcodec_find_encoder(dec_ctx->codec_id);
            if (!encoder) {
                av_log(NULL, AV_LOG_FATAL, "Necessary encoder not found\n");
                return AVERROR_INVALIDDATA;
            }


            enc_ctx = avcodec_alloc_context3(encoder);
            if (!enc_ctx) {
                av_log(NULL, AV_LOG_FATAL, "Failed to allocate the encoder context\n");
                return AVERROR(ENOMEM);
            }
 
            /* In this example, we transcode to same properties (picture size,
             * sample rate etc.). These properties can be changed for output
             * streams easily using filters */
            if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
                enc_ctx->height = dec_ctx->height;
                enc_ctx->width = dec_ctx->width;
                enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
                /* take first format from list of supported formats */
                if (encoder->pix_fmts)
                    enc_ctx->pix_fmt = encoder->pix_fmts[0];
                else
                    enc_ctx->pix_fmt = dec_ctx->pix_fmt;
                /* video time_base can be set to whatever is handy and supported by encoder */
				enc_ctx->framerate = dec_ctx->framerate;

                enc_ctx->framerate = av_guess_frame_rate(ifmt_ctx, in_stream, NULL);
                enc_ctx->time_base = av_inv_q(enc_ctx->framerate);

printf("bit_rate : %d - %d\n", dec_ctx->bit_rate, enc_ctx->bit_rate);
printf("buffer_size : %d - %d\n", dec_ctx->rc_buffer_size, enc_ctx->rc_buffer_size);
printf("max_rate : %d - %d\n", dec_ctx->rc_max_rate, enc_ctx->rc_max_rate);
printf("min_rate : %d - %d\n", dec_ctx->rc_min_rate, enc_ctx->rc_min_rate);
printf("gop_size : %d - %d\n", dec_ctx->gop_size, enc_ctx->gop_size);
printf("qmin : %d - %d\n", dec_ctx->qmin, enc_ctx->qmin);
printf("qmax : %d - %d\n", dec_ctx->qmax, enc_ctx->qmax);
enc_ctx->bit_rate = 4 * 1000 * 1000;
//enc_ctx->qmin = dec_ctx->qmin;
//enc_ctx->qmax = dec_ctx->qmax;
//enc_ctx->qcompress = 0.6;
//enc_ctx->max_qdiff = 4;
//enc_ctx->gop_size = dec_ctx->gop_size;
enc_ctx->rc_buffer_size = 4 * 1000 * 1000;
enc_ctx->rc_max_rate = 2 * 1000 * 1000;
enc_ctx->rc_min_rate = 8 * 1000 * 1000;

//				enc_ctx->flags |= AV_CODEC_FLAG_INTERLACED_DCT | AV_CODEC_FLAG_INTERLACED_ME;

//				av_opt_set(enc_ctx->priv_data, "crf", "27", AV_OPT_SEARCH_CHILDREN);
//				av_opt_set(enc_ctx->priv_data, "crf", "31", AV_OPT_SEARCH_CHILDREN);
//				av_opt_set(enc_ctx->priv_data, "x264opts", "bff=1", AV_OPT_SEARCH_CHILDREN);

            } else {
                enc_ctx->sample_rate = dec_ctx->sample_rate;
                enc_ctx->channel_layout = dec_ctx->channel_layout;
                enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);
                /* take first format from list of supported formats */
                enc_ctx->sample_fmt = encoder->sample_fmts[0];
                enc_ctx->time_base = (AVRational){1, enc_ctx->sample_rate};
            }
 
            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
 
  			// Try to open encoder (third parameter can be used to pass settings to encoder)
            ret = avcodec_open2(enc_ctx, encoder, NULL);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Cannot open video encoder for stream #%u\n", i);
                return ret;
            }

  			// Copy context settings to codecpar object
            ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Failed to copy encoder parameters to output stream #%u\n", i);
                return ret;
            }
 

			// FIXME
            out_stream->time_base = enc_ctx->time_base;
			out_stream->codecpar->codec_tag = 0;

            stream_ctx[i].enc_ctx = enc_ctx;
        } else if (dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN) {
            av_log(NULL, AV_LOG_FATAL, "Elementary stream #%d is of unknown type, cannot proceed\n", i);
            return AVERROR_INVALIDDATA;
        } else {
			// FIXME
			printf("copy params\n");
            // if this stream must be remuxed
            ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Copying parameters for stream #%u failed\n", i);
                return ret;
            }
            out_stream->time_base = in_stream->time_base;
			out_stream->codecpar->codec_tag = 0;
        }
 
    }





    av_dump_format(ofmt_ctx, 0, filename, 1);
 
  // Open output file for writing
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Could not open output file '%s'", filename);
            return ret;
        }
    }
 
    /* init muxer, write output file header */
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error occurred when opening output file\n");
        return ret;
    }
 
    return 0;
}
 
static int encode_write_frame(unsigned int stream_index, int flush)
{
    StreamContext *stream = &stream_ctx[stream_index];
    AVFrame *enc_frame = flush ? NULL : stream->enc_frame;
    AVPacket *enc_pkt = stream->enc_pkt;
    int ret;
 
//    av_log(NULL, AV_LOG_INFO, "Encoding frame\n");

	// encode dec frame
    av_packet_unref(enc_pkt);
 
    ret = avcodec_send_frame(stream->enc_ctx, enc_frame);
 
    if (ret < 0)
        return ret;
 
    while (ret >= 0) {
        ret = avcodec_receive_packet(stream->enc_ctx, enc_pkt);
 
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return 0;
    	else if (ret < 0) {
//    	  	fprintf(stderr, "Error while receiving packet from encoder: %s\n", av_err2str(ret));
      		return -1;
    	}
 
        // prepare packet for muxing
        enc_pkt->stream_index = stream_index;

        av_packet_rescale_ts(enc_pkt,
                             ifmt_ctx->streams[stream_index]->time_base,
                             ofmt_ctx->streams[stream_index]->time_base);

        av_log(NULL, AV_LOG_DEBUG, "Muxing frame\n");

//		log_packet(ofmt_ctx, enc_pkt, "out");

		// mux encoded frame
        ret = av_interleaved_write_frame(ofmt_ctx, enc_pkt);
    }
 
    return ret;
}
 
static int flush_encoder(unsigned int stream_index)
{
	if (!(stream_ctx[stream_index].enc_ctx))
		return 0;
    if (!(stream_ctx[stream_index].enc_ctx->codec->capabilities &
                AV_CODEC_CAP_DELAY))
        return 0;
 
    av_log(NULL, AV_LOG_INFO, "Flushing stream #%u encoder\n", stream_index);
    return encode_write_frame(stream_index, 1);
}
 
int main(int argc, char **argv)
{
    int ret;

	AVFrame *frame = NULL;
    AVPacket *packet = NULL;

    unsigned int stream_index;
    unsigned int i;
 
	av_log_set_level(AV_LOG_DEBUG);

    if (argc != 3) {
        av_log(NULL, AV_LOG_ERROR, "Usage: %s <input file> <output file>\n", argv[0]);
        return 1;
    }
 
	av_register_all();
	avcodec_register_all();




    if ((ret = open_input_file(argv[1])) < 0)
        goto end;
    if ((ret = open_output_file(argv[2])) < 0)
        goto end;




    AVStream *vs = ifmt_ctx->streams[video_stream_index];

	enum AVPixelFormat ideal_pix_fmt;

	int native_nb_channels;
	enum video_format_e native_pix_fmt;

	enum AVPixelFormat possible_pix_fmts[] = {
//		AV_PIX_FMT_RGB24,
		AV_PIX_FMT_RGBA,
//		AV_PIX_FMT_RGB48,
		AV_PIX_FMT_RGBA64,
		AV_PIX_FMT_NONE
	};

	struct SwsContext *sws_ctx, *sws_ctx2;

	ideal_pix_fmt = avcodec_find_best_pix_fmt_of_list(possible_pix_fmts, (AVPixelFormat) vs->codecpar->format, 1, NULL);

	// native pixel format
	native_pix_fmt = get_native_pixel_format(ideal_pix_fmt);
	native_nb_channels = get_native_nb_channels(ideal_pix_fmt);

	if (native_pix_fmt == kFormatInvalid) {
		fprintf(stderr, "Failed to find valid native pixel format for %d", ideal_pix_fmt);
	}

	fprintf(stderr,
			"Create scale context for the conversion "
			"fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
			av_get_pix_fmt_name((AVPixelFormat) vs->codecpar->format), vs->codecpar->width, vs->codecpar->height,
			av_get_pix_fmt_name(ideal_pix_fmt), vs->codecpar->width, vs->codecpar->height);

	sws_ctx = sws_getContext(vs->codecpar->width, vs->codecpar->height, (AVPixelFormat) vs->codecpar->format,
			vs->codecpar->width, vs->codecpar->height, ideal_pix_fmt,
			SWS_FAST_BILINEAR, NULL, NULL, NULL);

	if (!sws_ctx) {
        fprintf(stderr,
                "Impossible to create scale context for the conversion "
                "fmt:%s s:%dx%d -> fmt:%s s:%dx%d\n",
                av_get_pix_fmt_name((AVPixelFormat) vs->codecpar->format), vs->codecpar->width, vs->codecpar->height,
                av_get_pix_fmt_name(ideal_pix_fmt), vs->codecpar->width, vs->codecpar->height);
        ret = AVERROR(EINVAL);
        goto end;
    }


	sws_ctx2 = sws_getContext(vs->codecpar->width, vs->codecpar->height, ideal_pix_fmt,
			vs->codecpar->width, vs->codecpar->height, (AVPixelFormat) vs->codecpar->format,
			0, NULL, NULL, NULL);




    if (!(packet = av_packet_alloc()))
        goto end;

    // read all packets
    while (1) {
        if ((ret = av_read_frame(ifmt_ctx, packet)) < 0)
            break;

        stream_index = packet->stream_index;

        av_log(NULL, AV_LOG_DEBUG, "Demuxer gave frame of stream_index %u\n", stream_index);

		if (stream_index > 1) {
        	av_packet_unref(packet);
			continue;
		}


//		log_packet(ifmt_ctx, packet, "in");

       	StreamContext *stream = &stream_ctx[stream_index];

	    av_log(NULL, AV_LOG_DEBUG, "Going to reencode&filter the frame\n");


		ret = avcodec_send_packet(stream->dec_ctx, packet);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Decoding failed\n");
			break;
		}

		while (ret >= 0) {
			AVFrame *enc_frame;

			ret = avcodec_receive_frame(stream->dec_ctx, stream->dec_frame);

			if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
				break;
			else if (ret < 0)
				goto end;

			stream->dec_frame->interlaced_frame = 1;
			//stream->dec_frame->pts = stream->dec_frame->best_effort_timestamp;

//#if 0
			// Write on frame ???
			if (stream_index == video_stream_index)	{
					frame = stream->dec_frame;

					int dst_linesize = generate_linesize_bytes(frame->width, native_pix_fmt, native_nb_channels);

					size_t output_buffer_size = get_buffer_size(dst_linesize, frame->height, native_pix_fmt, native_nb_channels);

					uint8_t * output_buffer = (uint8_t *) malloc(output_buffer_size * sizeof(uint8_t));

	//				printf("stream index = %d\n", stream_index);
					printf("linesize = [%d,%d,%d] / dst_linesize = %d / height = %d\n", 
							frame->linesize[0], frame->linesize[1], frame->linesize[2], dst_linesize, frame->height);
					printf("buffsize = %d\n", output_buffer_size);

					ret = sws_scale(sws_ctx,
							(const uint8_t * const *) frame->data,
							frame->linesize,
							0,
							frame->height,
							&output_buffer,
							&dst_linesize);

					if (ret < 0) {
						av_log(NULL, AV_LOG_ERROR, "Failed to scale frame to native");
						goto end;
					}
	#if 0
					Color color(1.0, 0.0, 0.0);
					Color black(0.0, 0.0, 0.0);

					// Data can be easily updated
					for (int i=0; i<frame->width; i++) {
						for (int j=0; j<frame->height; j++) {
							set_pixel(output_buffer, i, j, ((i % 50) == 0 || (j % 50) == 0) ? black : color, 
									dst_linesize, native_pix_fmt, native_nb_channels);
						}
					}
	#endif


					// OIIO Convert frame (output_buffer) to imagebuf
					OIIO::ImageBuf frame_buffer(OIIO::ImageSpec(frame->width, frame->height, native_nb_channels, get_oiio_base_type_from_format(native_pix_fmt)));
					frame_buffer.set_pixels(OIIO::ROI(), frame_buffer.spec().format, output_buffer, OIIO::AutoStride, dst_linesize);
					
					// Open picto
					int x, y, w, h;
//					OIIO::ImageInput *img = OIIO::ImageInput::open("picto.png");
					auto img = OIIO::ImageInput::open("picto.png");
					const OIIO::ImageSpec& spec = img->spec();
					enum video_format_e img_fmt = get_format_from_oiio_base_type(spec.format.basetype);
					OIIO::TypeDesc::BASETYPE type = get_oiio_base_type_from_format(img_fmt);
					OIIO::ImageBuf *buf = new OIIO::ImageBuf(OIIO::ImageSpec(spec.width, spec.height, spec.nchannels, type)); //, OIIO::InitializePixels::No);
					img->read_image(type, buf->localpixels());

					x = 50;
					y = 900;
					w = spec.width;
					h = spec.height;

	//				printf("%d / %d\n", spec.nchannels, native_nb_channels);

					// Image over
					buf->specmod().x = x;
					buf->specmod().y = y;
					OIIO::ImageBufAlgo::over(frame_buffer, *buf, frame_buffer, OIIO::ROI());

					// Add text
					int pt = (h - (h/10) - (h/10) - (h/10)) / 2;
					float white[] = { 1.0, 1.0, 1.0, 1.0 };
					if (OIIO::ImageBufAlgo::render_text(frame_buffer, x + w + (w/10), y + (h/10) + pt, "VITESSE", pt, "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", white) == false)
						fprintf(stderr, "render text error\n");
					if (OIIO::ImageBufAlgo::render_text(frame_buffer, x + w + (w/10), y + (h/10) + pt + (h/10) + pt, "32 km/h", pt, "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", white) == false)
						fprintf(stderr, "render text error\n");

					// Now convert imagebuf to frame
					frame_buffer.get_pixels(OIIO::ROI(), frame_buffer.spec().format, output_buffer, OIIO::AutoStride, dst_linesize);

					delete buf;

					// Convert & encode frame
					enc_frame = av_frame_alloc();

					enc_frame->width = frame->width;
					enc_frame->height = frame->height;
					enc_frame->format = vs->codecpar->format;
					enc_frame->linesize[0] = frame->linesize[0];
					enc_frame->linesize[1] = frame->linesize[1];
					enc_frame->linesize[2] = frame->linesize[2];
printf("width x height: %d x %d\n", frame->width, frame->height);
printf("format = %d\n", vs->codecpar->format);
					enc_frame->interlaced_frame = 1;
					enc_frame->top_field_first = 1;

					ret = av_frame_get_buffer(enc_frame, 0);

					if (ret < 0) {
						av_log(NULL, AV_LOG_ERROR, "Failed to create AVFrame buffer");
						goto end;
					}

					ret = sws_scale(sws_ctx2,
							(const uint8_t * const *) &output_buffer,
							&dst_linesize,
							0,
							frame->height,
							enc_frame->data,
							enc_frame->linesize);

					printf("linesize = [%d,%d,%d] / dst_linesize = %d / height = %d\n", 
							enc_frame->linesize[0], enc_frame->linesize[1], enc_frame->linesize[2], dst_linesize, enc_frame->height);

					if (ret < 0) {
						av_log(NULL, AV_LOG_ERROR, "Failed to scale frame from native");
						goto end;
					}

					enc_frame->pts = stream->dec_frame->pts;

					stream->enc_frame = enc_frame;

					free(output_buffer);
			}
			else {
				stream->enc_frame = stream->dec_frame;
			}
//#endif

			// Encode
			ret = encode_write_frame(stream_index, 0);
			if (ret < 0)
				goto end;
//#if 0
			if (stream_index == video_stream_index)	{
				av_frame_free(&enc_frame);
			}
//#endif
		}
//		else {
//            // remux this frame without reencoding
//            av_packet_rescale_ts(packet,
//                                 ifmt_ctx->streams[stream_index]->time_base,
//                                 ofmt_ctx->streams[stream_index]->time_base);
// 
//            ret = av_interleaved_write_frame(ofmt_ctx, packet);
//            if (ret < 0)
//                goto end;
//        }
        
		av_packet_unref(packet);
	}

    // flush encoders 
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        // flush encoder
        ret = flush_encoder(i);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Flushing encoder failed\n");
            goto end;
        }
    }

    av_write_trailer(ofmt_ctx);

end:
    av_packet_free(&packet);
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        avcodec_free_context(&stream_ctx[i].dec_ctx);

        if (ofmt_ctx && ofmt_ctx->nb_streams > i && ofmt_ctx->streams[i] && stream_ctx[i].enc_ctx)
            avcodec_free_context(&stream_ctx[i].enc_ctx);

        av_packet_free(&stream_ctx[i].enc_pkt);
        av_frame_free(&stream_ctx[i].dec_frame);
    }
    av_free(stream_ctx);
    avformat_close_input(&ifmt_ctx);
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
 
    if (ret < 0) {
		char s[1024];
		av_strerror(ret, s, sizeof(s));
        av_log(NULL, AV_LOG_ERROR, "Error occurred: %s\n", s);
 	}

    return ret ? 1 : 0;
}

