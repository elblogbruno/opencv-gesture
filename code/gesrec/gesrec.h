#ifndef GESREC_H
#define GESREC_H

#include "cv.h"
#include "highgui.h"

extern "C" __declspec(dllexport) CvHistogram* gesSampleSkinHistogram(IplImage* src, CvRect window);
extern "C" __declspec(dllexport) void gesDetectHandHistogram(IplImage* src, IplImage* dst, CvHistogram* histTemp, CvRect window);
extern "C" __declspec(dllexport) void gesSampleSkinRange(IplImage* src, CvScalar* s);
extern "C" __declspec(dllexport) void gesDetectHandRange(IplImage* src, IplImage* dst, CvScalar* s = NULL, int flag = 0);
void gesGrayWorld(IplImage* src, IplImage* dst);
static int gesRectCompFunc(const void* _a, const void* _b, void* userdata);
CvSeq* gesMultiFloodFill(IplImage* src);

extern "C" __declspec(dllexport) void gesFindContours(IplImage* src, IplImage* dst);

#endif