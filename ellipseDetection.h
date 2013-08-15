// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "Ellipse.h"
#include <vector>

using namespace cv;

//ellipse constraints
#define MINMAJOR 40
#define MAXMAJOR 46
#define MINMINOR 25
#define MAXMINOR 46
#define EL_THRESH 42


double pointDistance(Point2d p, Point2d q) {
	return (sqrt((q.y-p.y)*(q.y-p.y) + (q.x-p.x)*(q.x-p.x)));
}

Ellipse detect_Ellipse(Mat &ec) {
	std::cout << "Start ..." << std::endl;
	//edge_pixel array, all edge pixels are stored in this array
	vector<Point2i> ep;
	vector<Ellipse>::iterator cand;
	unsigned int max_ind;

	double a,b,tau,d,f,alpha,costau;
	// a - half length of the ellipse' major axis
	// b - half length of the ellipse' minor axis
	// alpha - ellipse' orientation

	vector< vector<Point2i> > support;
	for (int i = 0; i < MAXMINOR/2+1; ++i) {
		support.push_back(vector<Point2i>());
	}


	Point2d p,p1,p2,center;
	vector<Ellipse> candidates;

	// (1) all white (being edge) pixels are written into the ep array
	MatIterator_<unsigned char> mit,end;
	for (mit=ec.begin<unsigned char>(), end=ec.end<unsigned char>(); mit != end; ++mit) {
		if (*mit.ptr == 255) {
			ep.push_back(mit.pos());
		}
	}

	// (2) initiate the accumulator array
	std::vector<unsigned int> accu(MAXMINOR/2+1);
	unsigned int vote_minor;

	// (3) loop through each edge pixel in ep
	for(std::vector<Point2i>::iterator it = ep.begin(); it != ep.end(); ++it) {
		p1.x = (*it).x;
		p1.y = (*it).y;

		// (4) loop through each other edge pixel in ep and propose a major axis between p1 and p2
		for(std::vector<Point2i>::iterator it2 = ep.begin(); it2 != ep.end(); ++it2) {
			if ((*it) != (*it2)) {
				p2.x= (*it2).x;
				p2.y = (*it2).y;
				// the proposed ellipse' length of the major axis lies between MINMAJOR and MAXMAJOR
				if (pointDistance(p1,p2) > MINMAJOR && pointDistance(p1,p2) < MAXMAJOR ) {
					// (5) calculate the ellipse' center, half length of the major axis (a) and orientation (alpha) based on p1 and p2
					center.x = (p1.x+p2.x)/2;
					center.y = (p1.y+p2.y)/2;
					a = pointDistance(p1,p2)/2;
					alpha = atan((p2.y-p1.y)/(p2.x-p1.x));
					// (6) loop through each third edge pixel eventually lying on the ellipse
					for (std::vector<Point2i>::iterator it3 = ep.begin(); it3 != ep.end(); ++it3) {

						if ((*it3) != (*it2) && (*it3) != (*it)) {
							p.x = (*it3).x;
							p.y = (*it3).y;
							d = pointDistance(p,center);

							if (d > MINMINOR/2.0 && d <= a) {
								// (7) estimate the half length of the minor axis (b)
								f = pointDistance(p,p2);
								costau = (a*a+d*d-f*f)/(2*a*d);
								tau = acos(costau);

								b = sqrt((a*a*d*d*sin(tau)*sin(tau))/((a*a)-(d*d*cos(tau)*cos(tau))));

								// (8) increment the accumulator for the minor axis' half length (b) just estimated
								if (b <= MAXMINOR/2.0 && b >= MINMINOR/2.0) {
									accu[cvRound(b) - 1] += 1;
									//support[cvRound(b) - 1].push_back((*it3));
								}
							}
						}
					}

					// (10) find the maximum within the accumulator, is it above the threshold?
					max_ind = std::max_element(accu.begin(),accu.end()) -accu.begin()+1;
					vote_minor = (*(max_ind+accu.begin()-1));
					if (vote_minor >= EL_THRESH) {
						//std::cout << "sup size , vote: " << support[max_ind].size() << " , " << vote_minor <<std::endl;
						//std::cout << "supp points" << support[max_ind] << std::endl;

						// (11) output ellipse parameters
						std::cout << "Center: " << center << "Major axis length: " << a*2 << ", Minor axis length: " << 2*(max_ind) << ", Orientation: " << alpha << ", votum: " << vote_minor << std::endl;
						Point2i cen = Point2i(cvRound(center.x), cvRound(center.y));
						Size axis = Size(cvRound(a),max_ind);
						double angle = (alpha*180)/CV_PI;
						candidates.push_back(Ellipse(vote_minor, cen, axis, angle));

						// (12) remove associated edge pixels from ep
						//for (vector<Point2i>::iterator sup = support[max_ind].begin(); sup != support[max_ind].end(); ++sup) {
						//	ep.erase(std::remove(ep.begin(),ep.end(),(*sup)), ep.end());
						//}
					}

					// (13) clear accumulator anyway
					for (std::vector<unsigned int>::iterator ini = accu.begin(); ini != accu.end(); ++ini) {
						(*ini) = 0;
					}
					// clear support
					//for (std::vector<vector<Point2i>>::iterator sup = support.begin(); sup != support.end(); ++sup) {
					//	(*sup).clear();
					//}
				}
			}
		}
	}
	std::cout << "Ellipse detection found " << candidates.size() << " candidates." << std::endl;
	std::sort(candidates.begin(),candidates.end());
	cand = candidates.begin();
//	for (int i = 0; i < ellipse_count; ++i) {
//// TODO return ->		ellipse(orig, (*cand).cen, (*cand).axis, (*cand).angle, 0, 360, Scalar(0,255,0),1);
//		cand += 1;
//	}
	std::cout << candidates.size() << " ellipse found" << endl;
	std::cout << "Xie ellipse detection done!" << std::endl;
	return candidates[0];
}
