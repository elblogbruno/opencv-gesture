#ifndef GESREC_H
#define GESREC_H

#include "cv.h"
#include "highgui.h"

extern "C" __declspec(dllexport) CvHistogram* gesSampleSkin(const IplImage* img, CvRect window);
extern "C" __declspec(dllexport) int gesDetectHandHistogram(IplImage* src, IplImage* dst, CvHistogram* histTemp, CvRect window);
extern "C" __declspec(dllexport) void gesDetectHandRange(IplImage* src, IplImage* dst);
void gesGrayWorld(IplImage* src, IplImage* dst);

#endif