#pragma once
#include <opencv2/opencv.hpp>
#include "Img.cpp"
#include "BoundingBox.cpp"

namespace mycv {
	using namespace cv;

	class HaarFeature {
	public:
		struct { float x1, y1, x2, y2; } a, b;
		int adjustCoord(float ratio, int lenght, int shift) const;

		HaarFeature();
		float	calc(ImgTRefC integral, BBRefC bb) const;
	};

	inline int HaarFeature::adjustCoord(float ratio, int lenght, int shift) const {
		return shift + int(lenght * ratio);
	}

	inline HaarFeature::HaarFeature() {
		auto& rng = theRNG();
		float min(0.1f);
		float max(0.5f);

		if (rng.next() % 2) {
			// horisontal feature
			/*
			*	|a|b|
			*/
			float w = (max - min)	* (float)rng + min;
			float h = (max - min)	* (float)rng + min;
			float x = (1.0f - w)	* (float)rng;
			float y = (1.0f - h)	* (float)rng;

			a.x1 = x; a.y1 = y;
			a.x2 = a.x1 + w * 0.5f;
			a.y2 = a.y1 + h;

			b.x1 = a.x2; b.y1 = y;
			b.x2 = b.x1 + w * 0.5f;
			b.y2 = b.y1 + h;
		} else {
			// vertical feature
			/*
			*	|a|
			*	---
			*	|b|
			*/
			float w = (max - min)	* (float)rng + min;
			float h = (max - min)	* (float)rng + min;
			float x = (1.0f - w)	* (float)rng;
			float y = (1.0f - h)	* (float)rng;

			a.x1 = x; a.y1 = y;
			a.x2 = a.x1 + w;
			a.y2 = a.y1 + h * 0.5f;

			b.x1 = x; b.y1 = a.y2;
			b.x2 = b.x1 + w;
			b.y2 = b.y1 + h * 0.5f;
		}

		if (rng.next() % 2 == 1) {
			auto tmp = a;
			a = b; b = tmp;
		}
	}

	inline float HaarFeature::calc(ImgTRefC img, BBRefC bb) const {
		int sumA = img.getSum(
			adjustCoord(a.x1, bb.width, bb.x),
			adjustCoord(a.y1, bb.height, bb.y),
			adjustCoord(a.x2, bb.width, bb.x),
			adjustCoord(a.y2, bb.height, bb.y));

		int sumB = img.getSum(
			adjustCoord(b.x1, bb.width, bb.x),
			adjustCoord(b.y1, bb.height, bb.y),
			adjustCoord(b.x2, bb.width, bb.x),
			adjustCoord(b.y2, bb.height, bb.y));

		return float(sumA - sumB);
	}



}