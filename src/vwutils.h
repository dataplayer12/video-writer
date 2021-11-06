#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <inttypes.h>
}

class VideoWriter
{
public:
	VideoWriter(char* filename, float fps, int fwidth, int fheight, bool iscolor=true);
	~VideoWriter();
	
	bool write(uint8_t* data); //assumed data is given in opencv bgr format
	//there is no need to release the object. It is done automatically

private:
	char* filepath;
	float _fps;
	int frame_width;
	int frame_height;
	bool iscolor;

	void release();
	void setup_frame();
	void setup_encoder();
	void add_video_stream();
	void get_ready_to_write();
	
	uint64_t pts=1;
	uint8_t* gray_rgb_data;
	AVIOContext* avioctx;
	AVCodecContext* avcctx;
	AVFormatContext* avfctx;
	AVStream* avs;
	AVPacket avpkt={0};
	AVFrame* avframe;
	AVFrame* avbgrframe;
	AVCodec* encoder=NULL;
	struct SwsContext *converter;
	char ebuf[255];

};