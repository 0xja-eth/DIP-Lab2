/*
 *  Author : Tomas Vojir
 *  Date   : 2013-06-05
 *  Desc   : Simple class for parsing VOT inputs and providing
 *           interface for image loading and storing output.
 */

#ifndef CPP_OTB_H
#define CPP_OTB_H

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs/legacy/constants_c.h"

#include "BaseTest.hpp"

class OTB : public BaseTest {
public:
	/*
	OTB(const std::string & region_file, const std::string & images, const std::string & ouput) {
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

	inline virtual std::string generateName() {
		char name_[16];
		sprintf(name_, "%04d.jpg", ++counter);
		return imageDir + name_;
	}

	inline virtual void processInput() {
		int x, y, w, h;
		while (p_region_stream >> x) {
			p_region_stream >> y >> w >> h;
			rects.push_back(cv::Rect(x, y, w, h));
		}
	}

};

#endif //CPP_VOT_H
