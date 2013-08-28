/*
 * ROISelection.h
 *
 *  Created on: May 29, 2013
 *      Author: felix
 */

#ifndef ROISELECTION_H_
#define ROISELECTION_H_

#include "ofxTangibleHandle.h"

class ROISelection : public ofxTangibleHandle{
public:
	ROISelection();
	virtual ~ROISelection();

	virtual void setup(float x,float y,float w,float h,float cornerWidth=15);
	virtual void moveInternal(float dx, float dy);
	virtual void draw();

	ofRectangle getROI();

	vector<ofxTangibleHandle> handles;

protected:
	virtual bool isOver(float px, float py);
};

#endif /* ROISELECTION_H_ */
