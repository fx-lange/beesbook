#ifndef CVTOOLS_H_
#define CVTOOLS_H_

#include "ofMain.h"
#include "ofxCvGrayscaleImage.h"

void debugPrint(cv::Mat Gx){
	for(int row = 0; row < Gx.rows; ++row) {
	    for (int col = 0; col < Gx.cols; ++col) {
	    	cout << (int)Gx.at<float>(row,col)<< "\t";
	    }
		std::cout << std::endl;
	}
}

void debugPrintPixels(unsigned char * pixels,int w,int h,int widthStep,int heightStep){
	cout << "--- uchar --- " << endl;
	for(int y=0;y<h;++y){
		for(int x=0;x<w;++x){
			unsigned char uc = pixels[x*heightStep+y*widthStep];
			char c = uc;
			cout << ofToString((int)pixels[x*heightStep+y*widthStep]) << "_"<< (int)uc << "_" << (int)c <<" \t\t ";
		}
		cout << endl;
	}
}

void debugPrintPixels(char * pixels,int w,int h,int widthStep){
	cout << "--- char --- " << endl;
	for(int y=0;y<h;++y){
		for(int x=0;x<w;++x){
			char c = pixels[x+y*widthStep];
			cout << ofToString((int)pixels[x+y*widthStep]) << "_"<< "_" << (int)c <<" \t\t ";
		}
		cout << endl;
	}
}

void debugPrintPixels(float * pixels,int w,int h,int widthStep){
	cout << "--- float --- " << endl;
	for(int y=0;y<h;++y){
		for(int x=0;x<w;++x){
			float f = pixels[x+y*widthStep];
			cout << f << "\t\t";
		}
		cout << endl;
	}
}

void debugPrintPixels(uchar * pixels,int w,int h){
	for(int y=0;y<h;++y){
		for(int x=0;x<w;++x){
			cout << ofToString((int)pixels[x+y*w]) << " \t ";
		}
		cout << endl;
	}
}
//void debugPrintPixels(short * pixels,int w,int h){
//	for(int y=0;y<h;++y){
//		for(int x=0;x<w;++x){
//			cout << ofToString((int)pixels[x+y*w]) << " \t ";
//		}
//		cout << endl;
//	}
//}

#endif /* CVTOOLS_H_ */
