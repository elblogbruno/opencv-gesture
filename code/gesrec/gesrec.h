#ifndef GESREC_H
#define GESREC_H

#include "cv.h"
#include "highgui.h"

extern "C" __declspec(dllexport) CvHistogram* gesSampleSkinHistogram(IplImage* src, CvRect window);
extern "C" __declspec(dllexport) void gesDetectHandHistogram(IplImage* src, IplImage* dst, CvHistogram* histTemp, CvRect window);
extern "C" __declspec(dllexport) void gesSampleSkinRange(IplImage* src, CvScalar* s);
extern "C" __declspec(dllexport) void gesDetectHandRange(IplImage* src, IplImage* dst, CvMemStorage* storage, CvSeq* comp, CvScalar* s = NULL, int flag = 0);
void gesGrayWorld(IplImage* src, IplImage* dst);
static int gesRectCompFunc(const void* _a, const void* _b, void* userdata);
void gesMultiFloodFill(IplImage* src, CvMemStorage* storage, CvSeq* comp);

extern "C" __declspec(dllexport) void gesFindContours(IplImage* src, IplImage* dst);

extern "C" __declspec(dllexport) CvSeq* gesTracking(IplImage* src, CvSeq* seq, CvScalar* s);

#endif