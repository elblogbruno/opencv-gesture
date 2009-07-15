#define NUMS 4

#include "gesrec.h"
#include <stdio.h>

//src: source image data of the function, input
//dst: result image data of the function, output
//seq: sequence of areas to track of last frame, input
//seq_out: sequence of areas to track of current frame, output
//s: sample skin color range of current capture, input
//flag: sign to decide whether to use default sample skin color range
void gesTracking(IplImage* src, IplImage* dst, CvSeq* seq, CvSeq* seq_out, CvScalar* s, int flag) {
	
	int i = 0;
	IplImage* srcYCrCb;
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

	cvSmooth(src, src, CV_GAUSSIAN);
	srcYCrCb = cvCreateImage(cvGetSize(src), IPL_DEPTH_8U, 3);
	//转化至YCrCb颜色空间
	cvCvtColor(src, srcYCrCb, CV_BGR2YCrCb);

	CvRect rect;
	int tempNum = 0;
	int sign = 0;
	cvClearSeq(seq_out);

	//遍历跟踪最大的4个区域
	while( i < min(seq_num, 4) ) {
		sign = 0;
		aComp = (CvConnectedComp* )cvGetSeqElem(seq, i);
		rect = aComp->rect;
		
		int tempx = 0;
		int tempy = 0;

		// 判断区域的最左边是向外膨胀还是向内收缩
		tempx = rect.x - 1;
		tempNum = 0;
		for(int k = rect.y; k < min(srcYCrCb->height, rect.y + rect.height); k++) {
			if(tempx < 0) {
				break;
			}
			tempS = cvGet2D(srcYCrCb, k, tempx);
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
		
		// 区域的最左边向外膨胀
		while(rect.x > 0 && sign == 1) {
			tempx = rect.x - 1;
			tempNum = 0;
			for(int k = rect.y; k < min(srcYCrCb->height, rect.y + rect.height); k++) {
				
				tempS = cvGet2D(srcYCrCb, k, tempx);
				if(tempS.val[1] >= range1 && tempS.val[1] <= range2 && 
					tempS.val[2] >= range3 && tempS.val[2] <= range4) {
					tempNum++;
					// 是否存在连续的NUMS个符合条件的像素点
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
		
		// 区域的最左边向内收缩
		while( ( rect.x < srcYCrCb->width-1 ) && rect.width > 0 && sign == -1) {
			tempx = rect.x + 1;
			tempNum = 0;
			for(int k = rect.y; k < min(srcYCrCb->height, rect.y + rect.height); k++) {
				
				tempS = cvGet2D(srcYCrCb, k, tempx);
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
			} else {
				break;
			}
		}

		// 判断区域的最右边是向外膨胀还是向内收缩
		tempx = min( rect.x + rect.width + 1, srcYCrCb->width-1 );
		tempNum = 0;
		for(int k = rect.y; k < min(srcYCrCb->height, rect.y + rect.height); k++) {
			if( tempx >= srcYCrCb->width-1) {
				break;
			}
			tempS = cvGet2D(srcYCrCb, k, tempx);
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
		
		// 区域的最右边向外膨胀
		while( (rect.x + rect.width) < (srcYCrCb->width-1) && sign == 1) {
			tempx = min( rect.x + rect.width + 1, srcYCrCb->width-1 );
			tempNum = 0;
			for(int k = rect.y; k < min(srcYCrCb->height, rect.y + rect.height); k++) {
				if( tempx >= srcYCrCb->width-1) {
					break;
				}
				tempS = cvGet2D(srcYCrCb, k, tempx);
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
		
		// 区域的最右边向内收缩
		while( rect.width > 0 && sign == -1) {
			tempx = max( rect.x + rect.width - 1, 0 );
			tempNum = 0;
			for(int k = rect.y; k < min(srcYCrCb->height, rect.y + rect.height); k++) {
				if(tempx <= rect.x) {
					break;
				}
				tempS = cvGet2D(srcYCrCb, k, tempx);
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

		// 判断区域的最上方是向外膨胀还是向内收缩
		tempy = rect.y - 1;
		tempNum = 0;
		for(int k = rect.x; k < min(srcYCrCb->width, rect.x + rect.width); k++) {
			if(tempy < 0) {
				break;
			}
			tempS = cvGet2D(srcYCrCb, tempy, k);
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
		
		// 区域的最上方向外膨胀
		while(rect.y > 0 && sign == 1) {
			tempy = rect.y - 1;
			tempNum = 0;
			for(int k = rect.x; k < min(srcYCrCb->width, rect.x + rect.width); k++) {
				if(tempy < 0) {
					break;
				}
				tempS = cvGet2D(srcYCrCb, tempy, k);
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
		
		// 区域的最上方向内收缩
		while( ( rect.y < srcYCrCb->height-1 ) && rect.height > 0 && sign == -1) {
			tempy = rect.y + 1;
			tempNum = 0;
			for(int k = rect.x; k < min(srcYCrCb->width, rect.x + rect.width); k++) {
				if( (tempy + rect.height) >= srcYCrCb->height-1) {
					break;
				}
				tempS = cvGet2D(srcYCrCb, tempy, k);
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
			} else {
				break;
			}
		}

		// 判断区域的最下方是向外膨胀还是向内收缩
		tempy = min( rect.y + rect.height + 1, srcYCrCb->height-1 );
		tempNum = 0;
		for(int k = rect.x; k < min(srcYCrCb->width, rect.x + rect.width); k++) {
			if( tempy >= srcYCrCb->height-1) {
				break;
			}
			tempS = cvGet2D(srcYCrCb, tempy, k);
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

		// 区域的最下方向外膨胀
		while( (rect.y + rect.height) < (srcYCrCb->height-1) && sign == 1) {
			tempy = min( rect.y + rect.height + 1, srcYCrCb->height-1 );
			tempNum = 0;
			for(int k = rect.x; k < min(srcYCrCb->width, rect.x + rect.width); k++) {
				if( tempy >= srcYCrCb->height-1) {
					break;
				}
				tempS = cvGet2D(srcYCrCb, tempy, k);
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
		
		// 区域的最下方向内收缩
		while( rect.height > 0 && sign == -1) {
			tempy = max( rect.y + rect.height - 1, 0 );
			tempNum = 0;
			for(int k = rect.x; k < min(srcYCrCb->width, rect.x + rect.width); k++) {
				if(tempy <= rect.y) {
					break;
				}
				tempS = cvGet2D(srcYCrCb, tempy, k);
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

		printf("%d, %d, %d, %d\n", aComp->rect.x, aComp->rect.y, aComp->rect.width, aComp->rect.height);

		cvRectangle(dst, cvPoint(aComp->rect.x, aComp->rect.y),
					cvPoint(aComp->rect.x + aComp->rect.width, aComp->rect.y + aComp->rect.height), 
					cvScalar(255, 0, 0), 1);
		i++;
	}

	cvReleaseImage(&srcYCrCb);
}