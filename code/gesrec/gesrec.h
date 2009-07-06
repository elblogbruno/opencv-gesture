#ifndef GESREC_H
#define GESREC_H

#include "highgui.h"

extern "C" __declspec(dllexport) int gesDetectHand(IplImage * img);

#endif