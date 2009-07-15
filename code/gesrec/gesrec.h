#ifndef GESREC_H
#define GESREC_H

#include "cv.h"
#include "highgui.h"

extern "C" __declspec(dllexport) CvHistogram* gesSampleSkinHistogram(IplImage* src, CvRect window);
extern "C" __declspec(dllexport) void gesDetectHandHistogram(IplImage* src, IplImage* dst, CvHistogram* histTemp, CvRect window);
extern "C" __declspec(dllexport) void gesSampleSkinRange(IplImage* src, CvScalar* s);
extern "C" __declspec(dllexport) void gesDetectHandRange(IplImage* src, IplImage* dst, CvSeq* comp, CvScalar* s = NULL, int flag = 0);
void gesGrayWorld(IplImage* src, IplImage* dst);
static int gesRectCompFunc(const void* _a, const void* _b, void* userdata);
void gesMultiFloodFill(IplImage* src, CvSeq* comp);

extern "C" __declspec(dllexport) void gesFindContours(IplImage* src, IplImage* dst, CvMemStorage* storage, CvSeq* contour);
static int gesContourCompFunc(const void* _a, const void* _b, void* userdata);

extern "C" __declspec(dllexport) void gesTracking(IplImage* src, IplImage* dst, CvSeq* seq, CvSeq* seq_out, CvScalar* s, int flag = 0);

#endif