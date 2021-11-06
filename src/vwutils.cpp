#include "vwutils.h"


VideoWriter::VideoWriter(char* filename, float fps, int fwidth, int fheight, bool iscolor)
{

	int error=avio_open(&avioctx, filename, AVIO_FLAG_WRITE);
	
	if (error<0)
	{
		av_strerror(error, ebuf, sizeof(ebuf));
		printf("Could not open output file: %s", ebuf);
	}
	filepath=filename;
	frame_width=fwidth;
	frame_height=fheight;
	iscolor=iscolor;
	_fps=fps;
	setup_frame();
	setup_encoder();
	add_video_stream();
	get_ready_to_write();
}

void VideoWriter::get_ready_to_write(void)
{

	if (avcodec_open2(avcctx, NULL, NULL)<0)
	{
		std::cerr << "Could not open codec context\n";
	}

	if (avcodec_parameters_from_context(avs->codecpar, avcctx)<0)
	{
		std::cerr << "Could not copy params\n";
	}

	av_init_packet(&avpkt);
	avformat_write_header(avfctx, NULL);
}

void VideoWriter::setup_frame()
{
	avframe=av_frame_alloc();
	if (!avframe)
	{
		std::cerr << "Could not allocate frame. What is this even? smh\n";
	}
	avframe->format=AV_PIX_FMT_YUV420P;
	avframe->width=frame_width;
	avframe->height=frame_height;
	if (av_frame_get_buffer(avframe, 32)<0)
	{
		std::cerr << "Yo frame so fat, the system ran out of memory!\n";
	}

	avbgrframe=av_frame_alloc();
	
	if(!avbgrframe)
	{
		std::cerr << "Could not allocate bgr\n";
	}

	avbgrframe->format=AV_PIX_FMT_BGR24;
	avbgrframe->width=frame_width;
	avbgrframe->height=frame_height;

	if(av_frame_get_buffer(avbgrframe, 32)<0)
	{
		std::cerr << "Could not allocate frame buffer for bgr\n";
	}

	converter=sws_getContext(
		frame_width, 
		frame_height, 
		AV_PIX_FMT_BGR24, 
		frame_width,
		frame_height,
		AV_PIX_FMT_YUV420P,
		SWS_BILINEAR,
		NULL, NULL, NULL);

	if (!converter)
	{
		std::cerr << "Cannot initialize converter\n";
	}
}

void VideoWriter::setup_encoder(void)
{
	//setup encoder and its context and properties

	encoder=avcodec_find_encoder_by_name("h264_videotoolbox");
	
	if (encoder==NULL)
	{
		printf("Could not load qsv encoder\n");
		std::cerr << "oops" << std::endl;
	}
	else
	{
		printf("Successfully found qsv encoder\n");
		/* H264 qsv supports: 0, 23, 160 ==> YUV420P, NV12, YUV440P12BE
		H265 qsv supports: 0, 23, 160, 28, 161 ==> all above and BGRA, AYUV64LE */
	}

	avcctx=avcodec_alloc_context3(encoder);

	if (!avcctx)
	{
		std::cerr << "Could not allocate context for encoder\n";
	}
    
	//allocate context for this encoder and set its properties
	avcctx->bit_rate=5000000;
	avcctx->width = frame_width;
	avcctx->height= frame_height;

}

void VideoWriter::add_video_stream(void)
{
	avfctx = avformat_alloc_context();

	if (!avfctx)
	{
		std::cerr <<"Could not create output format context\n";
	}

	avs = avformat_new_stream(avfctx, NULL);//allocate new stream
	avfctx->pb = avioctx;
	avfctx->oformat = av_guess_format(NULL, filepath, NULL);//new AVOutputFormat;
	printf("oformat name: %s,\n full: %s \n, codecid: %d\n", 
		avfctx->oformat->name, avfctx->oformat->long_name, avfctx->oformat->video_codec);
	avs->time_base=(AVRational){100, (int)(100*_fps)};

	avcctx->time_base=avs->time_base; //yay
	avcctx->gop_size=12; //is this a typical value?
	avcctx->pix_fmt=AV_PIX_FMT_YUV420P;

	avfctx->oformat->video_codec=encoder->id;
}

bool VideoWriter::write(uint8_t *data)
{
	av_frame_make_writable(avbgrframe);
	av_frame_make_writable(avframe);

	for (int j=0; j<frame_width*frame_height*3; j++)
	{
		avbgrframe->data[0][j] = *data;
		data++;
	}

	if(sws_scale(converter, (const uint8_t * const *) avbgrframe->data, 
		avbgrframe->linesize, 0, frame_height, avframe->data,
		avframe->linesize)!=frame_height)
	{
		std::cerr << "Error in scaling\n";
	}

	avframe->pts=pts;

	int ret=avcodec_send_frame(avcctx, avframe);
	
	if (ret<0)
	{
		std::cerr << "Error submitting frame for encoding\n";
	}
	printf("Frame number %d\n", avcctx->frame_number);
	while (ret>=0)
	{
		ret=avcodec_receive_packet(avcctx, &avpkt);

		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
		{
			//std::cerr<< "Unknown error in encoding video frame\n";
			break;
		}
		else if (ret<0)
		{
			std::cerr << "Problem\n";
		}
		av_packet_rescale_ts(&avpkt, avcctx->time_base, avs->time_base);
		avpkt.stream_index= avs->index;
		avpkt.pts=pts++;
		//pts++;
		ret=av_interleaved_write_frame(avfctx, &avpkt);
		if (ret<0)
		{
			std::cerr << "Error writing video frame\n";
		}
	}

	return true;
}

void VideoWriter::release()
{
	avcodec_free_context(&avcctx);
	av_frame_free(&avframe);
	av_frame_free(&avbgrframe);
	sws_freeContext(converter);
	avformat_free_context(avfctx);
	//avresample_free(&ost->avr);
}

VideoWriter::~VideoWriter()
{
	release();
}
