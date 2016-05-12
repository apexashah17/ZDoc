FFMPEG视频h264和音频aac混合编码过程

/*
The MIT License (MIT)

Copyright (c) 2013 winlin

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
// for int64_t print using PRId64 format.
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
// for cpp to use c-style macro UINT64_C in libavformat
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <vector>

extern "C" {
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/avcodec.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavutil/pixdesc.h>
}

bool received_sigterm = false;
void signal_handler(int signo)
{
    printf("get a signal %d(%#x)\n", signo, signo);

    if (signo == SIGINT)
    {
        received_sigterm = true;
        return;
    }

    if (signo == SIGTERM || signo == SIGHUP)
    {
        printf("emergence exit\n");
        exit(1);
    }
}

#define DEFAULT_VIDEO_INDEX 0
#define DEFAULT_AUDIO_INDEX 1
#include <set>
class InterleavedQueue
{
private:
    struct AVPacketCompare
    {
        bool operator() (const AVPacket* a, const AVPacket* b) const
        {
            return a->dts < b->dts;
        }
    };
public:
    InterleavedQueue()
    {
        got_video_ = false;
        start_dts_ = -1;
    }

    virtual ~InterleavedQueue()
    {
        std::multiset<AVPacket*, AVPacketCompare>::iterator it;
        for(it = interleaved_packets_.begin(); it != interleaved_packets_.end(); ++it)
        {
            AVPacket* pkt = *it;

            av_free_packet(pkt);
            av_free(pkt);
        }

        interleaved_packets_.clear();
    }

    void add_packet(AVPacket* pkt)
    {
        if (pkt->stream_index == DEFAULT_VIDEO_INDEX)
        {
            got_video_ = true;
        }

        if (start_dts_ == -1)
        {
            start_dts_ = pkt->dts;
        }

        pkt->dts -= start_dts_;
        pkt->pts -= start_dts_;

        interleaved_packets_.insert(pkt);
    }

    bool should_flush()
    {
        // more than one stream in queue, we can flush the queue.
        // if flush, must flush util this function is false.
        // when flushed, must invoke the reset_criteria
        return !interleaved_packets_.empty() && (got_video_ || interleaved_packets_.size() >= 10000);
    }

    bool empty()
    {
        return interleaved_packets_.empty();
    }

    int size()
    {
        return (int)interleaved_packets_.size();
    }

    void adjust(int diff)
    {
        std::multiset<AVPacket*, AVPacketCompare>::iterator it;
        for(it = interleaved_packets_.begin(); it != interleaved_packets_.end(); ++it)
        {
            AVPacket* pkt = *it;

            bool is_video = pkt->stream_index == DEFAULT_VIDEO_INDEX;
            printf("[%s] adjust exists packet, pts=%"PRId64" to %"PRId64", dts=%"PRId64" to %"PRId64"\n",
                   (is_video? "video": "audio"), pkt->pts, pkt->pts + diff, pkt->dts, pkt->dts + diff);

            pkt->dts += diff;
            pkt->pts += diff;
        }
    }

    AVPacket* pop_packet()
    {
        AVPacket* pkt = NULL;

        if (!interleaved_packets_.empty())
        {
            pkt = *(interleaved_packets_.begin());
            interleaved_packets_.erase(interleaved_packets_.begin());
        }

        // flush finished, reset the criteria
        if (interleaved_packets_.empty())
        {
            reset_criteria();
        }

        // when get video, we must not dequeue anymore
        // for the video is delayed more than audio.
        if (pkt && pkt->stream_index == DEFAULT_VIDEO_INDEX)
        {
            reset_criteria();
        }

        return pkt;
    }

private:
    void reset_criteria()
    {
        got_video_ = false;
    }
private:
    bool got_video_;
    int64_t start_dts_;
    std::multiset<AVPacket*, AVPacketCompare> interleaved_packets_;
};
InterleavedQueue queue;

/**
* open input and output files
* AVFormatContext* ic, AVStream* ist, AVCodecContext* ist->codec, AVCodec* dec
* AVFormatContext* oc, AVStream* ost, AVCodecContext* ost->codec, AVCodec* enc
* @remark ist->codec->codec is NULL.
* @remark ost->codec->codec is NULL.
*/
int demo_video_open_input_output_files(
    /*input*/
    const char* input, const char* iformat_name, const char* output, const char* oformat_name,
    const char* encoder_name,
    /*output*/
    AVFormatContext*& ic, int& stream_index, AVStream*& ist,
    AVCodec*& dec, AVFormatContext*& oc, AVStream*& ost, AVCodec*& enc)
{
    int ret = 0;

    AVInputFormat *file_iformat = av_find_input_format(iformat_name);
    assert(ret >= 0);

    // open ic
    ret = avformat_open_input(&ic, input, file_iformat, NULL);
    assert(ret >= 0);

    ret = avformat_find_stream_info(ic, NULL);
    assert(ret >= 0);

    // find decoder
    stream_index = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    assert(stream_index >= 0);
    ist = ic->streams[stream_index];

    dec = avcodec_find_decoder(ist->codec->codec_id);
    assert(dec);
    av_dump_format(ic, 0, input, 0);

    // open oc
    if (!oc)
    {
        ret = avformat_alloc_output_context2(&oc, NULL, oformat_name, output);
        assert(ret >= 0);
    }
    ost = avformat_new_stream(oc, NULL);
    assert(ost);
    enc = avcodec_find_encoder_by_name(encoder_name);
    assert(enc);

    if (true)
    {
        ost->id = DEFAULT_VIDEO_INDEX;
        // copy codec info to stream.
        ost->codec->codec_id = enc->id;
        avcodec_get_context_defaults3(ost->codec, enc);
        ost->discard = AVDISCARD_NONE;
        // Some formats want stream headers to be separate.
        if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        {
            ost->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
        }
    }

    av_dict_copy(&oc->metadata, ic->metadata, AV_DICT_DONT_OVERWRITE);
    av_dict_set(&oc->metadata, "creation_time", NULL, 0);
    av_dict_copy(&ost->metadata, ist->metadata, AV_DICT_DONT_OVERWRITE);

    return ret;
}

/**
* setup the filter graph, init the ifilter(buffersrc_ctx) and ofilter(buffersink_ctx).
* AVFilterContext* buffersrc_ctx, to where put decoded frame
* AVFilterContext* buffersink_ctx, from where get filtered frame
*/
#if 1
int demo_video_configure_filtergraph(
    /*input*/
    AVStream* ist, AVStream* ost, AVCodec* enc, AVFilterGraph* graph,
    /*output*/
    AVFilterContext*& buffersrc_ctx, AVFilterContext*& buffersink_ctx)
{
    int ret = 0;

    assert(ost);

    // inputs/outputs build by avfilter_graph_parse2
    AVFilterInOut* inputs = NULL;
    AVFilterInOut* outputs = NULL;
    // init filter graph
    if (true)
    {
        // init simple filters
        const char* graph_desc = "null";
        // ost->sws_flags
        graph->scale_sws_opts = av_strdup("flags=0x4");
        av_opt_set(graph, "aresample_swr_opts", "", 0);
        graph->resample_lavr_opts = av_strdup("");
        // build filter graph
        ret = avfilter_graph_parse2(graph, graph_desc, &inputs, &outputs);
        assert(ret >= 0);
        // simple filter must have only one input and output.
        assert(inputs && !inputs->next);
        assert(outputs && !outputs->next);
    }
    // config input filter
    if (true)
    {
        // first_filter is "null"
        AVFilterContext* first_filter = inputs->filter_ctx;
        int pad_idx = inputs->pad_idx;

        // get buffer audio filter
        AVFilter* buffersrc = avfilter_get_by_name("buffer");
        // init buffer audio filter
        char args[512];
        memset(args, 0, sizeof(args));
        // time_base=1/44100:sample_rate=44100:sample_fmt=fltp:channel_layout=0x3
        snprintf(args, sizeof(args),
                 "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d:sws_param=flags=%d:frame_rate=%d/%d",
                 ist->codec->width, ist->codec->height, ist->codec->pix_fmt, ist->time_base.num, ist->time_base.den,
                 ist->codec->sample_aspect_ratio.num, ist->codec->sample_aspect_ratio.den,
                 SWS_BILINEAR + ((ist->codec->flags&CODEC_FLAG_BITEXACT) ? SWS_BITEXACT:0),
                 ist->r_frame_rate.num, ist->r_frame_rate.den);
        printf("[video] filter -> %s %s\n", "buffer", args);
        ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "buffer-filter", args, NULL, graph);
        assert(ret >= 0);

        // TODO: add filter "setpts" if output fps changed.

        // link src "buffer" to dst "null"
        // the data flow: buffer ===> null
        ret = avfilter_link(buffersrc_ctx, 0, first_filter, pad_idx);
        assert(ret >= 0);

        avfilter_inout_free(&inputs);
    }
    // config output filter
    if (true)
    {
        // last_filter is "null"
        AVFilterContext* last_filter = outputs->filter_ctx;
        int pad_idx = outputs->pad_idx;

        // init ffbuffersink audio filter
        // link it later.
        AVFilter* buffersink = avfilter_get_by_name("ffbuffersink");
        printf("[video] filter -> %s\n", "ffbuffersink");
        ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "buffersink-filter", NULL, NULL, graph);
        assert(ret >= 0);

        // TODO: add filter "scale" if output size changed.

        // pix_fmt filter, see: choose_pix_fmts
        if (enc && enc->pix_fmts)
        {
            char args[512];
            memset(args, 0, sizeof(args));
            for (const AVPixelFormat* p = enc->pix_fmts; *p != AV_PIX_FMT_NONE; p++)
            {
                const char *name = av_get_pix_fmt_name(*p);
                int size = strlen(args);
                snprintf(args + size, sizeof(args) - size, "%s:", name);
            }
            args[strlen(args) - 1] = 0;

            AVFilterContext* format_ctx = NULL;
            AVFilter* format = avfilter_get_by_name("format");
            printf("[video] filter -> %s %s\n", "format", args);
            ret = avfilter_graph_create_filter(&format_ctx, format, "format-filter", args, NULL, graph);
            assert(ret >= 0);

            // link to and change the last filter.
            ret = avfilter_link(last_filter, pad_idx, format_ctx, 0);
            assert(ret >= 0);

            last_filter = format_ctx;
            pad_idx     = 0;
        }

        // TODO: add filter "fps" if output fps changed.

        // link the buffersink to the last filer
        // the data flow: aformat ===> buffersink
        // full data flow: null ===> aformat ===> buffersink
        ret = avfilter_link(last_filter, pad_idx, buffersink_ctx, 0);
        assert(ret >= 0);

        avfilter_inout_free(&outputs);
    }
    ret = avfilter_graph_config(graph, NULL);
    assert(ret >= 0);

    // output frame_rate change to:
    // av_buffersink_get_frame_rate(buffersink_ctx)
    // if not specified, use the ist frame_rate.
    // see: ffmpeg.c:2290, after configure_filtergraph.

    return ret;
}
#else
int demo_video_configure_filtergraph(
    /*input*/
    AVStream* ist, AVStream* ost, AVCodec* enc, AVFilterGraph* graph,
    /*output*/
    AVFilterContext*& buffersrc_ctx, AVFilterContext*& buffersink_ctx)
{
    int ret = 0;

    assert(ost);

    if (true)
    {
        // get buffer audio filter
        AVFilter* buffersrc = avfilter_get_by_name("buffer");
        // init buffer audio filter
        char args[512];
        memset(args, 0, sizeof(args));
        // time_base=1/44100:sample_rate=44100:sample_fmt=fltp:channel_layout=0x3
        snprintf(args, sizeof(args),
                 "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d:sws_param=flags=%d:frame_rate=%d/%d",
                 ist->codec->width, ist->codec->height, ist->codec->pix_fmt, ist->time_base.num, ist->time_base.den,
                 ist->codec->sample_aspect_ratio.num, ist->codec->sample_aspect_ratio.den,
                 SWS_BILINEAR + ((ist->codec->flags&CODEC_FLAG_BITEXACT) ? SWS_BITEXACT:0),
                 ist->r_frame_rate.num, ist->r_frame_rate.den);
        printf("[video] filter -> %s %s\n", "buffer", args);
        ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "buffer-filter", args, NULL, graph);
        assert(ret >= 0);
    }

    if (true)
    {
        // init ffabuffersink audio filter
        AVFilter* buffersink = avfilter_get_by_name("ffbuffersink");
        printf("[video] filter -> %s\n", "ffbuffersink");
        ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "buffersink-filter", NULL, NULL, graph);
        assert(ret >= 0);
    }

    if (true)
    {
        AVFilterInOut *outputs = avfilter_inout_alloc();
        outputs->name       = av_strdup("in");
        outputs->filter_ctx = buffersrc_ctx;
        outputs->pad_idx    = 0;
        outputs->next       = NULL;

        AVFilterInOut *inputs  = avfilter_inout_alloc();
        inputs->name       = av_strdup("out");
        inputs->filter_ctx = buffersink_ctx;
        inputs->pad_idx    = 0;
        inputs->next       = NULL;

        // aresample=8000,aconvert=s16:mono
        char filters_descr[512];
        memset(filters_descr, 0, sizeof(filters_descr));
        snprintf(filters_descr, sizeof(filters_descr),
                 "scale=%d:%d", ist->codec->width, ist->codec->height);
        printf("[video] filter -> filters_descr %s\n", filters_descr);

        ret = avfilter_graph_parse(graph, filters_descr, &inputs, &outputs, NULL);
        assert(ret >= 0);

        ret = avfilter_graph_config(graph, NULL);
        assert(ret >= 0);
    }

    return ret;
}
#endif

/**
* setup ost->codec, open enc and dec
* @remark ist->codec->codec equals to dec
* @remark ost->codec->codec equals to enc
*/
int demo_video_setup_and_open_codec(
    AVDictionary* x264_opts,
    AVFilterContext* ofilter, AVStream* ost, AVCodec* enc,
    AVFormatContext* oc, AVStream* ist, AVCodec* dec)
{
    int ret = 0;

    // set encoder
    if (true)
    {
        ost->codec->time_base      = av_inv_q(av_buffersink_get_frame_rate(ofilter));
        ost->codec->width  = ofilter->inputs[0]->w;
        ost->codec->height = ofilter->inputs[0]->h;
        ost->codec->pix_fmt = (AVPixelFormat)ofilter->inputs[0]->format;
        // TODO: overridden by the -aspect cli option
        ost->codec->sample_aspect_ratio = ost->sample_aspect_ratio = ofilter->inputs[0]->sample_aspect_ratio;

        AVDictionary* opts = NULL;
        av_dict_copy(&opts, x264_opts, 0);
        if (!av_dict_get(opts, "threads", NULL, 0))
        {
            av_dict_set(&opts, "threads", "auto", 0);
        }
        // open encoder, set ost->codec->codec to enc
        ret = avcodec_open2(ost->codec, enc, &opts);
        assert(ret >= 0);
        av_dict_free(&opts);
        // set frame size
        if (enc->type == AVMEDIA_TYPE_AUDIO && !(enc->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE))
        {
            av_buffersink_set_frame_size(ofilter, ost->codec->frame_size);
        }
    }
    // open decoder
    if (true)
    {
        AVDictionary* opts = NULL;
        if (!av_dict_get(opts, "threads", NULL, 0))
        {
            av_dict_set(&opts, "threads", "auto", 0);
        }

        // TODO: maybe need to setup the buffer.
        // when codec->type == AVMEDIA_TYPE_VIDEO && ist->dr1
        // see: ffmpeg.c:1969, before open the dec.

        // ffmpeg donot open the dec when find it.
        ret = avcodec_open2(ist->codec, dec, &opts);
        assert(ret >= 0);
        av_dict_free(&opts);
    }
    // write encoder header
    if (avformat_write_header(oc, NULL) != 0)
    {
        exit(-1);
    }

    return ret;
}

/**
* output packet to filter
*/
int demo_video_output_packet(AVFilterContext* ifilter, AVStream* ist, AVPacket* pkt,
                             AVFrame*& decoded_frame)
{
    int ret = 0;

    // alloc frame if NULL
    if (!decoded_frame)
    {
        decoded_frame = avcodec_alloc_frame();
    }

    int got_frame = 0;
    // decode pkt to frame
    ret = avcodec_decode_video2(ist->codec, decoded_frame, &got_frame, pkt);
    assert(ret >= 0);

    // not ready yet.
    if (!got_frame)
    {
        return ret;
    }

    int64_t best_effort_timestamp = av_frame_get_best_effort_timestamp(decoded_frame);
    // ffmpeg also set the ist->next_pts = ist->pts,
    // see: ffmpeg.c:1672
    decoded_frame->pts = best_effort_timestamp;
    printf("[video] decoder ->  frame pts=%"PRId64"\n", decoded_frame->pts);

    // seems that ffmpeg copy the frame to buffer and push to filter directly
    // when: ist->dr1 && decoded_frame->type==FF_BUFFER_TYPE_USER && !changed
    // see: ffmpeg.c:1725

    // output to filter: "buffer"
    ret = av_buffersrc_add_frame(ifilter, decoded_frame, AV_BUFFERSRC_FLAG_PUSH);
    assert(ret >= 0);

    return ret;
}

/**
* output EOF packet to filter to flush
*/
int demo_video_output_eof_packet(AVStream* ist, AVFrame*& decoded_frame, AVFilterContext* ifilter)
{
    int ret = 0;

    // alloc frame if NULL
    if (!decoded_frame)
    {
        decoded_frame = avcodec_alloc_frame();
    }

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    int got_frame = 0;
    ret = avcodec_decode_video2(ist->codec, decoded_frame, &got_frame, &pkt);
    // EOF, assert got nothing and ret is 0.
    // TODO: here we still got frame, different to ffmpeg.
    assert(ret >= 0);

    // flush filter
    av_buffersrc_add_ref(ifilter, NULL, 0);

    return ret;
}

int demo_do_video_out(AVFormatContext* oc, AVStream* ost, AVFrame* filtered_frame, int* pgot_packet);
/**
* read from filter, encode and output
*/
int demo_video_reap_filters(AVFormatContext* oc, AVStream* ost, AVFilterContext* ofilter, AVFrame*& filtered_frame)
{
    int ret = 0;

    if (!filtered_frame)
    {
        filtered_frame = avcodec_alloc_frame();
    }
    avcodec_get_frame_defaults(filtered_frame);

    // pull filtered audio from the filtergraph
    // we ignore the starttime.
    int64_t start_time = 0;
    while (true)
    {
        // get filtered frame.
        AVFilterBufferRef* picref = NULL;
        ret = av_buffersink_get_buffer_ref(ofilter, &picref, AV_BUFFERSINK_FLAG_NO_REQUEST);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            return 0; // no frame filtered.
        }
        assert(ret >= 0);

        // correct the pts
        int64_t filtered_frame_pts = AV_NOPTS_VALUE;
        if (picref->pts != AV_NOPTS_VALUE)
        {
            // rescale the tb, actual the ofilter tb equals to ost tb,
            // so this step canbe ignored and we always set start_time to 0.
            filtered_frame_pts = av_rescale_q(picref->pts, ofilter->inputs[0]->time_base, ost->codec->time_base)
                                 - av_rescale_q(start_time, AV_TIME_BASE_Q, ost->codec->time_base);
        }

        // convert to frame
        avfilter_copy_buf_props(filtered_frame, picref);
        printf("[video] filter -> picref_pts=%"PRId64", frame_pts=%"PRId64", filtered_pts=%"PRId64"\n",
               picref->pts, filtered_frame->pts, filtered_frame_pts);
        filtered_frame->pts = filtered_frame_pts;

        // do_audio_out
        ret = demo_do_video_out(oc, ost, filtered_frame, NULL);
        assert(ret >= 0);

        // never free the picref before the encode, for it will use it.
        avfilter_unref_bufferp(&picref);
    }
}

// the audio/video starttime.
static int64_t av_starttime = -1;

/**
* encode and output
*/
int demo_do_video_out(AVFormatContext* /*oc*/, AVStream* ost, AVFrame* filtered_frame, int* pgot_packet)
{
    int ret = 0;

    if (!filtered_frame)
    {
        return ret;
    }

    AVPacket pkt;

    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    if (filtered_frame->interlaced_frame)
    {
        ost->codec->field_order = AV_FIELD_PROGRESSIVE;
    }
    if (!ost->codec->me_threshold)
    {
        filtered_frame->pict_type = AV_PICTURE_TYPE_NONE;
    }

    int got_packet = 0;
    ret = avcodec_encode_video2(ost->codec, &pkt, filtered_frame, &got_packet);
    assert(ret >= 0);

    if (pgot_packet)
    {
        *pgot_packet = got_packet;
    }
    if (!got_packet)
    {
        return ret;
    }

    // correct the output, enforce start at 0.
#if 1
    // rescale audio ts to AVRational(1, 1000) for flv format.
    AVRational flv_tb = (AVRational)
    {
        1, 1000
    };
    pkt.dts = av_rescale_q(pkt.dts, ost->codec->time_base, flv_tb);
    pkt.pts = av_rescale_q(pkt.pts, ost->codec->time_base, flv_tb);
#endif
#if 1
    if (av_starttime < 0)
    {
        av_starttime = (pkt.dts < pkt.pts)? pkt.dts : pkt.pts;
    }
    if (pkt.dts < av_starttime)
    {
        int diff = av_starttime - pkt.dts;
        printf("[video] adjust starttime from %"PRId64" to %"PRId64", diff=%d, queue-size=%d\n",
               av_starttime, av_starttime - diff, diff, queue.size());
        av_starttime -= diff;
        queue.adjust(diff);
    }
    pkt.dts -= av_starttime;
    pkt.pts -= av_starttime;
#endif

    static int64_t last_dts = 0;
    printf("[video] encoder -> packet start=%"PRId64", pts=%"PRId64", pts_time=%s, dts=%"PRId64", dts_time=%s, diff=%"PRId64", diff_time=%s, size=%d\n",
           av_starttime, pkt.pts, av_ts2timestr(pkt.pts, &ost->time_base), pkt.dts, av_ts2timestr(pkt.dts, &ost->time_base),
           pkt.dts - last_dts, av_ts2timestr(pkt.dts - last_dts, &ost->time_base), pkt.size);
    last_dts = pkt.dts;

    AVPacket *new_pkt = (AVPacket*) av_malloc(sizeof(AVPacket));
    av_copy_packet(new_pkt, &pkt);
    new_pkt->stream_index = DEFAULT_VIDEO_INDEX;
    queue.add_packet(new_pkt);

    av_free_packet(&pkt);

    return ret;
}

int demo_video_transcode_step(
    /*input*/
    AVFilterGraph* graph, AVFormatContext*ic, AVFormatContext* oc,
    AVStream* ist, AVStream* ost, AVFilterContext* ifilter, AVFilterContext* ofilter,
    int stream_index, int rate_emulate,
    /*output*/
    AVFrame*& decoded_frame,
    AVFrame*& filtered_frame,
    bool& need_output)
{
    int ret = 0;

    /* transcode_from_filter */
    // if filter is EOF, flush it.
    ret = avfilter_graph_request_oldest(graph);
    if (ret >= 0)
    {
        ret = demo_video_reap_filters(oc, ost, ofilter, filtered_frame);
        assert(ret >= 0);
        return ret;
    }
    if (ret == AVERROR_EOF)
    {
        need_output = false;
        ret = demo_video_reap_filters(oc, ost, ofilter, filtered_frame);
        assert(ret >= 0);
        return ret;
    }

    // get_input_packet
    AVPacket pkt;
    ret = av_read_frame(ic, &pkt);
    if (ret < 0)
    {
        //assert(ret == AVERROR_EOF);
        ret = demo_video_output_eof_packet(ist, decoded_frame, ifilter);
        assert(ret >= 0);
        return ret;
    }
    if (pkt.stream_index != stream_index)
    {
        av_free_packet(&pkt);
        return ret;
    }
    printf("[video] demuxer -> packet pts=%"PRId64", pts_time=%s, dts=%"PRId64", dts_time=%s\n",
           pkt.pts, av_ts2timestr(pkt.pts, &ist->time_base), pkt.dts, av_ts2timestr(pkt.dts, &ist->time_base));

    if (rate_emulate)
    {
        static int64_t start_dts = pkt.dts;
        static double last_time_s = 0;
        static int64_t last_time_ms = av_gettime();
        double now_s = av_q2d(ist->time_base) * (pkt.dts - start_dts);
        if (last_time_s == 0)
        {
            last_time_s = now_s;
        }
        if (now_s - last_time_s > 0.3)
        {
            int64_t sleep_us = now_s * 1000 * 1000 - (av_gettime() - last_time_ms);
            printf("[video] re -> rate emulate, last_time=%.4f, now=%.3f, diff=%.3f, sleep=%"PRId64"\n",
                   last_time_s, now_s, now_s - last_time_s, sleep_us);
            // max sleep 3s
            if (sleep_us > 0 && sleep_us < (now_s - last_time_s) * 1000 * 1000 * 10)
            {
                av_usleep(sleep_us);
            }
            last_time_s = now_s;
        }
    }

    // output_packet: output packet to filter
    ret = demo_video_output_packet(ifilter, ist, &pkt, decoded_frame);
    assert(ret >= 0);
    av_free_packet(&pkt);

    // reap_filters: read from filter, encode and output
    ret = demo_video_reap_filters(oc, ost, ofilter, filtered_frame);
    assert(ret >= 0);

    return ret;
}

/**
* open input and output files
* AVFormatContext* ic, AVStream* ist, AVCodecContext* ist->codec, AVCodec* dec
* AVFormatContext* oc, AVStream* ost, AVCodecContext* ost->codec, AVCodec* enc
* @remark ist->codec->codec is NULL.
* @remark ost->codec->codec is NULL.
*/
int demo_audio_open_input_output_files(
    /*input*/
    const char* input, const char* iformat_name, const char* output, const char* oformat_name,
    int sample_rate, int channels, const char* encoder_name,
    /*output*/
    AVFormatContext*& ic, int& stream_index, AVStream*& ist,
    AVCodec*& dec, AVFormatContext*& oc, AVStream*& ost, AVCodec*& enc)
{
    int ret = 0;

    AVInputFormat *file_iformat = av_find_input_format(iformat_name);
    assert(ret >= 0);

    // open ic
    ret = avformat_open_input(&ic, input, file_iformat, NULL);
    assert(ret >= 0);

    ret = avformat_find_stream_info(ic, NULL);
    assert(ret >= 0);

    // find decoder
    stream_index = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    assert(stream_index >= 0);
    ist = ic->streams[stream_index];

    dec = avcodec_find_decoder(ist->codec->codec_id);
    assert(dec);
    av_dump_format(ic, 0, input, 0);

    // open oc
    if (!oc)
    {
        ret = avformat_alloc_output_context2(&oc, NULL, oformat_name, output);
        assert(ret >= 0);
    }
    ost = avformat_new_stream(oc, NULL);
    assert(ost);
    enc = avcodec_find_encoder_by_name(encoder_name);
    assert(enc);

    if (true)
    {
        ost->id = DEFAULT_AUDIO_INDEX;
        // copy codec info to stream.
        ost->codec->codec_id = enc->id;
        avcodec_get_context_defaults3(ost->codec, enc);
        ost->discard = AVDISCARD_NONE;
        // Some formats want stream headers to be separate.
        if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        {
            ost->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
        }
        // set encode params
        ost->codec->channels = channels;
        ost->codec->sample_rate = sample_rate;
    }

    av_dict_copy(&oc->metadata, ic->metadata, AV_DICT_DONT_OVERWRITE);
    av_dict_set(&oc->metadata, "creation_time", NULL, 0);
    av_dict_copy(&ost->metadata, ist->metadata, AV_DICT_DONT_OVERWRITE);

    return ret;
}

/**
* setup the filter graph, init the ifilter(buffersrc_ctx) and ofilter(buffersink_ctx).
* AVFilterContext* buffersrc_ctx, to where put decoded frame
* AVFilterContext* buffersink_ctx, from where get filtered frame
*/
#if 0
int demo_audio_configure_filtergraph(
    /*input*/
    AVStream* ist, AVStream* ost, AVCodec* enc, AVFilterGraph* graph,
    /*output*/
    AVFilterContext*& buffersrc_ctx, AVFilterContext*& buffersink_ctx)
{
    int ret = 0;

    // inputs/outputs build by avfilter_graph_parse2
    AVFilterInOut* inputs = NULL;
    AVFilterInOut* outputs = NULL;
    // init filter graph
    if (true)
    {
        // init simple filters
        const char* anull_filters_desc = "anull";
        // ost->sws_flags
        graph->scale_sws_opts = av_strdup("flags=0x4");
        av_opt_set(graph, "aresample_swr_opts", "", 0);
        graph->resample_lavr_opts = av_strdup("");
        // build filter graph
        ret = avfilter_graph_parse2(graph, anull_filters_desc, &inputs, &outputs);
        assert(ret >= 0);
        // simple filter must have only one input and output.
        assert(inputs && !inputs->next);
        assert(outputs && !outputs->next);
    }
    // config input filter
    if (true)
    {
        // first_filter is "anull"
        AVFilterContext* first_filter = inputs->filter_ctx;
        int pad_idx = inputs->pad_idx;

        // get abuffer audio filter
        AVFilter* abuffersrc = avfilter_get_by_name("abuffer");
        // init abuffer audio filter
        char args[512];
        memset(args, 0, sizeof(args));
        // time_base=1/44100:sample_rate=44100:sample_fmt=fltp:channel_layout=0x3
        snprintf(args, sizeof(args),
                 "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%"PRIx64,
                 1, ist->codec->sample_rate, ist->codec->sample_rate,
                 av_get_sample_fmt_name(ist->codec->sample_fmt), ist->codec->channel_layout);
        ret = avfilter_graph_create_filter(&buffersrc_ctx, abuffersrc, "abuffer-filter", args, NULL, graph);
        assert(ret >= 0);
        // link src "abuffer" to dst "anull"
        // the data flow: abuffer ===> anull
        ret = avfilter_link(buffersrc_ctx, 0, first_filter, pad_idx);
        assert(ret >= 0);

        avfilter_inout_free(&inputs);
    }
    // config output filter
    if (true)
    {
        // last_filter is "anull"
        AVFilterContext* last_filter = outputs->filter_ctx;
        int pad_idx = outputs->pad_idx;

        // init ffabuffersink audio filter
        // link it later.
        AVABufferSinkParams* params = av_abuffersink_params_alloc();
        params->all_channel_counts = 1;
        AVFilter* abuffersink = avfilter_get_by_name("ffabuffersink");
        ret = avfilter_graph_create_filter(&buffersink_ctx, abuffersink, "abuffersink-filter", NULL, params, graph);
        assert(ret >= 0);
        av_free(params);

        // init the encoder context channel_layout.
        // if aformat not specified, encoder failed,
        // error message: [pcm_s16le @ 0x25b62e0] Specified sample format fltp is invalid or not supported
        if (ost->codec->channels && !ost->codec->channel_layout)
        {
            ost->codec->channel_layout = av_get_default_channel_layout(ost->codec->channels);

            const char* sample_fmts = av_get_sample_fmt_name(*enc->sample_fmts);
            char args[512];
            memset(args, 0, sizeof(args));
            snprintf(args, sizeof(args),
                     "sample_fmts=%s:sample_rates=%d:channel_layouts=0x%"PRIx64":",
                     sample_fmts, ost->codec->sample_rate, ost->codec->channel_layout);

            AVFilterContext* aformat_ctx = NULL;
            AVFilter* aformat = avfilter_get_by_name("aformat");
            ret = avfilter_graph_create_filter(&aformat_ctx, aformat, "aformat-filter", args, NULL, graph);
            assert(ret >= 0);

            // the data flow: anull ===> aformat
            ret = avfilter_link(last_filter, pad_idx, aformat_ctx, 0);
            assert(ret >= 0);
            // now, "aformat" is the last filter
            last_filter = aformat_ctx;
            pad_idx = 0;
        }

        // link the abuffersink to the last filer
        // the data flow: aformat ===> abuffersink
        // full data flow: anull ===> aformat ===> abuffersink
        ret = avfilter_link(last_filter, pad_idx, buffersink_ctx, 0);
        assert(ret >= 0);

        avfilter_inout_free(&outputs);
    }
    ret = avfilter_graph_config(graph, NULL);
    assert(ret >= 0);

    return ret;
}
#else
int demo_audio_configure_filtergraph(
    /*input*/
    AVStream* ist, AVStream* ost, AVCodec* enc, AVFilterGraph* graph,
    /*output*/
    AVFilterContext*& buffersrc_ctx, AVFilterContext*& buffersink_ctx)
{
    int ret = 0;

    if (true)
    {
        if (!ist->codec->channel_layout)
        {
            ist->codec->channel_layout = av_get_default_channel_layout(ist->codec->channels);
        }

        // get abuffer audio filter
        AVFilter* abuffersrc = avfilter_get_by_name("abuffer");
        // init abuffer audio filter
        char args[512];
        memset(args, 0, sizeof(args));
        // time_base=1/44100:sample_rate=44100:sample_fmt=fltp:channel_layout=0x3
        snprintf(args, sizeof(args),
                 "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%"PRIx64,
                 1, ist->codec->sample_rate, ist->codec->sample_rate,
                 av_get_sample_fmt_name(ist->codec->sample_fmt), ist->codec->channel_layout);
        printf("[audio] filter -> %s %s\n", "abuffer", args);
        ret = avfilter_graph_create_filter(&buffersrc_ctx, abuffersrc, "abuffer-filter", args, NULL, graph);
        assert(ret >= 0);
    }

    if (true)
    {
        // init ffabuffersink audio filter
        // link it later.
        AVABufferSinkParams* params = av_abuffersink_params_alloc();
        const enum AVSampleFormat sample_fmts[] = { AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE };
        params->sample_fmts     = sample_fmts;
        params->all_channel_counts = 1;
        AVFilter* abuffersink = avfilter_get_by_name("ffabuffersink");
        printf("[audio] filter -> %s\n", "ffabuffersink");
        ret = avfilter_graph_create_filter(&buffersink_ctx, abuffersink, "abuffersink-filter", NULL, NULL, graph);
        assert(ret >= 0);
        av_free(params);
    }

    if (true)
    {
        AVFilterInOut *outputs = avfilter_inout_alloc();
        outputs->name       = av_strdup("in");
        outputs->filter_ctx = buffersrc_ctx;
        outputs->pad_idx    = 0;
        outputs->next       = NULL;

        AVFilterInOut *inputs  = avfilter_inout_alloc();
        inputs->name       = av_strdup("out");
        inputs->filter_ctx = buffersink_ctx;
        inputs->pad_idx    = 0;
        inputs->next       = NULL;

        if (!ost->codec->channel_layout)
        {
            ost->codec->channel_layout = av_get_default_channel_layout(ost->codec->channels);
        }

        // aresample=8000,aconvert=s16:mono
        char filters_descr[512];
        memset(filters_descr, 0, sizeof(filters_descr));
        snprintf(filters_descr, sizeof(filters_descr),
                 "aresample=%d,aconvert=%s:%s", ost->codec->sample_rate,
                 av_get_sample_fmt_name(*enc->sample_fmts),
                 (ost->codec->channel_layout==AV_CH_LAYOUT_MONO)? "mono":"stereo");
        printf("[audio] filter -> filters_descr %s\n", filters_descr);

        ret = avfilter_graph_parse(graph, filters_descr, &inputs, &outputs, NULL);
        assert(ret >= 0);

        ret = avfilter_graph_config(graph, NULL);
        assert(ret >= 0);
    }

    return ret;
}
#endif

/**
* setup ost->codec, open enc and dec
* @remark ist->codec->codec equals to dec
* @remark ost->codec->codec equals to enc
*/
int demo_audio_setup_and_open_codec(
    AVFilterContext* ofilter, AVStream* ost, AVCodec* enc,
    AVFormatContext* oc, AVStream* ist, AVCodec* dec)
{
    int ret = 0;

    // set encoder
    if (true)
    {
        ost->codec->sample_fmt     = (AVSampleFormat)ofilter->inputs[0]->format;
        ost->codec->sample_rate    = ofilter->inputs[0]->sample_rate;
        ost->codec->channels       = avfilter_link_get_channels(ofilter->inputs[0]);
        ost->codec->channel_layout = ofilter->inputs[0]->channel_layout;
        ost->codec->time_base      = (AVRational)
        {
            1, ost->codec->sample_rate
        };

        AVDictionary* opts = NULL;
        if (!av_dict_get(opts, "threads", NULL, 0))
        {
            av_dict_set(&opts, "threads", "auto", 0);
        }
        // open encoder, set ost->codec->codec to enc
        ret = avcodec_open2(ost->codec, enc, &opts);
        assert(ret >= 0);
        av_dict_free(&opts);
        // set frame size
        if (enc->type == AVMEDIA_TYPE_AUDIO && !(enc->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE))
        {
            av_buffersink_set_frame_size(ofilter, ost->codec->frame_size);
        }
    }
    // open decoder
    if (true)
    {
        AVDictionary* opts = NULL;
        if (!av_dict_get(opts, "threads", NULL, 0))
        {
            av_dict_set(&opts, "threads", "auto", 0);
        }
        // ffmpeg donot open the dec when find it.
        ret = avcodec_open2(ist->codec, dec, &opts);
        assert(ret >= 0);
        av_dict_free(&opts);
    }
    // write encoder header
    if (avformat_write_header(oc, NULL) != 0)
    {
        exit(-1);
    }

    return ret;
}

int demo_do_audio_out(AVFormatContext* oc, AVStream* ost, AVFrame* filtered_frame, int* pgot_packet);
/**
* read from filter, encode and output
*/
int demo_audio_reap_filters(AVFormatContext* oc, AVStream* ost, AVFilterContext* ofilter, AVFrame*& filtered_frame)
{
    int ret = 0;

    if (!filtered_frame)
    {
        filtered_frame = avcodec_alloc_frame();
    }
    avcodec_get_frame_defaults(filtered_frame);

    // pull filtered audio from the filtergraph
    // we ignore the starttime.
    int64_t start_time = 0;
    while (true)
    {
        // get filtered frame.
        AVFilterBufferRef* picref = NULL;
        ret = av_buffersink_get_buffer_ref(ofilter, &picref, AV_BUFFERSINK_FLAG_NO_REQUEST);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            return 0; // no frame filtered.
        }
        assert(ret >= 0);

        // correct the pts
        int64_t filtered_frame_pts = AV_NOPTS_VALUE;
        if (picref->pts != AV_NOPTS_VALUE)
        {
            // rescale the tb, actual the ofilter tb equals to ost tb,
            // so this step canbe ignored and we always set start_time to 0.
            filtered_frame_pts = av_rescale_q(picref->pts, ofilter->inputs[0]->time_base, ost->codec->time_base)
                                 - av_rescale_q(start_time, AV_TIME_BASE_Q, ost->codec->time_base);
        }

        // convert to frame
        avfilter_copy_buf_props(filtered_frame, picref);
        printf("[audio] filter -> picref_pts=%"PRId64", frame_pts=%"PRId64", filtered_pts=%"PRId64"\n",
               picref->pts, filtered_frame->pts, filtered_frame_pts);
        filtered_frame->pts = filtered_frame_pts;

        // do_audio_out
        ret = demo_do_audio_out(oc, ost, filtered_frame, NULL);
        assert(ret >= 0);

        // never free the picref before the encode, for it will use it.
        avfilter_unref_bufferp(&picref);
    }
}

/**
* output EOF packet to filter to flush
*/
int demo_audio_output_eof_packet(AVStream* ist, AVFrame*& decoded_frame, AVFilterContext* ifilter)
{
    int ret = 0;

    // alloc frame if NULL
    if (!decoded_frame)
    {
        decoded_frame = avcodec_alloc_frame();
    }

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    int got_frame = 0;
    ret = avcodec_decode_audio4(ist->codec, decoded_frame, &got_frame, &pkt);
    // EOF, assert got nothing and ret is 0.
    assert(ret == 0 && got_frame == 0);

    // flush filter
    av_buffersrc_add_ref(ifilter, NULL, 0);

    return ret;
}

/**
* output packet to filter
*/
int demo_audio_output_packet(AVFilterContext* ifilter, AVStream* ist, AVPacket* pkt,
                             AVFrame*& decoded_frame, int64_t& rescale_last_pts)
{
    int ret = 0;

    // alloc frame if NULL
    if (!decoded_frame)
    {
        decoded_frame = avcodec_alloc_frame();
    }

    int got_frame = 0;
    // decode pkt to frame
    // maybe not got_frame, but the ret>0, we need to decode again? ffmpeg did this.
    // see ffmpeg.c:1895, 1898
    ret = avcodec_decode_audio4(ist->codec, decoded_frame, &got_frame, pkt);
    assert(ret >= 0);

    // not ready yet.
    if (!got_frame)
    {
        return ret;
    }

    // set decoded frame ts
    // it's very important, or the filter will got wrong pts.
#if 1
    AVRational decoded_frame_tb;
    if (decoded_frame->pkt_pts != AV_NOPTS_VALUE)
    {
        decoded_frame->pts = decoded_frame->pkt_pts;
        pkt->pts           = AV_NOPTS_VALUE;
        decoded_frame_tb   = ist->time_base;
    }
    if (decoded_frame->pts != AV_NOPTS_VALUE)
    {
        AVRational in_tb = decoded_frame_tb;
        AVRational fs_tb = (AVRational)
        {
            1, ist->codec->sample_rate
        };
        int duration = decoded_frame->nb_samples;
        AVRational out_tb = (AVRational)
        {
            1, ist->codec->sample_rate
        };

        /*
        // init the rescale_last_pts, set to 0 for the first decoded_frame->pts is 0
        if (rescale_last_pts == AV_NOPTS_VALUE) {
            rescale_last_pts = av_rescale_q(decoded_frame->pts, in_tb, fs_tb);
        }
        // the fs_tb equals to out_tb, so decoded_frame->pts equals to rescale_last_pts
        decoded_frame->pts = av_rescale_q(rescale_last_pts, fs_tb, out_tb);;
        rescale_last_pts += duration;
        */
        decoded_frame->pts = av_rescale_delta(in_tb, decoded_frame->pts, fs_tb, duration, &rescale_last_pts, out_tb);
    }
#else
    /**
    * for audio encoding, we simplify the rescale algorithm to following.
    */
    if (rescale_last_pts == AV_NOPTS_VALUE)
    {
        rescale_last_pts = 0;
    }
    decoded_frame->pts = rescale_last_pts;
    rescale_last_pts += decoded_frame->nb_samples; // duration
#endif
    printf("[audio] decoder ->  frame pts=%"PRId64", last=%"PRId64"\n", decoded_frame->pts, rescale_last_pts);

    // output to filter: "abuffer"
    ret = av_buffersrc_add_frame(ifilter, decoded_frame, AV_BUFFERSRC_FLAG_PUSH);
    assert(ret >= 0);

    // reset the pts
    //decoded_frame->pts = AV_NOPTS_VALUE;
    //pkt->dts = pkt->pts = AV_NOPTS_VALUE;

    return ret;
}

/**
* encode and output
*/
int demo_do_audio_out(AVFormatContext* /*oc*/, AVStream* ost, AVFrame* filtered_frame, int* pgot_packet)
{
    int ret = 0;

    if (!filtered_frame)
    {
        return ret;
    }

    AVPacket pkt;

    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    int got_packet = 0;
    ret = avcodec_encode_audio2(ost->codec, &pkt, filtered_frame, &got_packet);
    assert(ret >= 0);

    if (pgot_packet)
    {
        *pgot_packet = got_packet;
    }
    if (!got_packet)
    {
        return ret;
    }

    // correct the output, enforce start at 0.
#if 1
    // rescale audio ts to AVRational(1, 1000) for flv format.
    AVRational flv_tb = (AVRational)
    {
        1, 1000
    };
    pkt.dts = av_rescale_q(pkt.dts, ost->codec->time_base, flv_tb);
    pkt.pts = av_rescale_q(pkt.pts, ost->codec->time_base, flv_tb);
#endif
#if 1
    if (av_starttime < 0)
    {
        av_starttime = (pkt.dts < pkt.pts)? pkt.dts : pkt.pts;
    }
    if (pkt.dts < av_starttime)
    {
        int diff = av_starttime - pkt.dts;
        printf("[audio] adjust starttime from %"PRId64" to %"PRId64", diff=%d, queue-size=%d\n",
               av_starttime, av_starttime - diff, diff, queue.size());
        av_starttime -= diff;
        queue.adjust(diff);
    }
    pkt.dts -= av_starttime;
    pkt.pts -= av_starttime;
#endif

    static int64_t last_dts = 0;
    printf("[audio] encoder -> packet start=%"PRId64", pts=%"PRId64", pts_time=%s, dts=%"PRId64", dts_time=%s, diff=%"PRId64", diff_time=%s, size=%d\n",
           av_starttime, pkt.pts, av_ts2timestr(pkt.pts, &ost->time_base), pkt.dts, av_ts2timestr(pkt.dts, &ost->time_base),
           pkt.dts - last_dts, av_ts2timestr(pkt.dts - last_dts, &ost->time_base), pkt.size);
    last_dts = pkt.dts;

    AVPacket *new_pkt = (AVPacket*) av_malloc(sizeof(AVPacket));
    av_copy_packet(new_pkt, &pkt);
    new_pkt->stream_index = DEFAULT_AUDIO_INDEX;
    queue.add_packet(new_pkt);

    av_free_packet(&pkt);

    return ret;
}

std::vector<AVPacket*> audio_queue;
pthread_mutex_t audio_mutex;
bool audio_thread_exit = false;
int audio_thread_ret = 0;
/**
* if rate-emulate is enabled, we should never start the ingest audio thread,
* for we can read all audios in this thread and break the rate-emulate ruler
* which need to control the read of audio/video.
*/
void* ingest_audio(void* args)
{
    AVFormatContext* ic = (AVFormatContext*)args;
    assert(ic);

    while (!audio_thread_exit)
    {
        AVPacket* pkt = (AVPacket*) av_malloc(sizeof(AVPacket));

        int ret = av_read_frame(ic, pkt);
        if (ret >= 0)
        {
            pthread_mutex_lock(&audio_mutex);
            audio_queue.push_back(pkt);
            pthread_mutex_unlock(&audio_mutex);
            continue;
        }

        if (ret == AVERROR_EOF || ret == -11)
        {
            printf("[audio] ingest thread EOF. ret=%d\n", audio_thread_ret);
            break;
        }

        audio_thread_ret = ret;
        printf("[audio] ignore ingest thread error. ret=%d\n", audio_thread_ret);

        av_free_packet(pkt);
        av_free(pkt);
    }

    return NULL;
}

int demo_audio_transcode_step(
    /*input*/
    AVFilterGraph* graph, AVFormatContext* ic, AVFormatContext* oc,
    AVStream* ist, AVStream* ost, AVFilterContext* ifilter, AVFilterContext* ofilter,
    int stream_index, int rate_emulate,
    /*output*/
    AVFrame*& decoded_frame,
    AVFrame*& filtered_frame,
    int64_t& rescale_last_pts,
    bool& need_output)
{
    int ret = 0;

    /* transcode_from_filter */
    // if filter is EOF, flush it.
    ret = avfilter_graph_request_oldest(graph);
    if (ret >= 0)
    {
        ret = demo_audio_reap_filters(oc, ost, ofilter, filtered_frame);
        assert(ret >= 0);
        return ret;
    }
    if (ret == AVERROR_EOF)
    {
        need_output = false;
        ret = demo_audio_reap_filters(oc, ost, ofilter, filtered_frame);
        assert(ret >= 0);
        return ret;
    }

    std::vector<AVPacket*> audios;

    if (!rate_emulate)
    {
        // get all packets
        if (audio_queue.empty())
        {
            return 0;
        }
        pthread_mutex_lock(&audio_mutex);
        audios.swap(audio_queue);
        pthread_mutex_unlock(&audio_mutex);
    }
    else
    {
        // donot use thread, directly read.
        AVPacket* pkt = (AVPacket*) av_malloc(sizeof(AVPacket));
        ret = av_read_frame(ic, pkt);
        if (ret >= 0)
        {
            audios.push_back(pkt);
        }
        else
        {
            audio_thread_ret = ret;
            av_free_packet(pkt);
            av_free(pkt);
        }
    }

    // get_input_packet
    for (std::vector<AVPacket*>::iterator it = audios.begin(); it != audios.end(); ++it)
    {
        AVPacket* pkt = *it;
        assert(pkt != NULL);
        if (pkt->stream_index != stream_index)
        {
            av_free_packet(pkt);
            av_free(pkt);
            continue;
        }
        printf("[audio] demuxer -> packet pts=%"PRId64", pts_time=%s, dts=%"PRId64", dts_time=%s\n",
               pkt->pts, av_ts2timestr(pkt->pts, &ist->time_base), pkt->dts, av_ts2timestr(pkt->dts, &ist->time_base));

        // output_packet: output packet to filter
        ret = demo_audio_output_packet(ifilter, ist, pkt, decoded_frame, rescale_last_pts);
        assert(ret >= 0);
        av_free_packet(pkt);
        av_free(pkt);

        // reap_filters: read from filter, encode and output
        ret = demo_audio_reap_filters(oc, ost, ofilter, filtered_frame);
        assert(ret >= 0);
    }
    ret = audio_thread_ret;
    if (ret < 0)
    {
        //assert(ret == AVERROR_EOF);
        ret = demo_audio_output_eof_packet(ist, decoded_frame, ifilter);
        assert(ret >= 0);
        return ret;
    }

    return ret;
}

int flush_queue(AVFormatContext* oc, AVStream* video_ost, AVStream* audio_ost, bool force_flush_all)
{
    int ret = 0;

    // output by orderded queue.
    // force to flush all: to send all out.
    // should_flush: queue is ready to flush.
    int count = 0;
    while ((force_flush_all && !queue.empty()) || queue.should_flush())
    {
        AVPacket* pkt = queue.pop_packet();

        bool is_video = (pkt->stream_index == DEFAULT_VIDEO_INDEX);
        AVRational time_base = is_video? video_ost->time_base : audio_ost->time_base;

        static int64_t last_dts = 0;
        printf("[%s] muxer -> packet pts=%"PRId64", pts_time=%s, dts=%"PRId64", dts_time=%s, diff=%"PRId64", diff_time=%s, size=%d\n",
               is_video? "video":"audio", pkt->pts, av_ts2timestr(pkt->pts, &time_base), pkt->dts, av_ts2timestr(pkt->dts, &time_base),
               pkt->dts - last_dts, av_ts2timestr(pkt->dts - last_dts, &time_base), pkt->size);
        last_dts = pkt->dts;

        ret = av_write_frame(oc, pkt);
        assert(ret >= 0);

        av_free_packet(pkt);
        av_free(pkt);

        count++;
    }
    printf("[media] muxer -> queue flushed %d packets==========================================\n", count);

    return ret;
}

int main(int argc, char** argv)
{
    int ret = 0;

    if (argc <= 11)
    {
        printf("Usage: %s <rate_emulate> <audio_input> <audio_iformat_name> <video_input> <video_iformat_name> "
               "<output> <oformat_name> <audio_encoder> <sample_rate> <channels> <video_encoder> [x264_options]\n"
               "   rate_emulate: like the -re of ffmpeg. eg. 1\n"
               "   audio_input: the input file. eg. /home/winlin/test_22m.flv\n"
               "   audio_iformat_name: the input file format name. eg. flv\n"
               "   video_input: the input file. eg. /home/winlin/test_22m.flv\n"
               "   video_iformat_name: the input file format name. eg. flv\n"
               "   output: the output file. eg. /home/winlin/output/winlin.mp4\n"
               "   oformat_name: the output file format name. eg. mp4\n"
               "   audio_encoder: the audio encoder name. eg. libfdk_aac pcm_s16le\n"
               "   sample_rate: the sample_rate. eg. 8000 22050 32000 44100\n"
               "   channels: the channels. eg. 1 2\n"
               "   video_encoder: the video encoder name. eg. libx264\n"
               "   x264_options: the video encoder options. eg. coder 0 b_strategy 0 bf 0 refs 1 b 300k\n"
               "For example:\n"
               "   %s 0 test_22m.flv flv test_22m.flv flv /home/winlin/output/winlin.mp4 mp4 libfdk_aac 8000 1 libx264 coder 0 b_strategy 0 bf 0 refs 1 b 300k\n"
               "   %s 0 test_22m.flv flv test_22m.flv flv rtmp://dev:1935/live/livestream flv libfdk_aac 8000 1 libx264 coder 0 b_strategy 0 bf 0 refs 1 b 300k\n"
               "   %s 0 hw:0,0 alsa /dev/video0 v4l2 rtmp://dev:1935/live/livestream flv libfdk_aac 8000 1 libx264 coder 0 b_strategy 0 bf 0 refs 1 b 300k\n",
               argv[0], argv[0], argv[0], argv[0]);
        exit(-1);
    }
    int index = 1;
    int rate_emulate = ::atoi(argv[index++]);
    const char* audio_input = argv[index++];
    const char* audio_iformat_name = argv[index++];
    const char* video_input = argv[index++];
    const char* video_iformat_name = argv[index++];
    const char* output = argv[index++];
    const char* oformat_name = argv[index++];

    const char* audio_encoder = argv[index++];
    int sample_rate = ::atoi(argv[index++]);
    int channels = ::atoi(argv[index++]);

    const char* video_encoder = argv[index++];
    AVDictionary* x264_opts = NULL;
    for (int i = index; i < argc; i += 2)
    {
        av_dict_set(&x264_opts, argv[i], argv[i + 1], 0);
    }

    // handle signal.
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGHUP, signal_handler);

    // register all.
    avcodec_register_all();
    avdevice_register_all();
    av_register_all();
    avfilter_register_all();
    avformat_network_init();

    /* ffmpeg_parse_options */

    // open input and output files
    AVFormatContext* oc = NULL;
    // video specified
    AVFormatContext* video_ic = NULL;
    int video_stream_index = 0;
    AVStream* video_ist = NULL;
    AVCodec* video_dec = NULL;
    AVStream* video_ost = NULL;
    AVCodec* video_enc = NULL;
    // audio specified
    AVFormatContext* audio_ic = NULL;
    int audio_stream_index = 0;
    AVStream* audio_ist = NULL;
    AVCodec* audio_dec = NULL;
    AVStream* audio_ost = NULL;
    AVCodec* audio_enc = NULL;
    ret = demo_video_open_input_output_files(
              /*input*/video_input, video_iformat_name, output, oformat_name, video_encoder,
              /*output*/video_ic, video_stream_index, video_ist, video_dec, oc, video_ost, video_enc);
    assert(ret >= 0);
    ret = demo_audio_open_input_output_files(
              /*input*/audio_input, audio_iformat_name, output, oformat_name, sample_rate, channels, audio_encoder,
              /*output*/audio_ic, audio_stream_index, audio_ist, audio_dec, oc, audio_ost, audio_enc);
    assert(ret >= 0);
    // open oc
    ret = avio_open2(&oc->pb, output, AVIO_FLAG_WRITE, &oc->interrupt_callback, NULL);
    assert(ret >= 0);

    /* transcode_init */

    AVFilterGraph* graph = avfilter_graph_alloc();
    assert(graph);
    // configure_filtergraph: setup the filter graph, init the ifilter(buffersrc_ctx) and ofilter(buffersink_ctx).
    AVFilterContext* video_buffersrc_ctx = NULL;
    AVFilterContext* video_buffersink_ctx = NULL;
    ret = demo_video_configure_filtergraph(/*input*/video_ist, video_ost, video_enc, graph,
            /*output*/video_buffersrc_ctx, video_buffersink_ctx);
    assert(ret >= 0);
    // configure_filtergraph: setup the filter graph, init the ifilter(buffersrc_ctx) and ofilter(buffersink_ctx).
    AVFilterContext* audio_buffersrc_ctx = NULL;
    AVFilterContext* audio_buffersink_ctx = NULL;
    ret = demo_audio_configure_filtergraph(/*input*/audio_ist, audio_ost, audio_enc, graph,
            /*output*/audio_buffersrc_ctx, audio_buffersink_ctx);
    assert(ret >= 0);

    // setup encoder, open the encoder then decoder
    AVFilterContext* video_ofilter = video_buffersink_ctx; // the output filter is the buffersink
    ret = demo_video_setup_and_open_codec(x264_opts, video_ofilter, video_ost, video_enc, oc, video_ist, video_dec);
    assert(ret >= 0);
    // setup encoder, open the encoder then decoder
    AVFilterContext* audio_ofilter = audio_buffersink_ctx; // the output filter is the buffersink
    ret = demo_audio_setup_and_open_codec(audio_ofilter, audio_ost, audio_enc, oc, audio_ist, audio_dec);
    assert(ret >= 0);
    av_dump_format(oc, 0, output, 1);

    // create thread to ingest audio.
    audio_thread_exit = false;
    pthread_t audio_tid;
    if (!rate_emulate)
    {
        ret = pthread_mutex_init(&audio_mutex, NULL);
        assert(ret >= 0);
        ret = pthread_create(&audio_tid, 0, ingest_audio, audio_ic);
        assert(ret >= 0);
    }

    // the decoded_frame and filtered_frame is shared.
    AVFrame* video_decoded_frame = NULL;
    AVFrame* video_filtered_frame = NULL;
    bool need_output = true;
    AVFrame* audio_decoded_frame = NULL;
    AVFrame* audio_filtered_frame = NULL;
    int64_t rescale_last_pts = AV_NOPTS_VALUE;
    while (!received_sigterm)
    {
        if (!need_output)
        {
            printf("stream EOF.\n");
            break;
        }

        /* transcode_step */
        if (true)
        {
            AVFilterContext* ifilter = audio_buffersrc_ctx;
            ret = demo_audio_transcode_step(
                      /*input*/graph, audio_ic, oc, audio_ist, audio_ost, ifilter, audio_ofilter, audio_stream_index, rate_emulate,
                      /*output*/audio_decoded_frame, audio_filtered_frame, rescale_last_pts, need_output);
            assert(ret >= 0);
        }

        /* transcode_step */
        if (true)
        {
            AVFilterContext* ifilter = video_buffersrc_ctx;
            ret = demo_video_transcode_step(
                      /*input*/graph, video_ic, oc, video_ist, video_ost, ifilter, video_ofilter, video_stream_index, rate_emulate,
                      /*output*/video_decoded_frame, video_filtered_frame, need_output);
            assert(ret >= 0);
        }

        // output by orderded queue.
        flush_queue(oc, video_ost, audio_ost, false);
    }

    /* flush_encoders */
    if (video_ost->codec->codec_type == AVMEDIA_TYPE_VIDEO && video_ost->codec->codec->id != AV_CODEC_ID_RAWVIDEO)
    {
        int stop_encoding = false;
        while (!stop_encoding)
        {
            int got_packet = 0;
            ret = demo_do_video_out(oc, video_ost, NULL, &got_packet);
            assert(ret >= 0);

            if (!got_packet)
            {
                stop_encoding = true;
            }
        }
    }
    /* flush_encoders */
    if (audio_ost->codec->codec_type == AVMEDIA_TYPE_AUDIO && audio_ost->codec->frame_size > 1)
    {
        int stop_encoding = false;
        while (!stop_encoding)
        {
            int got_packet = 0;
            ret = demo_do_audio_out(oc, audio_ost, NULL, &got_packet);
            assert(ret >= 0);

            if (!got_packet)
            {
                stop_encoding = true;
            }
        }
    }
    // output by orderded queue.
    flush_queue(oc, video_ost, audio_ost, true);
    // write trailer
    av_write_trailer(oc);

    // stop thread
    audio_thread_exit = true;
    if (!rate_emulate)
    {
        pthread_join(audio_tid, NULL);
    }

    // cleanup.
    if (audio_ost->codec)
    {
        avcodec_close(audio_ost->codec);
    }
    if (audio_ist->codec)
    {
        avcodec_close(audio_ist->codec);
    }
    avformat_close_input(&audio_ic);

    // cleanup.
    av_dict_free(&x264_opts);
    if (video_ost->codec)
    {
        avcodec_close(video_ost->codec);
    }
    if (video_ist->codec)
    {
        avcodec_close(video_ist->codec);
    }
    avformat_close_input(&video_ic);

    if (oc)
    {
        avformat_free_context(oc);
    }

    return 0;
}
