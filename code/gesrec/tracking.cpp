#define NUMS 4

#include "gesrec.h"

//int getPointNum(IplImage* src, 

CvSeq* gesTracking(IplImage* src, CvSeq* seq, CvScalar* s) {
	
	int i = 0;
	double range1, range2, range3, range4;
	CvScalar tempS;
	CvConnectedComp* aComp;
	int seq_num = seq->total;
	CvMemStorage* storage;//¶¯Ì¬ÄÚ´æ
	CvSeq* curr_seq;

	range1 = s->val[1] - 10;
	range2 = s->val[1] + 10;
	range3 = s->val[2] - 10;
	range4 = s->val[2] + 10;

	storage = cvCreateMemStorage(0);
	curr_seq = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvConnectedComp), storage);

	CvRect rect;
	int tempNum = 0;

	while( i < seq_num ) {
		aComp = (CvConnectedComp* )cvGetSeqElem(seq, i);
		rect = aComp->rect;
		int tempx = 0;

		while(rect.x > 0) {
			tempx = rect.x - 1;
			tempNum = 0;
			for(int k = rect.y; k < min(src->height, rect.y + rect.height); k++) {
				tempS = cvGet2D(src, k, tempx);
				if(tempS.val[1] >= range1 && tempS.val[1] <= range2 && 
					tempS.val[2] >= range3 && tempS.val[2] <= range4) {
					tempNum++;
					if(tempNum >= NUMS) {
						rect.x -= 1;
						break;
					}
				} else {
					tempNum = 0;
				}
			}
			if(tempNum < NUMS) {
				break;
			}
		}

		i++;
	}

	return curr_seq;
}