# video-writer
Super fast video writer for opencv using ffmpeg and libav
(C++ and python)

## The Problem

Opencv's `cv::VideoWriter` is a pain to use. 
- It hides the critical choice of encoders behind an obscure fourcc code and it isn't always transparent which codecs are available for use on the system. This is because opencv depends on a number of [backends](https://docs.opencv.org/3.4.15/d0/da7/videoio_overview.html) like ffmpeg and VFW. If you are someone who codes a lot on multiple systems (linux/mac/win/x86/arm) and embedded boards, this can quickly become frustrating.
- Even setting aside the issue of codecs, `cv::VideoWriter` doesn't let you set important parameters like bitrate and pixel format.

## The easy solution

Unsurprisingly, many other people have noticed the issue and the [most upvoted solution](https://stackoverflow.com/questions/38686359/opencv-videowriter-control-bitrate) on stackoverflow recommends (at least for python) opening a subprocess with ffmpeg and passing frames as JPEGs. Now there are better formats to pass frames into ffmpeg than JPEG and you would be better off really adapting this pipeline for your own usecase, but the bigger problem is that due to the limitations of python multiprocessing, the shells created this way are not freed as long as the parent python program is running (effin' GIL). This lead of all kinds of ugliness. For example, if you are making many videos from one python script, videos which have finished writing will not be viewable in vlc/ffplay/etc until all the videos have finished processing and the parent python script has exited.

## Why this project?
In spite of its limitations, the easy solution works and 99% of users don't need anything else, but if video processing is an essential part of your workflow, you might find it worthwhile to invest in a more satisfying solution. This is where this project comes in. We use LibAV, the backend behind ffmpeg to build a simple video writer object which can be used in C++ an python.

## Status
This is a **work in progress**. Please do not use it in anything critical and feel free to contribute by sending PRs.

## Build
Currently verified on macOS.

```Shell
#Install dependencies
brew install ffmpeg pkg-config cmake

#Clone repo
git clone https://github.com/dataplayer12/video-writer.git
cd video-writer
mkdir build
cd build

#Configure and build project
cmake ../
make
```

`make` will generate a minimum C++ sample and a shared library which can be used in a python script with ctypes module.

## How to use and To-Dos

The goal is to create a `VideoWriter` class which can be instantiated like
```Cpp
//C++
VideoWriter writer("filename.mp4", fps, width, height, encoder_name, bitrate);
writer.write(cvFrame); //cvFrame is a cv::Mat object
```

```Python
#python
import video_writer as vw
import numpy as np

writer= vw.VideoWriter(filename, fps, width, height, encoder_name, bitrate)
x=np.ones((height, width, 3), dtype=np.uint8)
writer.write(x)
```
Currently the `write` method accepts pointers and is not ready for use in python.

## Credits
In making this project, I have learnt a lot from the excellent work of [Bartholomew Joyce](https://github.com/bartjoyce), his [videos](https://www.youtube.com/watch?v=MEMzo59CPr8) and [git repo](https://github.com/bartjoyce/video-app). It helped me st up the environment and get a feel for the funcitons of libav. The difference between his repo and this is that I want to encode rather than decode.