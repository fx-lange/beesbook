#pragma once

#include "ofMain.h"

#include "ofxOpenCv.h"

#include "ROISelection.h"
#include "ofxCvInputManager.h"

#include "ofxSnapshot.h"

class beesbookApp : public ofBaseApp{

	public:
		void setup();
		void setupGui();
		void update();
		void inputChanged();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);		

        #ifdef _USE_LIVE_VIDEO
		  ofVideoGrabber 		vidGrabber;
		#else
		  ofVideoPlayer 		vidPlayer;
		#endif

        ofxCvColorImage			colorImg;

        ofxCvContourFinder 	contourFinder;

		int 				threshold;
		bool				bLearnBakground;


		// ------------------------ //

		ROISelection roiSelection;
		ofxSnapshot snapshot;

};

