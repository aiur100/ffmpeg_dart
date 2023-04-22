#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/timestamp.h>

const char *get_ffmpeg_version()
{
    return av_version_info();
}

#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>

int concatenate_videos(const char *input1, const char *input2, const char *output)
{
    AVFormatContext *ifmt_ctx1 = NULL, *ifmt_ctx2 = NULL;
    AVFormatContext *ofmt_ctx = NULL;
    AVPacket pkt;
    int ret, i;

    // Open input files
    if ((ret = avformat_open_input(&ifmt_ctx1, input1, 0, 0)) < 0)
    {
        fprintf(stderr, "Could not open input file '%s'", input1);
        return ret;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx1, 0)) < 0)
    {
        fprintf(stderr, "Failed to retrieve input stream information");
        return ret;
    }

    if ((ret = avformat_open_input(&ifmt_ctx2, input2, 0, 0)) < 0)
    {
        fprintf(stderr, "Could not open input file '%s'", input2);
        return ret;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx2, 0)) < 0)
    {
        fprintf(stderr, "Failed to retrieve input stream information");
        return ret;
    }

    // Allocate the output context
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, output);
    if (!ofmt_ctx)
    {
        fprintf(stderr, "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    // Copy streams from both input contexts to the output context
    for (int i = 0; i < 2; i++)
    {
        AVFormatContext *ifmt_ctx = (i == 0) ? ifmt_ctx1 : ifmt_ctx2;
        for (int j = 0; j < ifmt_ctx->nb_streams; j++)
        {
            AVStream *in_stream = ifmt_ctx->streams[j];
            AVStream *out_stream = avformat_new_stream(ofmt_ctx, avcodec_find_encoder(in_stream->codecpar->codec_id));
            if (!out_stream)
            {
                fprintf(stderr, "Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                goto end;
            }

            ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
            if (ret < 0)
            {
                fprintf(stderr, "Failed to copy parameters from input to output stream codec context\n");
                goto end;
            }
            out_stream->codecpar->codec_tag = 0;
            out_stream->time_base = in_stream->time_base;
        }
    }

    // Open output file
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&ofmt_ctx->pb, output, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            fprintf(stderr, "Could not open output file '%s'", output);
            goto end;
        }
    }

    // Write header
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Error occurred when opening output file\n");
        goto end;
    }

    // Write packets from both input files
    for (int i = 0; i < 2; i++)
    {
        AVFormatContext *ifmt_ctx = (i == 0) ? ifmt_ctx1 : ifmt_ctx2;

        while (1)
        {
            ret = av_read_frame(ifmt_ctx, &pkt);
            if (ret < 0)
            {
                break;
            }

            AVStream *in_stream = ifmt_ctx->streams[pkt.stream_index];
            AVStream *out_stream = ofmt_ctx->streams[pkt.stream_index + i * ifmt_ctx1->nb_streams];

            // Rescale packet timestamp
            pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
            pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
            pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
            pkt.pos = -1;

            // Ensure DTS values are in the correct order
            if (i == 1 && out_stream->nb_frames > 0)
            {
                AVStream *prev_out_stream = ofmt_ctx->streams[pkt.stream_index];
                int64_t last_dts = prev_out_stream->cur_dts;
                if (pkt.dts <= last_dts)
                {
                    pkt.dts = last_dts + 1;
                }
                if (pkt.pts <= last_dts)
                {
                    pkt.pts = last_dts + 1;
                }
            }

            ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
            if (ret < 0)
            {
                fprintf(stderr, "Error writing packet\n");
                goto end;
            }

            // Free the packet
            av_packet_unref(&pkt);
        }
    }

    // Write trailer
    ret = av_write_trailer(ofmt_ctx);
    if (ret < 0)
    {
        fprintf(stderr, "Error writing trailer\n");
        goto end;
    }

end:
    // Close input files
    if (ifmt_ctx1)
    {
        avformat_close_input(&ifmt_ctx1);
    }
    if (ifmt_ctx2)
    {
        avformat_close_input(&ifmt_ctx2);
    }

    // Close output file
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        avio_closep(&ofmt_ctx->pb);
    }

    // Free the output context
    if (ofmt_ctx)
    {
        avformat_free_context(ofmt_ctx);
    }

    return ret;
}
