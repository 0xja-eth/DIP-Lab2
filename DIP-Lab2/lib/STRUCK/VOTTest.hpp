/*
 *  Author : Tomas Vojir
 *  Date   : 2013-06-05
 *  Desc   : Simple class for parsing VOT inputs and providing
 *           interface for image loading and storing output.
 */

#ifndef CPP_VOT_H
#define CPP_VOT_H

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs/legacy/constants_c.h"

#include "BaseTest.hpp"

struct BB {
	cv::Point a, b, c, d;

	cv::Rect toRect() {
		cv::Point minP = a, maxP = a;

		minP.x = min(minP.x, b.x); minP.y = min(minP.y, b.y);
		minP.x = min(minP.x, c.x); minP.y = min(minP.y, c.y);
		minP.x = min(minP.x, d.x); minP.y = min(minP.y, d.y);

		maxP.x = max(maxP.x, b.x); maxP.y = max(maxP.y, b.y);
		maxP.x = max(maxP.x, c.x); maxP.y = max(maxP.y, c.y);
		maxP.x = max(maxP.x, d.x); maxP.y = max(maxP.y, d.y);

		return cv::Rect(minP, maxP);
	}
};

class VOT : public BaseTest {
public:
	/*
	VOT(const std::string & region_file, const std::string & images, const std::string & ouput) {
		p_region_stream.open(region_file.c_str());
		if (p_region_stream.is_open()) {
		} else {
			std::cerr << "Error loading initial region in file " << region_file << "!" << std::endl;
			rects.push_back(cv::Rect(0, 0, 0, 0));
		}

		p_images_stream.open(images.c_str());
		if (autoImage = !p_images_stream.is_open()) {
			// std::cerr << "Error loading image file " << images << "!" << std::endl;
			imageDir = images;
		}

		p_output_stream.open(ouput.c_str());
		if (!p_output_stream.is_open())
			std::cerr << "Error opening output file " << ouput << "!" << std::endl;
	}
	*/
	inline BB currentTruthBB() {
		return boxes[counter];
	}

	inline virtual std::string generateName() {
		char name_[16];
		sprintf(name_, "%08d.jpg", counter+1);
		return imageDir + name_;
	}

	inline virtual void processInput() {
		float x, y; char ch;
		while (p_region_stream >> x) {
			BB box;
			p_region_stream >> ch >> y >> ch;
			box.a = cv::Point(x, y);

			p_region_stream >> x >> ch >> y >> ch;
			box.b = cv::Point(x, y);

			p_region_stream >> x >> ch >> y >> ch;
			box.c = cv::Point(x, y);

			p_region_stream >> x >> ch >> y;
			box.d = cv::Point(x, y);

			boxes.push_back(box);
			rects.push_back(box.toRect());
		}
	}

	virtual void drawTruth(cv::Mat frame) {
		BB truth = currentTruthBB();

		std::vector<cv::Point> contour;
		contour.reserve(4);
		contour.push_back(truth.a);
		contour.push_back(truth.b);
		contour.push_back(truth.c);
		contour.push_back(truth.d);

		cv::polylines(frame, contour, true, cv::Scalar(255, 0, 0), 2);
	}

	std::vector<BB> boxes;

};

#endif //CPP_VOT_H
