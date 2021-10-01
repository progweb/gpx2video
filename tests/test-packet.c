/*
 * Copyright (c) 2010 Nicolas George
 * Copyright (c) 2011 Stefano Sabatini
 * Copyright (c) 2014 Andrey Utkin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
/**
 * @file
 * API example for demuxing, decoding, filtering, encoding and muxing
 * @example transcoding.c
 */
 
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
//#include <libavfilter/buffersink.h>
//#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

 
static int video_stream_index = -1;
static int audio_stream_index = -1;

static AVFormatContext *ifmt_ctx;
static AVFormatContext *ofmt_ctx;
 
typedef struct StreamContext {
    AVCodecContext *dec_ctx;
    AVCodecContext *enc_ctx;
 
    AVFrame *dec_frame;
    AVPacket *enc_pkt;
} StreamContext;
static StreamContext *stream_ctx;
 
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

	stream_ctx = av_mallocz_array(ifmt_ctx->nb_streams, sizeof(*stream_ctx));
	if (!stream_ctx)
		return AVERROR(ENOMEM);

//	// For each stream
//	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
//		// Get reference to correct AVStream
//		AVStream *stream = ifmt_ctx->streams[i];
//
//		// Find decoder
//		const AVCodec *dec = avcodec_find_decoder(stream->codecpar->codec_id);
//		AVCodecContext *codec_ctx;
//		if (!dec) {
//			av_log(NULL, AV_LOG_ERROR, "Failed to find decoder for stream #%u\n", i);
//			//            return AVERROR_DECODER_NOT_FOUND;
//			stream->discard = AVDISCARD_ALL;           
//		}
//
//		// Allocate context for the decoder
//		codec_ctx = avcodec_alloc_context3(dec);
//		if (!codec_ctx) {
//			av_log(NULL, AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u\n", i);
//			return AVERROR(ENOMEM);
//		}
//
//		// Copy parameters from the AVStream to the AVCodecContext
//		ret = avcodec_parameters_to_context(codec_ctx, stream->codecpar);
//		if (ret < 0) {
//			av_log(NULL, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context "
//					"for stream #%u\n", i);
//			return ret;
//		}
//
//		// Reencode video & audio and remux subtitles etc.
//		if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
//				|| codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
//			if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
//				video_stream_index = i;
//				codec_ctx->framerate = av_guess_frame_rate(ifmt_ctx, stream, NULL);
//			}
//			else
//				audio_stream_index = i;
//
//			// Open decoder
//			ret = avcodec_open2(codec_ctx, dec, NULL);
//			if (ret < 0) {
//				av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
//				return ret;
//			}
//		}
//
//		stream_ctx[i].dec_ctx = codec_ctx;
//
//		stream_ctx[i].dec_frame = av_frame_alloc();
//
//		if (!stream_ctx[i].dec_frame)
//			return AVERROR(ENOMEM);
//
//		stream_ctx[i].enc_pkt = av_packet_alloc();
//		if (!stream_ctx[i].enc_pkt)
//			return AVERROR(ENOMEM);
//	}

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
		if (i > 1) {
			continue;
		}

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
//
			continue;
		}

        out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
            return AVERROR_UNKNOWN;
        }

//        if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
//                || dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
//            // in this example, we choose transcoding to same codec
//  			
//			// Find encoder with this name
//            encoder = avcodec_find_encoder(dec_ctx->codec_id);
//            if (!encoder) {
//                av_log(NULL, AV_LOG_FATAL, "Necessary encoder not found\n");
//                return AVERROR_INVALIDDATA;
//            }
//
//// FIXME
////			  if (encoder->type != type) {
////				Error(QStringLiteral("Retrieved unexpected codec type %1 for codec %2").arg(QString::number(encoder->type), codec));
////				return false;
////			  }
//
//            enc_ctx = avcodec_alloc_context3(encoder);
//            if (!enc_ctx) {
//                av_log(NULL, AV_LOG_FATAL, "Failed to allocate the encoder context\n");
//                return AVERROR(ENOMEM);
//            }
// 
//            /* In this example, we transcode to same properties (picture size,
//             * sample rate etc.). These properties can be changed for output
//             * streams easily using filters */
//            if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
//                enc_ctx->height = dec_ctx->height;
//                enc_ctx->width = dec_ctx->width;
//                enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
//                /* take first format from list of supported formats */
//                if (encoder->pix_fmts)
//                    enc_ctx->pix_fmt = encoder->pix_fmts[0];
//                else
//                    enc_ctx->pix_fmt = dec_ctx->pix_fmt;
//                /* video time_base can be set to whatever is handy and supported by encoder */
//                enc_ctx->time_base = av_inv_q(dec_ctx->framerate);
//            } else {
//                enc_ctx->sample_rate = dec_ctx->sample_rate;
//                enc_ctx->channel_layout = dec_ctx->channel_layout;
//                enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);
//                /* take first format from list of supported formats */
//                enc_ctx->sample_fmt = encoder->sample_fmts[0];
//                enc_ctx->time_base = (AVRational){1, enc_ctx->sample_rate};
//            }
// 
//            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
//                enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
// 
//  			// Try to open encoder (third parameter can be used to pass settings to encoder)
//            ret = avcodec_open2(enc_ctx, encoder, NULL);
//            if (ret < 0) {
//                av_log(NULL, AV_LOG_ERROR, "Cannot open video encoder for stream #%u\n", i);
//                return ret;
//            }
//
//  			// Copy context settings to codecpar object
//            ret = avcodec_parameters_from_context(out_stream->codecpar, enc_ctx);
//            if (ret < 0) {
//                av_log(NULL, AV_LOG_ERROR, "Failed to copy encoder parameters to output stream #%u\n", i);
//                return ret;
//            }
// 
//
//			// FIXME
//            out_stream->time_base = enc_ctx->time_base;
//            stream_ctx[i].enc_ctx = enc_ctx;
//        } else if (dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN) {
//            av_log(NULL, AV_LOG_FATAL, "Elementary stream #%d is of unknown type, cannot proceed\n", i);
//            return AVERROR_INVALIDDATA;
//        } else {
			// FIXME
            // if this stream must be remuxed
            ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Copying parameters for stream #%u failed\n", i);
                return ret;
            }
//            out_stream->time_base = in_stream->time_base;
			out_stream->codecpar->codec_tag = 0;
//        }
 
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
 
//static int encode_write_frame(unsigned int stream_index, int flush)
//{
//    StreamContext *stream = &stream_ctx[stream_index];
//    AVFrame *dec_frame = flush ? NULL : stream->dec_frame;
//    AVPacket *enc_pkt = stream->enc_pkt;
//    int ret;
// 
////    av_log(NULL, AV_LOG_INFO, "Encoding frame\n");
//
//	// encode dec frame
//    av_packet_unref(enc_pkt);
// 
//    ret = avcodec_send_frame(stream->enc_ctx, dec_frame);
// 
//    if (ret < 0)
//        return ret;
// 
//    while (ret >= 0) {
//        ret = avcodec_receive_packet(stream->enc_ctx, enc_pkt);
// 
//        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
//            return 0;
// 
//        // prepare packet for muxing
//        enc_pkt->stream_index = stream_index;
//// 		enc_pkt->duration = ofmt_ctx->streams[stream_index]->time_base.den / ofmt_ctx->streams[stream_index]->time_base.num / ifmt_ctx->streams[stream_index]->avg_frame_rate.num * ifmt_ctx->streams[stream_index]->avg_frame_rate.den
//
////        av_packet_rescale_ts(enc_pkt,
////                             stream->enc_ctx->time_base,
////                             ofmt_ctx->streams[stream_index]->time_base);
//        av_packet_rescale_ts(enc_pkt,
//                             ifmt_ctx->streams[stream_index]->time_base,
//                             ofmt_ctx->streams[stream_index]->time_base);
// 
//        av_log(NULL, AV_LOG_DEBUG, "Muxing frame\n");
//
//		// mux encoded frame
//        ret = av_interleaved_write_frame(ofmt_ctx, enc_pkt);
//    }
// 
//    return ret;
//}
 
//static int flush_encoder(unsigned int stream_index)
//{
//	if (!(stream_ctx[stream_index].enc_ctx))
//		return 0;
//    if (!(stream_ctx[stream_index].enc_ctx->codec->capabilities &
//                AV_CODEC_CAP_DELAY))
//        return 0;
// 
//    av_log(NULL, AV_LOG_INFO, "Flushing stream #%u encoder\n", stream_index);
//    return encode_write_frame(stream_index, 1);
//}
 
int main(int argc, char **argv)
{
    int ret;
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
	printf(".............................\n");
    if ((ret = open_output_file(argv[2])) < 0)
        goto end;

	AVStream *in_stream = ifmt_ctx->streams[0];
	double duration = in_stream->duration * av_q2d(in_stream->time_base);

	av_log(NULL, AV_LOG_INFO, "Duration : %f\n", duration);

	av_log(NULL, AV_LOG_INFO, "Process...\n");

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

//        StreamContext *stream = &stream_ctx[stream_index];
//
//      av_log(NULL, AV_LOG_DEBUG, "Going to reencode&filter the frame\n");
//
//        av_packet_rescale_ts(packet,
//                             ifmt_ctx->streams[stream_index]->time_base,
//                             stream->dec_ctx->time_base);
//
//        ret = avcodec_send_packet(stream->dec_ctx, packet);
//        if (ret < 0) {
//            av_log(NULL, AV_LOG_ERROR, "Decoding failed\n");
//            break;
//        }
//
//		while (ret >= 0) {
//			ret = avcodec_receive_frame(stream->dec_ctx, stream->dec_frame);
//
//			if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
//				break;
//			else if (ret < 0)
//				goto end;
//
////			stream->dec_frame->pts = stream->dec_frame->best_effort_timestamp;
//
//
//			// Encode
//			ret = encode_write_frame(stream_index, 0);
//			if (ret < 0)
//				goto end;
//        }
//	   	else {

//            // remux this frame without reencoding
//            av_packet_rescale_ts(packet,
//                                 ifmt_ctx->streams[stream_index]->time_base,
//                                 ofmt_ctx->streams[stream_index]->time_base);
 
            ret = av_interleaved_write_frame(ofmt_ctx, packet);
            if (ret < 0)
                goto end;
//		}

        av_packet_unref(packet);
	}

//    // flush encoders 
//    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
//        // flush encoder
//        ret = flush_encoder(i);
//        if (ret < 0) {
//            av_log(NULL, AV_LOG_ERROR, "Flushing encoder failed\n");
//            goto end;
//        }
//    }

    av_write_trailer(ofmt_ctx);

end:
    av_packet_free(&packet);
//    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
//        avcodec_free_context(&stream_ctx[i].dec_ctx);
//
//        if (ofmt_ctx && ofmt_ctx->nb_streams > i && ofmt_ctx->streams[i] && stream_ctx[i].enc_ctx)
//            avcodec_free_context(&stream_ctx[i].enc_ctx);
//
//        av_packet_free(&stream_ctx[i].enc_pkt);
//        av_frame_free(&stream_ctx[i].dec_frame);
//    }
    av_free(stream_ctx);
    avformat_close_input(&ifmt_ctx);
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
 
    if (ret < 0)
        av_log(NULL, AV_LOG_ERROR, "Error occurred: %s\n", av_err2str(ret));
 
    return ret ? 1 : 0;



//    av_log(NULL, AV_LOG_ERROR, "Init filters\n");
//    if ((ret = init_filters()) < 0)
//        goto end;
//    if (!(packet = av_packet_alloc()))
//        goto end;
// 
//    /* read all packets */
//    while (1) {
//        if ((ret = av_read_frame(ifmt_ctx, packet)) < 0)
//            break;
//        stream_index = packet->stream_index;
//        av_log(NULL, AV_LOG_DEBUG, "Demuxer gave frame of stream_index %u\n",
//                stream_index);
//
//		if (stream_index > 1) {
//        	av_packet_unref(packet);
//			continue;
//		}
//
//        if (filter_ctx[stream_index].filter_graph) {
//            StreamContext *stream = &stream_ctx[stream_index];
// 
//            av_log(NULL, AV_LOG_DEBUG, "Going to reencode&filter the frame\n");
// 
//            av_packet_rescale_ts(packet,
//                                 ifmt_ctx->streams[stream_index]->time_base,
//                                 stream->dec_ctx->time_base);
//            ret = avcodec_send_packet(stream->dec_ctx, packet);
//            if (ret < 0) {
//                av_log(NULL, AV_LOG_ERROR, "Decoding failed\n");
//                break;
//            }
// 
//            while (ret >= 0) {
//                ret = avcodec_receive_frame(stream->dec_ctx, stream->dec_frame);
//                if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
//                    break;
//                else if (ret < 0)
//                    goto end;
// 
//                stream->dec_frame->pts = stream->dec_frame->best_effort_timestamp;
//                ret = filter_encode_write_frame(stream->dec_frame, stream_index);
//                if (ret < 0)
//                    goto end;
//            }
//        } else {
//            // remux this frame without reencoding
//            av_packet_rescale_ts(packet,
//                                 ifmt_ctx->streams[stream_index]->time_base,
//                                 ofmt_ctx->streams[stream_index]->time_base);
// 
//            ret = av_interleaved_write_frame(ofmt_ctx, packet);
//            if (ret < 0)
//                goto end;
//        }
//
//        av_packet_unref(packet);
//    }
//
//    // flush filters and encoders 
//    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
//        /* flush filter */
//        if (!filter_ctx[i].filter_graph)
//            continue;
//        ret = filter_encode_write_frame(NULL, i);
//        if (ret < 0) {
//            av_log(NULL, AV_LOG_ERROR, "Flushing filter failed\n");
//            goto end;
//        }
// 
//        /* flush encoder */
//        ret = flush_encoder(i);
//        if (ret < 0) {
//            av_log(NULL, AV_LOG_ERROR, "Flushing encoder failed\n");
//            goto end;
//        }
//    }
// 
//    av_write_trailer(ofmt_ctx);
//end:
//    av_packet_free(&packet);
//    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
//        avcodec_free_context(&stream_ctx[i].dec_ctx);
//        if (ofmt_ctx && ofmt_ctx->nb_streams > i && ofmt_ctx->streams[i] && stream_ctx[i].enc_ctx)
//            avcodec_free_context(&stream_ctx[i].enc_ctx);
//        if (filter_ctx && filter_ctx[i].filter_graph) {
//            avfilter_graph_free(&filter_ctx[i].filter_graph);
//            av_packet_free(&filter_ctx[i].enc_pkt);
//            av_frame_free(&filter_ctx[i].filtered_frame);
//        }
// 
//        av_frame_free(&stream_ctx[i].dec_frame);
//    }
//    av_free(filter_ctx);
//    av_free(stream_ctx);
//    avformat_close_input(&ifmt_ctx);
//    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
//        avio_closep(&ofmt_ctx->pb);
//    avformat_free_context(ofmt_ctx);
// 
//    if (ret < 0)
//        av_log(NULL, AV_LOG_ERROR, "Error occurred: %s\n", av_err2str(ret));
// 
//    return ret ? 1 : 0;
}

