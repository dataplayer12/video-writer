#include "vwutils.h"

void fillbgrframe(uint8_t* data)
{
	for (int i=0;i<1920*1080*3;i++)
	{
		if (i<1920*1080)
		{
			*data=250;		
		}
		else
		{
			*data=0;
		}
		data++;
	}
}

int main(void)
{
	VideoWriter writer("vwtest.mp4", 29.97, 1920, 1080, true);
	uint8_t data[1920*1080*3];
	fillbgrframe(&data[0]);
	//printf("%d %d %d %d\n", data[0], data[1], data[2], data[3]);
	for (int i=0; i<100; i++)
		writer.write(&data[0]);
	printf("Finished writing\n");
	return 0;
}