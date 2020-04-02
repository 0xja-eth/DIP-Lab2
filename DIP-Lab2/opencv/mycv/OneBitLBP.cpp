#pragma once
#include <opencv2/opencv.hpp>
#include "Img.cpp"
#include "BoundingBox.cpp"

namespace mycv {
	using namespace cv;

	class OneBitLBP {
		struct { float x1, y1, x2, y2; } f;
		int adjustCoord(float ratio, int lenght, int shift) const;
	public:
		OneBitLBP();
		float	calc(ImgTRefC integral, BBRefC bb) const;
	};

	inline int OneBitLBP::adjustCoord(float ratio, int lenght, int shift) const {
		return shift + int(lenght * ratio);
	}

	inline OneBitLBP::OneBitLBP() {
		auto& rng = theRNG();
		float min(0.1f);
		float max(0.5f);

		f.x1 = (max - min)	* (float)rng + min;
		f.y1 = (max - min)	* (float)rng + min;
		f.x2 = 1.0f - ((max - min)	* (float)rng + min);
		f.y2 = 1.0f - ((max - min)	* (float)rng + min);
	}

	inline float OneBitLBP::calc(ImgTRefC img, BBRefC bb) const {
		int A = img.img(
			adjustCoord(f.x1, bb.width, bb.x),
			adjustCoord(f.y1, bb.height, bb.y));

		int B = img.img(
			adjustCoord(f.x2, bb.width, bb.x),
			adjustCoord(f.y2, bb.height, bb.y));

		return float(A - B);
	}

}