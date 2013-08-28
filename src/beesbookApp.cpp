#include "beesbookApp.h"
#include "ofxGui.h"
#include "cv.h"
#include "highgui_c.h"
#include "cvTools.h"
#include "beesbook/ellipseDetection.h"
#include "beesbook/tagOrientation.h"
#include "beesbook/tagDecoding.h"

/**TODO
 *
 *	beispielbild
 *	datentyp probleme verstehen!
 *
 *	es funktioniert mit den oFBildern (uchar) aber eben unsigned
 *	8u vs 16s - bilder darstellen und verstehen
 */

bool eNextLoop = false;
bool eNewEllipseDetection = false;
float orientation;

float scale;
ofPoint translate;
float rowWidth;

ofxPanel gui;
ofxFloatSlider thresholdValue, cannyLow, cannyHigh, akkuThresh;
ofxFloatSlider minCenterDistance, minRadius, maxRadius, scaleDown;
ofxFloatSlider centerAreaSize, testRotate;
ofxIntSlider sobelKernelSize, topX;

ofxCvColorImage selectionImg;
ofxCvGrayscaleImage grayImg, binaryImg, cannyImg, centerAreaImg, imageGx, imageGy, imageG;
vector<cv::Vec3f> circles;
vector<cv::Point2d> maxCoords;

ofxCvImageInput halfImg,fullImg,centered0,centered45,fc2,cl0,cl9,cq100,ellipseTest,ellipsigeEllipse;
ofxCvLiveInput live;
ofxCvInputManager inputs;

bool ePrintDebug = false;

vector<vector<double> > realTagCoords;
vector<cv::Mat> idealTagCoords;
vector<float> grayMeanValues;

//grid reference points (for debugging)
int downLeftCornerIdx = 3; // 4,1 in xy grid coords
int downRightCornerIdx = 15; // 4,4 in xy grid coords
int whiteCenterIdx = 6;
int whiteCenterIdx2 = 10;
int blackCenterIdx = 5;
int blackCenterIdx2 = 9;

//new ellipse detection
Ellipse ellipseCandidate;

void ellipseDraw(Ellipse & ellipse){
	ofPushStyle();
	ofNoFill();
	ofSetColor(200,255,30);
//	ellipse(orig, (*cand).cen, (*cand).axis, (*cand).angle, 0, 360, Scalar(0,255,0),1);
	ofPushMatrix();
	ofTranslate(ellipse.cen.x,ellipse.cen.y);
	ofRotate(ellipse.angle);
	ofTranslate(-ellipse.cen.x,-ellipse.cen.y);
	ofEllipse(ellipse.cen.x,ellipse.cen.y,ellipse.axis.width*2,ellipse.axis.height*2);
	ofPopMatrix();
	ofPopStyle();
}

//--------------------------------------------------------------
void beesbookApp::setup(){
    ofBackground(100,100,100);
    ofEnableAlphaBlending();
    inputs.setup();
    fullImg.name = "full.jpg";
    fullImg.image.loadImage(fullImg.name);
    halfImg.name = "half.jpg";
    halfImg.image.loadImage(halfImg.name);
    centered0.name = "centered rot 0";
    centered0.image.loadImage("marker/centered_4_2_rot_0_gray.png");
    centered45.name = "centered rot 45";
    centered45.image.loadImage("marker/centered_4_2_rot_45_gray.png");
    fc2.name = "markerAndBees/fc2_save.png";
    fc2.image.loadImage(fc2.name);
    cl0.name = "markerAndBees/half2_CL0.png";
    cl0.image.loadImage(cl0.name);
	cl9.name = "markerAndBees/half_CL9.png";
	cl9.image.loadImage(cl9.name);
	cq100.name = "markerAndBees/half_CQ100.jpg";
	cq100.image.loadImage(cq100.name);
	ellipseTest.name = "ellipse_test_single.png";
	ellipseTest.image.loadImage(ellipseTest.name);
	ellipsigeEllipse.name = "ellipsige_ellipse.png";
	ellipsigeEllipse.image.loadImage(ellipsigeEllipse.name);
	inputs.addInputSource(&ellipseTest);
	inputs.addInputSource(&ellipsigeEllipse);
    inputs.addInputSource(&centered45);
    inputs.addInputSource(&centered0);
    inputs.addInputSource(&fc2);
    inputs.addInputSource(&cl0);
    inputs.addInputSource(&cl9);
    inputs.addInputSource(&cq100);
    inputs.addInputSource(&fullImg);
    inputs.addInputSource(&halfImg);
//    live.setup(640,480);
//    inputs.addInputSource(&live);

    rowWidth = ofGetWidth() / 4.f;
    scale = rowWidth / halfImg.image.width;

    colorImg.allocate(halfImg.image.width,halfImg.image.height);
    colorImg.setFromPixels(halfImg.getPixels(),halfImg.image.width,halfImg.image.height);

    roiSelection.setup(50 + ofGetWidth()-rowWidth,50,300,300);

//    createIdealTagGridCoords(idealTagCoords);

    setupGui();
}

void beesbookApp::setupGui(){
	gui.setup("gui","gui.xml",ofGetWidth()- rowWidth + 15,400);
	gui.add(thresholdValue.setup("tresh",150,0,255));
//	gui.add(cannyLow.setup("canny low",50,0,255));
	ofxPanel * houghPanel = new ofxPanel();
	houghPanel->setup("hough trans");
	houghPanel->add(cannyHigh.setup("canny high",200,0,255));
	houghPanel->add(minCenterDistance.setup("min dis betw. centers",20,0,200));
	houghPanel->add(minRadius.setup("min radius",50,0,500));
	houghPanel->add(maxRadius.setup("max radius",200,0,1000));
	houghPanel->add(akkuThresh.setup("akku thresh",15,0,100));
	gui.add(houghPanel);
	ofxPanel * orientationPanel = new ofxPanel();
	orientationPanel->setup("orientation");
	orientationPanel->add(centerAreaSize.setup("centerROI size",15,5,50));
	orientationPanel->add(sobelKernelSize.setup("sobel kernel 3 5 7",0,0,2));
	orientationPanel->add(topX.setup("top X orientation",20,10,40));
	orientationPanel->add(testRotate.setup("testRotate",0,-180,180));
	orientationPanel->add(scaleDown.setup("scale GxGy",0.001,0.0001,0.01));
	gui.add(orientationPanel);

	gui.loadFromFile("gui.xml");
}

void beesbookApp::inputChanged(){
	scale = rowWidth / inputs.getInputWidth();

	colorImg.allocate(inputs.getInputWidth(),inputs.getInputHeight());
	colorImg.setFromPixels(inputs.getPixels(),colorImg.width,colorImg.height);
}

//--------------------------------------------------------------
void beesbookApp::update(){
	inputs.update();
	if(inputs.bInputChanged){
		inputChanged();
		inputs.bInputChanged = false;
	}else if(inputs.isNewFrame()){
		colorImg.setFromPixels(inputs.getPixels(),colorImg.width,colorImg.height);
	}

	//get and transform ROI depend to the translations of the source image
	ofRectangle roi = roiSelection.getROI();
	roi.x -= ofGetWidth()-colorImg.width*scale;
	roi.x /= scale;
	roi.y /= scale;
	roi.width /= scale;
	roi.height /= scale;

	//copy ROI from source
	selectionImg.allocate(floor(roi.width),floor(roi.height)); //REVISIT wird nicht gleich gerundet?
	colorImg.setROI(floor(roi.x),floor(roi.y),floor(roi.width),floor(roi.height));
	selectionImg.setFromPixels(colorImg.getRoiPixelsRef());
	colorImg.resetROI();

	//color selection -> gray selection
	grayImg.allocate(roi.width,roi.height);
	grayImg = selectionImg;

	//thresh - only for drawing
	binaryImg.allocate(roi.width,roi.height);
	binaryImg = grayImg;
	binaryImg.threshold(thresholdValue);

	//canny - only for drawing (to see what the hough transform is doing)
	cannyImg.allocate(roi.width,roi.height);
//	cv::Canny(grayImg.get,cannyImg.getCvImage(),5.,5.);//(double)cannyLow.value.getValue(),(double)cannyHigh.value.getValue());
	cvCanny(grayImg.getCvImage(),cannyImg.getCvImage(),cannyHigh/2.f,cannyHigh);
	cannyImg.flagImageChanged();

	//ellipse detection //TODO use these results instead of houghCircles
	if(eNewEllipseDetection){
		cv::Mat cannyMat (cannyImg.getCvImage());
		ellipseCandidate = detect_Ellipse(cannyMat);
		eNewEllipseDetection = false;
	}

	//hough transformation for circles //TODO only use detect_Ellipse()
	cv::Mat houghInputMat(grayImg.getCvImage());
	circles.clear();
	cv::HoughCircles(houghInputMat,circles,CV_HOUGH_GRADIENT,1,minCenterDistance,cannyHigh,akkuThresh,minRadius,maxRadius);

	//orientation
	if(circles.size()>0 && eNextLoop){
		eNextLoop = false;
		cv::Vec3f & circle = circles[0];
		ofPoint center = ofPoint(circle[0],circle[1]);
		float radius = circle[2];

		//test rotation
		ofxCvGrayscaleImage grayTmp;
		grayTmp = grayImg;
		grayTmp.rotate(testRotate,circle[0],circle[1]);

		//get center img //TODO use a bigger ROI but same size for G (G,Gx,Gy)
		cv::Mat centerImgMat;
		cv::Mat grayImgMat = grayTmp.getCvImage();
		getCenterImg(grayImgMat,centerImgMat,centerAreaSize,center.x,center.y);

		//WORKAROUND for drawing - //TODO only realloc if centerAreaSize has changed - MEMORY LEAK
		IplImage tmpImg = (IplImage)centerImgMat;
		IplImage * centerImg = cvCloneImage(&tmpImg);
		centerAreaImg.allocate(centerAreaSize,centerAreaSize);//TODO GUI
		centerAreaImg = centerImg;

		//calc gradients
		cv::Mat dstG,dstGx,dstGy;
		calcGradients(centerImgMat,dstG,dstGx,dstGy,3+sobelKernelSize*2,scaleDown);
		orientation = getOrientationRad(dstGx,dstGy,dstG,topX,maxCoords);

		//WORKAROUND for drawing - //TODO only realloc if centerAreaSize has changed - MEMORY LEAK
//		IplImage tmpImgG = (IplImage)dstG;
//		IplImage * tmpClonePtrG = cvCloneImage(&tmpImgG);
//		imageG.allocate(centerAreaSize,centerAreaSize);
//		imageG = tmpClonePtrG;
//
//		IplImage tmpImgGx = (IplImage)dstGx;
//		IplImage * tmpClonePtrGx = cvCloneImage(&tmpImgGx);
//		imageGx.allocate(centerAreaSize,centerAreaSize);
//		imageGx = tmpClonePtrGx;
//
//		IplImage tmpImgGy = (IplImage)dstGy;
//		IplImage * tmpClonePtrGy = cvCloneImage(&tmpImgGy);
//		imageGy.allocate(centerAreaSize,centerAreaSize);
//		imageGy = tmpClonePtrGy; <--- NOT WORKING


		// ----- !!!!!!!!!!!!!!!!!!! --- //
		// --- leave bugs behind!! --- //

		double fieldWidth = (double)radius / (2.0 * sqrt(2.0));

		//Create Ideal Tag Coords y0 x... y1 x.... y2 x....
		vector<vector<double> > idealTagCoord;
		idealTagCoord.resize(16);
		for(int y=0;y<4;++y){
			for(int x=0;x<4;++x){
				idealTagCoord[y*4+x].resize(2);
				idealTagCoord[y*4+x][0] = x+1;
				idealTagCoord[y*4+x][1] = y+1;
			}
		}


		//-2.5 (center of 4x4 grid)
		for(int i=0;i<(int)idealTagCoord.size();++i){
			idealTagCoord[i][0] -= 2.5;
			idealTagCoord[i][1] -= 2.5;
			cout << idealTagCoord[i][0] << "/" << idealTagCoord[i][1] << endl;
		}

		//create rotation matrix
		double rotateScale[2][2] = {{fieldWidth*cos(orientation),-1 * fieldWidth * sin(orientation)}
									,{fieldWidth*sin(orientation),fieldWidth * cos(orientation)}};

		//create real tag coordinates - init with 0
		realTagCoords.clear();
		realTagCoords.resize(16);
		for(int i=0;i<(int)realTagCoords.size();++i){
			realTagCoords[i].resize(2);
			realTagCoords[i][0] = 0;
			realTagCoords[i][1] = 0;
		}

		//rotate ideal tag coordinates -> real tag coordinates (TODO use cv matrix multiplication) -> multiply each coords with the rotation matrix
//		cv::Mat(cv::Size(2,2realTagCoords[k][i] += rotateScale[i][j] * idealTagCoord[k][j];)); //TODO use cv::Mat
		for(int k=0;k<(int)realTagCoords.size();++k){
			for(int i = 0;i< 2;++i){
				for( int j = 0; j < 2; ++j){
					realTagCoords[k][i] += rotateScale[i][j] * idealTagCoord[k][j];
				}
			}
		}

		//add center to real tag coords
		for(int i=0;i<(int)realTagCoords.size();++i){
			realTagCoords[i][0] += center.x;
			realTagCoords[i][1] += center.y;
		}

		calcTagGrayMeanValues(houghInputMat,realTagCoords,grayMeanValues);

		float referenceWhite = (grayMeanValues[whiteCenterIdx] + grayMeanValues[whiteCenterIdx2]) / 2.f;
		float referenceBlack = (grayMeanValues[blackCenterIdx] + grayMeanValues[blackCenterIdx2]) / 2.f;

		getBinaryTagValues(grayMeanValues,referenceWhite,referenceBlack);

		//sorted:
		// |  |
		// v  v
		// -> ->




		//OR NEW - not working yet //TODO
//		realTagCoords.clear();
//		realTagCoords.resize(16);
//
//		vector<cv::Mat> realTagMat = getEmptyTagGrid();
//
//		//get transformation matrix
//		cv::Mat transformMat = createTagTransformMat(orientation,radius);
//
//		transformCoords(idealTagCoords,realTagMat,transformMat);
//
//
//		//add center to real tag coords
//		for(int i=0;i<(int)realTagCoords.size();++i){
//			realTagCoords[i][0] += (center.x + realTagMat[i].at<float>(0,0));
//			realTagCoords[i][1] += (center.y + realTagMat[i].at<float>(0,1));
//		}
	}

}

//--------------------------------------------------------------
void beesbookApp::draw(){
	ofBackground(0,0,0);
	ofSetColor(255,255,255);

	ofPushMatrix();
	ofTranslate(ofGetWidth()-colorImg.width*scale,0);
	ofScale(scale,scale);
	inputs.draw(0,0);

	ofPopMatrix();

	float scaleTmp = rowWidth / selectionImg.width;
	ofPushMatrix();
	ofScale(scaleTmp,scaleTmp);
	selectionImg.draw(0,0);
	ofPopMatrix();

	ofPushMatrix();
	ofTranslate(rowWidth,0);
	ofScale(scaleTmp,scaleTmp);
	grayImg.draw(0,0);
	ellipseDraw(ellipseCandidate);

	ofPushStyle();
	ofSetColor(0,255,0);
	for(int i=0;i<(int)realTagCoords.size();++i){
		for(int j=0;j<(int)realTagCoords.size();++j){
			if(j == downLeftCornerIdx || j == downRightCornerIdx){ // 4,1 4,4
				ofSetColor(0,255,0);
			}else{
				ofSetColor(255,0,0);
			}
			ofCircle(realTagCoords[j][1],realTagCoords[j][0],1);
		}
	}
	ofPopStyle();
	ofPopMatrix();

	ofPushMatrix();
	ofTranslate(rowWidth*2,0);
	ofScale(scaleTmp,scaleTmp);
	binaryImg.draw(0,0);
	ofPopMatrix();

	//canny and circle
	ofPushMatrix();
	ofPushStyle();
	ofTranslate(rowWidth*2,binaryImg.height * scaleTmp);
	ofScale(scaleTmp,scaleTmp);
	cannyImg.draw(0,0);
	ofSetColor(255,0,0);
	ofNoFill();

	//new ellipse detection
	ellipseDraw(ellipseCandidate);

	//real tag coords
	ofPushStyle();
	for(int i=0;i<(int)circles.size();++i){
		cv::Vec3f & vec = circles[i];
		ofCircle(vec.val[0],vec.val[1],vec.val[2]);
		cout << "circle" << endl;
		if(i==0){
			for(int j=0;j<(int)realTagCoords.size();++j){

				ofPushStyle();
				ofFill();
				ofSetColor(grayMeanValues[j]);
				ofCircle(realTagCoords[j][1], realTagCoords[j][0] ,5); //REVISIT why swap?!
				ofSetColor(255,0,0);
				ofDrawBitmapString(ofToString(grayMeanValues[j]),realTagCoords[j][1], realTagCoords[j][0]);
				ofPopStyle();

				if(j == downLeftCornerIdx || j == downRightCornerIdx){
					ofSetColor(0,255,0);
				}else{
					ofSetColor(255,0,0);
				}
				ofCircle(realTagCoords[j][1], realTagCoords[j][0] ,1); //REVISIT why swap?!

			}
		}
	}
	ofPopStyle();

	ofPopStyle();
	ofPopMatrix();

	// draw gradients
//	ofPushMatrix();
//	ofPushStyle();
//	ofTranslate(rowWidth,binaryImg.height*scaleTmp);
//	float scaleROIs = rowWidth / (centerAreaSize * 3.f);
//	ofScale(scaleROIs,scaleROIs);
//	centerAreaImg.draw(0,0);
//	imageGx.draw(centerAreaSize,0);
//	imageGy.draw(centerAreaSize*2,0);
//	imageG.draw(0,centerAreaSize);
//	ofTranslate(0,centerAreaSize);
//	ofSetColor(255,0,0);
//	for(int i=0;i<(int)maxCoords.size();++i){
//		ofEllipse(maxCoords[i].x,maxCoords[i].y,1,1);
//	}
//	ofPopStyle();
//	ofPopMatrix();

	roiSelection.draw();

	snapshot.draw();

	ofSetColor(255,255,255);
	ofDrawBitmapString("source: "+inputs.getName() + "\n  fps: "+ofToString(ofGetFrameRate()),ofGetWidth() - 300, 30);
	ofSetColor(255,0,0);
	float angle = ofRadToDeg(orientation) ;//< 0 ? result + 180 : result -180;

	ofDrawBitmapString("angle: "+ofToString(angle),ofGetWidth() - 300, 500);
	gui.draw();

}

//--------------------------------------------------------------
void beesbookApp::keyPressed(int key){

	switch (key){
		case ' ':
//			bLearnBakground = true;
//			break;
		case '+':
			scale += 0.1;
			break;
		case '-':
			scale -= 0.1;
			break;
		case 'd':
			ePrintDebug = true;
			break;
		case 'n':
			eNextLoop = true;
			break;
		case 'e':
			eNewEllipseDetection = true;
			break;
	}
}

//--------------------------------------------------------------
void beesbookApp::keyReleased(int key){

}

//--------------------------------------------------------------
void beesbookApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void beesbookApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void beesbookApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void beesbookApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void beesbookApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void beesbookApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void beesbookApp::dragEvent(ofDragInfo dragInfo){

}
