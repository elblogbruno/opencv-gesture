#ifndef GESREC_H
#define GESREC_H

#include "cv.h"
#include "highgui.h"

extern "C" __declspec(dllexport) CvHistogram* gesSampleSkin(const IplImage* img, CvRect window);
extern "C" __declspec(dllexport) int gesDetectHand(IplImage* src, IplImage* dst);

#endif