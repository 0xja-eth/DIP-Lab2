/*
 *  Author : Tomas Vojir
 *  Date   : 2013-06-05
 *  Desc   : Simple class for parsing VOT inputs and providing
 *           interface for image loading and storing output.
 */

#ifndef CPP_BASETEST_H
#define CPP_BASETEST_H

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <ctime>
#include <cmath>

#include <windows.h>

#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs/legacy/constants_c.h"

class BaseTest {
public:
	void setup(const std::string & region_file, const std::string & images, const std::string & ouput) {
		p_region_stream.open(region_file.c_str());
		if (p_region_stream.is_open()) processInput();
		else {
			std::cerr << "Error loading initial region in file " << region_file << "!" << std::endl;
			rects.push_back(cv::Rect(0, 0, 0, 0));
		}

		auto cnt = rects.size();
		outRects = new cv::Rect[cnt];
		distances = new double[cnt];
		overlapSpaces = new double[cnt];
		times = new double[cnt];

		p_images_stream.open(images.c_str());
		if (autoImage = !p_images_stream.is_open()) {
			// std::cerr << "Error loading image file " << images << "!" << std::endl;
			imageDir = images;
		}

		p_output_stream.open(ouput.c_str());
		if (!p_output_stream.is_open())
			std::cerr << "Error opening output file " << ouput << "!" << std::endl;
	}

	~BaseTest() {
		p_region_stream.close();
		p_images_stream.close();
		p_output_stream.close();

		delete[] outRects;
		delete[] distances;
		delete[] overlapSpaces;
		delete[] times;
	}

	inline virtual void processInput() { 
		rects.push_back(cv::Rect(0, 0, 0, 0));
	}

	inline cv::Rect getInitRectangle() const { return rects[0]; }

	inline void outputBoundingBox(const cv::Rect & bbox) {
		p_output_stream << bbox.x << ", " << bbox.y << ", " << bbox.width << ", " << bbox.height << std::endl;
	}
	inline void saveBoundingBox(const cv::Rect & bbox) {
		outRects[counter] = bbox;
	}

	inline virtual std::string generateName() {
		char name_[16];
		sprintf(name_, "%d.jpg", counter+1);
		return imageDir + name_;
	}

	inline int getImage(cv::Mat & img) {
		if (!autoImage && (p_images_stream.eof() || !p_images_stream.is_open()))
			return -1;

		std::string line;

		if (autoImage) {
			if (counter >= rects.size()) return -1;
			line = generateName();
		} else
			std::getline(p_images_stream, line);

		img = cv::imread(line, CV_LOAD_IMAGE_COLOR);

		printf("Processing");
		printf(line.c_str());
		printf("\n");

		return 1;
	}

	inline cv::Rect currentRect() {
		return outRects[counter];
	}
	inline cv::Rect currentTruth() {
		return rects[counter];
	}
	inline void next() { counter++; }

	inline double calcDistance() {
		auto rect = currentRect();
		auto truth = currentTruth();

		double dx = rect.x + (rect.width / 2.0);
		double dy = rect.y + (rect.height / 2.0);
		double tx = truth.x + (truth.width / 2.0);
		double ty = truth.y + (truth.height / 2.0);
		auto dist = sqrt((tx - dx)*(tx - dx) + (ty - dy)*(ty - dy));

		return distances[counter] = dist;
	}

	inline double calcOverlapSpace() {
		auto rect = currentRect();
		auto truth = currentTruth();

		auto cross = _calcCrossSpace(rect, truth);
		auto os = cross / _calcMergeSpace(rect, truth, cross);

		return overlapSpaces[counter] = os;
	}

	virtual void drawTruth(cv::Mat frame) {
		cv::Rect truth = currentTruth();
		cv::rectangle(frame, truth, cv::Scalar(255, 0, 0), 2);
	}

	double doTrack(Tracker &tracker, cv::Mat &frame) {
		double start = static_cast<double>(GetTickCount());
		tracker.Track(frame);
		double end = static_cast<double>(GetTickCount());
		return times[counter] = (end - start) / 1000;// getTickFrequency();
	}

	virtual void doTest() {
		osRates.clear();
		distRates.clear();
		long len = rects.size();

		for (double t = 0; t <= 1; t += DeltaTreshold) {
			auto distT = t * MaxDistTreshold;
			auto osT = t * MaxOSTreshold;

			double distRate, osRate;
			long distCnt = 0, osCnt = 0;
			for (int i = 1; i < len; ++i) {
				auto dist = distances[i];
				auto os = overlapSpaces[i];

				if (dist <= distT) distCnt++;
				if (os >= osT) osCnt++;
			}

			if (len > 1) {
				distRate = distCnt * 1.0 / (len - 1);
				osRate = osCnt * 1.0 / (len - 1);
			} else distRate = osRate = 0;

			distRates[distT] = distRate;
			osRates[osT] = osRate;
		}
	}

	virtual void saveResult() {
		OutTable::iterator oit;
		p_output_stream << "Overall" << std::endl;
		p_output_stream << "Frame Distance OverlapSpace Time(s)" << std::endl;
		for (int i = 0; i < rects.size(); ++i) 
			p_output_stream << (i + 1) << " " << distances[i] << " "
				<< overlapSpaces[i] << " " << times[i] << std::endl;

		p_output_stream << "Distance" << std::endl;
		p_output_stream << "Treshold Ratio" << std::endl;
		for (oit = distRates.begin(); oit != distRates.end(); ++oit) {
			auto pair = *oit;
			p_output_stream << pair.first << " " << pair.second << std::endl;
		}

		p_output_stream << "OverlapSpace" << std::endl;
		p_output_stream << "Treshold Ratio" << std::endl;
		for (oit = osRates.begin(); oit != osRates.end(); ++oit) {
			auto pair = *oit;
			p_output_stream << pair.first << " " << pair.second << std::endl;
		}
	}

	typedef std::map<double, double> OutTable;

	bool autoImage = false;
	int counter = 0;

	std::string imageDir;

	std::vector<cv::Rect> rects;

	cv::Rect* outRects;
	double *distances, *overlapSpaces, *times;

	OutTable distRates, osRates;

	std::ifstream p_region_stream;
	std::ifstream p_images_stream;
	std::ofstream p_output_stream;

protected:

	const double DeltaTreshold = 0.01;
	const double MaxDistTreshold = 50;
	const double MaxOSTreshold = 1;

	virtual double _calcCrossSpace(cv::Rect dist, cv::Rect truth) {
		double dx = dist.x, dy = dist.y,
			dw = dist.width, dh = dist.height;
		double tx = truth.x, ty = truth.y,
			tw = truth.width, th = truth.height;

		double minX = max(dx, tx), maxX = min(dx + dw, tx + tw);
		double minY = max(dy, ty), maxY = min(dy + dh, ty + th);

		double width = maxX - minX, height = maxY - minY;
		if (width <= 0 || height <= 0) return 0;
		return width * height;
	}

	virtual double _calcMergeSpace(cv::Rect dist, cv::Rect truth, double cross) {
		return dist.area() + truth.area() - cross;
	}

};

#endif //CPP_VOT_H
