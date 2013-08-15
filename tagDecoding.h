/*
 * tagDecoding.h
 *
 *      Author:  Felix Lange, Juan
 */

#ifndef TAGDECODING_H_
#define TAGDECODING_H_

#include "cv.h"

void createIdealTagGridCoords(std::vector<cv::Mat> & coordinates){
	//create grid coordinates
	coordinates.clear();
	coordinates.resize(16); //because 4x4
	for(int y=0;y<4;++y){
		for(int x=0;x<4;++x){
			coordinates[y*4+x].zeros(2,1,CV_32FC1);
			coordinates[y*4+x].at<float>(0,0) = x+1;
			coordinates[y*4+x].at<float>(1,0) = y+1;
		}
	}
	//center coordinates (-2.5 center of 4x4 grid)
	for(int i=0;i<(int)coordinates.size();++i){
		coordinates[i].at<float>(0,0) -= 2.5;
		coordinates[i].at<float>(0,0) -= 2.5;
	}
}

std::vector<cv::Mat> getEmptyTagGrid(){
	std::vector<cv::Mat> tagGridCoords;
	tagGridCoords.resize(16); //4x4
	for(int i=0;i<16;++i){
		tagGridCoords[i].zeros(2,1,CV_32FC1);
	}
	return tagGridCoords;
}

void createTagTransformMat(cv::Mat & rotationMat, float angleRad, float radius){
	//create transformation matrix

//	double rotateScale[2][2] = {{fieldWidth*cos(orientation),   -1 * fieldWidth * sin(orientation)}
//							  ,{fieldWidth*sin(orientation),   fieldWidth * cos(orientation)}};

	float scale = radius / (2.0 * sqrt(2.0));

	rotationMat.zeros(2,2,CV_32FC1);
	rotationMat.at<float>(0,0) = scale*cos(angleRad);
	rotationMat.at<float>(0,1) = -1 * scale * sin(angleRad);
	rotationMat.at<float>(1,0) = scale*sin(angleRad);
	rotationMat.at<float>(1,1) = scale*scale * cos(angleRad);
}

void transformCoords(std::vector<cv::Mat> & idealTagCoords,std::vector<cv::Mat> & realTagCoords,cv::Mat & transformMat){
	for(int i=0;i<(int)realTagCoords.size();++i){
		realTagCoords[i] = transformMat * idealTagCoords[i];
	}
}


#endif /* TAGDECODING_H_ */
