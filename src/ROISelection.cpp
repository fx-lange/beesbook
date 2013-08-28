/*
 * ROISelection.cpp
 *
 *  Created on: May 29, 2013
 *      Author: felix
 */

#include "ROISelection.h"

ROISelection::ROISelection(){}

ROISelection::~ROISelection(){}

void ROISelection::setup(float x,float y,float w, float h,float cornerWidth){
	ofxTangibleHandle::setup(x,y,w,h);

	ofxTangibleHandle h1,h2,h3,h4;

	h1.setup(x,y,cornerWidth,cornerWidth);
	h2.setup(x+w,y+h,cornerWidth,cornerWidth);
	h3.setup(x+w,y,cornerWidth,cornerWidth);
	h4.setup(x,y+h,cornerWidth,cornerWidth);

	h1.startListeningTo(this);
	h2.startListeningTo(this);

	ofxListener::listenToEachOther(h1,h3,TANGIBLE_MOVE_VERTICAL);
	ofxListener::listenToEachOther(h1,h4,TANGIBLE_MOVE_HORIZONTAL);
	ofxListener::listenToEachOther(h2,h3,TANGIBLE_MOVE_HORIZONTAL);
	ofxListener::listenToEachOther(h2,h4,TANGIBLE_MOVE_VERTICAL);

	handles.push_back(h1);
	handles.push_back(h2);
	handles.push_back(h3);
	handles.push_back(h4);

}

void ROISelection::draw(){
	ofPushStyle();
	for(int i=0;i<(int)handles.size();++i){
		handles[i].draw();
	}

	ofNoFill();
	ofRect(ofRectangle(handles[0],handles[1]));
	ofPopStyle();
}

bool ROISelection::isOver(float mx,float my){
	x = (handles[0].x+ handles[1].x) / 2.f;
	y = (handles[0].y+ handles[1].y) / 2.f;

	width = fabs(handles[0].x - handles[1].x);
	height = fabs(handles[0].y - handles[1].y);
	return ofxTangibleHandle::isOver(mx,my);
}

ofRectangle ROISelection::getROI(){
	return ofRectangle(handles[0],handles[1]);
}

void ROISelection::moveInternal(float dx, float dy){
	for(int i=0;i<(int)handles.size();++i){
		if(handles[i].isPressed()){
			return;
		}
	}
	ofxTangibleCore::moveInternal(dx,dy);
}
