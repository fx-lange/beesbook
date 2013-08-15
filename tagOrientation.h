/*
 * tagOrientation.h
 *
 *      Author: Felix Lange, Juan
 */

#ifndef TAGORIENTATION_H_
#define TAGORIENTATION_H_

#include "cv.h"

void getCenterImg(cv::Mat & srcMat,cv::Mat & centerImg,float centerSize, float centerX, float centerY, float radius){

	cv::Rect centerRoi = cvRect(centerX-centerSize/2.f,
			centerY-centerSize/2.f,
			centerSize,centerSize);

	//copy the ROI of srcMat into centerImg
	centerImg = srcMat(centerRoi).clone();
}

void calcGradients(cv::Mat & srcMat, cv::Mat & dstG, cv::Mat & dstGx, cv::Mat & dstGy,int kernelSize,float scale){
	cv::Mat absGx, absGy;
	//TODO gx and gy switched?

	//Gradient X
	cv::Sobel(srcMat,dstGx,CV_32F,1,0,kernelSize);
	absGx = cv::abs(dstGx);

	//Gradient Y
	cv::Sobel(srcMat,dstGy,CV_32F,0,1,kernelSize);
	absGy = cv::abs(dstGy);

	//Total Gradient
	cv::addWeighted( absGx, scale, absGy, scale, 0, dstG );
}

float getOrientationRad(cv::Mat Gx,cv::Mat Gy, cv::Mat G,int count,std::vector<cv::Point2d> & maxGradientCoords){
	//initialization
	cv::Mat copyOfG;
	G.copyTo(copyOfG);
	int maxIdx[2];
	double sumAngleRad = 0.0;
	double testMaxval;

	//find max gradients and calculate their angles
	for(int i=0;i<count;++i){

		//find max i of G
		cv::minMaxIdx(copyOfG, 0, &testMaxval, 0, maxIdx,cv::Mat());
		//delete max i in G
		copyOfG.at<float>(maxIdx[0],maxIdx[1]) = 0;

		//orientation of max i
		float x = Gx.at<float>(maxIdx[0],maxIdx[1]);
		float y = Gy.at<float>(maxIdx[0],maxIdx[1]);

		//TODO swap x y - why?!
		swap(x,y);

		float angleRad = atan2(y,x);
		sumAngleRad += angleRad;

		//save coordinates of max i for drawing
		maxGradientCoords.push_back(Point2d(maxIdx[1],maxIdx[0]));
	}

	double avgAngleRad = sumAngleRad / (float)count;

	return avgAngleRad;
}


#endif /* TAGORIENTATION_H_ */
