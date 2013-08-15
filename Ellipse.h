/*
 * Ellipse.h
 *
 *  Created on: Aug 11, 2013
 *      Author: felix
 */

#ifndef ELLIPSE_H_
#define ELLIPSE_H_

#include "cv.h"

using namespace cv;

class Ellipse {
	public:
		// number of edge pixels supporting this ellipse
		unsigned int vote;
		// this ellipse' center pixel
		Point2i cen;
		// tuple of axis' length (major_axes_length, minor_axes_length)
		Size axis;
		// ellipse' orientation in degrees
		double angle;

		Ellipse(){
			vote = 0;
		}

		Ellipse(unsigned int vote, Point2i center, Size axis_length, double angle) {
			this->vote = vote;
			this->angle = angle;
			cen = center;
			axis = axis_length;
		}

		bool operator<(const Ellipse & elli2) const{
			return vote < elli2.vote;
		}
};

#endif /* ELLIPSE_H_ */
