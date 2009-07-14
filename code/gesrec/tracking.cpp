#define NUMS 4

#include "gesrec.h"
#include <stdio.h>

void gesTracking(IplImage* src, IplImage* dst, CvSeq* seq, CvSeq* seq_out, CvScalar* s, int flag) {
	
	int i = 0;
	double range1, range2, range3, range4;
	CvScalar tempS;
	CvConnectedComp* aComp;
	int seq_num = seq->total;

	//设置肤色的范围
	if(flag)//根据样品设置肤色范围
	{
		range1 = s->val[1] - 10;
		range2 = s->val[1] + 10;
		range3 = s->val[2] - 10;
		range4 = s->val[2] + 10;
	}
	else//自动设置肤色范围
	{
		range1 = 143;//133
		range2 = 163;//173
		range3 = 77;//77
		range4 = 127;//127
	}

	CvRect rect;
	int tempNum = 0;
	int sign = 0;

	while( i < min(seq_num, 4) ) {
		sign = 0;
		aComp = (CvConnectedComp* )cvGetSeqElem(seq, i);
		rect = aComp->rect;
		printf("%d, %d, %d, %d\n", aComp->rect.x, aComp->rect.y, aComp->rect.width, aComp->rect.height);
		int tempx = 0;
		int tempy = 0;

		// x 最左列
		printf("1\n");
		tempx = rect.x - 1;
		tempNum = 0;
		for(int k = rect.y; k < min(src->height, rect.y + rect.height); k++) {
			if(tempx < 0) {
				break;
			}
			tempS = cvGet2D(src, k, tempx);
			if(tempS.val[1] >= range1 && tempS.val[1] <= range2 && 
				tempS.val[2] >= range3 && tempS.val[2] <= range4) {
				tempNum++;
				if(tempNum >= NUMS) {
					rect.x -= 1;
					rect.width += 1;
					sign = 1;
					break;
				}
			} else {
				tempNum = 0;
			}
		}
		if(tempNum < NUMS) {
			sign = -1;
		}
		printf("1.1\n");
		while(rect.x > 0 && sign == 1) {
			tempx = rect.x - 1;
			tempNum = 0;
			for(int k = rect.y; k < min(src->height, rect.y + rect.height); k++) {
				if(tempx < 0) {
					break;
				}
				tempS = cvGet2D(src, k, tempx);
				if(tempS.val[1] >= range1 && tempS.val[1] <= range2 && 
					tempS.val[2] >= range3 && tempS.val[2] <= range4) {
					tempNum++;
					if(tempNum >= NUMS) {
						rect.x -= 1;
						rect.width += 1;
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
		printf("1.2\n");
		while( ( (rect.x+rect.width) < src->width-1 ) && sign == -1) {
			tempx = rect.x + 1;
			tempNum = 0;
			for(int k = rect.y; k < min(src->height, rect.y + rect.height); k++) {
				if( (tempx + rect.width) >= src->width-1) {
					break;
				}
				tempS = cvGet2D(src, k, tempx);
				if(tempS.val[1] >= range1 && tempS.val[1] <= range2 && 
					tempS.val[2] >= range3 && tempS.val[2] <= range4) {
					tempNum++;
					if(tempNum >= NUMS) {
						break;
					}
				} else {
					tempNum = 0;
				}
			}
			if(tempNum < NUMS) {
				rect.x += 1;
				rect.width -= 1;
				if(rect.width <= 0) {
					break;
				}
			} else {
				break;
			}
		}

		// x 最右列
		printf("2\n");
		tempx = min( rect.x + rect.width + 1, src->width-1 );
		tempNum = 0;
		for(int k = rect.y; k < min(src->height, rect.y + rect.height); k++) {
			if( tempx >= src->width-1) {
				break;
			}
			tempS = cvGet2D(src, k, tempx);
			if(tempS.val[1] >= range1 && tempS.val[1] <= range2 && 
				tempS.val[2] >= range3 && tempS.val[2] <= range4) {
				tempNum++;
				if(tempNum >= NUMS) {
					rect.width += 1;
					sign = 1;
					break;
				}
			} else {
				tempNum = 0;
			}
		}
		if(tempNum < NUMS) {
			sign = -1;
		}
		printf("2.1\n");
		while( (rect.x + rect.width) < (src->width-1) && sign == 1) {
			tempx = min( rect.x + rect.width + 1, src->width-1 );
			tempNum = 0;
			for(int k = rect.y; k < min(src->height, rect.y + rect.height); k++) {
				if( tempx >= src->width-1) {
					break;
				}
				tempS = cvGet2D(src, k, tempx);
				if(tempS.val[1] >= range1 && tempS.val[1] <= range2 && 
					tempS.val[2] >= range3 && tempS.val[2] <= range4) {
					tempNum++;
					if(tempNum >= NUMS) {
						rect.width += 1;
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
		printf("2.2\n");
		while( rect.width > 0 && sign == -1) {
			tempx = max( rect.x + rect.width - 1, 0 );
			tempNum = 0;
			for(int k = rect.y; k < min(src->height, rect.y + rect.height); k++) {
				if(tempx <= rect.x) {
					break;
				}
				tempS = cvGet2D(src, k, tempx);
				if(tempS.val[1] >= range1 && tempS.val[1] <= range2 && 
					tempS.val[2] >= range3 && tempS.val[2] <= range4) {
					tempNum++;
					if(tempNum >= NUMS) {
						break;
					}
				} else {
					tempNum = 0;
				}
			}
			if(tempNum < NUMS) {
				rect.width -= 1;
			} else {
				break;
			}
		}


		// y 最左列
		printf("3\n");
		tempy = rect.y - 1;
		tempNum = 0;
		for(int k = rect.x; k < min(src->width, rect.x + rect.width); k++) {
			if(tempy < 0) {
				break;
			}
			tempS = cvGet2D(src, tempy, k);
			if(tempS.val[1] >= range1 && tempS.val[1] <= range2 && 
				tempS.val[2] >= range3 && tempS.val[2] <= range4) {
				tempNum++;
				if(tempNum >= NUMS) {
					rect.y -= 1;
					rect.height += 1;
					sign = 1;
					break;
				}
			} else {
				tempNum = 0;
			}
		}
		if(tempNum < NUMS) {
			sign = -1;
		}
		printf("3.1\n");
		while(rect.y > 0 && sign == 1) {
			tempy = rect.y - 1;
			tempNum = 0;
			for(int k = rect.x; k < min(src->width, rect.x + rect.width); k++) {
				if(tempy < 0) {
					break;
				}
				tempS = cvGet2D(src, tempy, k);
				if(tempS.val[1] >= range1 && tempS.val[1] <= range2 && 
					tempS.val[2] >= range3 && tempS.val[2] <= range4) {
					tempNum++;
					if(tempNum >= NUMS) {
						rect.y -= 1;
						rect.height += 1;
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
		printf("3.2\n");
		while( ( (rect.y+rect.height) < src->height-1 ) && sign == -1) {
			tempy = rect.y + 1;
			tempNum = 0;
			for(int k = rect.x; k < min(src->width, rect.x + rect.width); k++) {
				if( (tempy + rect.height) >= src->height-1) {
					break;
				}
				tempS = cvGet2D(src, tempy, k);
				if(tempS.val[1] >= range1 && tempS.val[1] <= range2 && 
					tempS.val[2] >= range3 && tempS.val[2] <= range4) {
					tempNum++;
					if(tempNum >= NUMS) {
						break;
					}
				} else {
					tempNum = 0;
				}
			}
			if(tempNum < NUMS) {
				rect.y += 1;
				rect.height -= 1;
				if(rect.height <= 0) {
					break;
				}
			} else {
				break;
			}
		}

		// y 最右列
		printf("4\n");
		tempy = min( rect.y + rect.height + 1, src->height-1 );
		tempNum = 0;
		for(int k = rect.x; k < min(src->width, rect.x + rect.width); k++) {
			if( tempy >= src->height-1) {
				break;
			}
			tempS = cvGet2D(src, tempy, k);
			if(tempS.val[1] >= range1 && tempS.val[1] <= range2 && 
				tempS.val[2] >= range3 && tempS.val[2] <= range4) {
				tempNum++;
				if(tempNum >= NUMS) {
					rect.height += 1;
					sign = 1;
					break;
				}
			} else {
				tempNum = 0;
			}
		}
		if(tempNum < NUMS) {
			sign = -1;
		}
		printf("4.1\n");
		while( (rect.y + rect.height) < (src->height-1) && sign == 1) {
			tempy = min( rect.y + rect.height + 1, src->height-1 );
			tempNum = 0;
			for(int k = rect.x; k < min(src->width, rect.x + rect.width); k++) {
				if( tempy >= src->height-1) {
					break;
				}
				tempS = cvGet2D(src, tempy, k);
				if(tempS.val[1] >= range1 && tempS.val[1] <= range2 && 
					tempS.val[2] >= range3 && tempS.val[2] <= range4) {
					tempNum++;
					if(tempNum >= NUMS) {
						rect.height += 1;
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
		printf("4.2\n");
		while( rect.height > 0 && sign == -1) {
			tempy = max( rect.y + rect.height - 1, 0 );
			tempNum = 0;
			for(int k = rect.x; k < min(src->width, rect.x + rect.width); k++) {
				if(tempy <= rect.y) {
					break;
				}
				tempS = cvGet2D(src, tempy, k);
				if(tempS.val[1] >= range1 && tempS.val[1] <= range2 && 
					tempS.val[2] >= range3 && tempS.val[2] <= range4) {
					tempNum++;
					if(tempNum >= NUMS) {
						break;
					}
				} else {
					tempNum = 0;
				}
			}
			if(tempNum < NUMS) {
				rect.height -= 1;
			} else {
				break;
			}
		}

		i++;
		
		aComp->rect = rect;
		cvSeqPush(seq_out, aComp);
	}

	//用矩形绘制连通部件
	i = 0;
	int sizeofSeq_out = seq_out->total;
	
	while(i < min(sizeofSeq_out, 4))
	{
		aComp = (CvConnectedComp* )cvGetSeqElem(seq_out, i);
		cvRectangle(dst, cvPoint(aComp->rect.x, aComp->rect.y),
					cvPoint(aComp->rect.x + aComp->rect.width, aComp->rect.y + aComp->rect.height), 
					cvScalar(255, 0, 0), 1);
		i++;
	}
}