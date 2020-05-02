#pragma once
#include <opencv2/opencv.hpp>
#include "HaarFeatureFERNS.cpp"
#include "Sample.cpp"

#define DEBUG_HAAR_FEATURE_EXTRACTOR 1
namespace mycv {
	using namespace cv;

	typedef struct HaarSample_ {
		vector<vector<float> >	data;
		float					label;
		HaarSample_&		setLabel(float lbl) { label = lbl; return (*this); }
	};
	typedef HaarSample_			HaarSample;
	typedef HaarSample&			HaarSampleRef;
	typedef const HaarSample&	HaarSampleRefC;

	class HaarFeaturesExtractor {
		int							_featureCount;
		int							_groupCount;
		vector<vector<HaarFeature> > _feats;
	public:
		typedef HaarSample sample_type;
		HaarFeaturesExtractor(int featureCount, int groupCount);
		sample_type	calc(ImgTRefC img, BBRefC bb) const;

	};


	inline HaarFeaturesExtractor::HaarFeaturesExtractor(int featureCount, int groupCount) {
		_featureCount = featureCount;
		_groupCount = groupCount;
		_feats.resize(groupCount);
		for (size_t i = 0; i < groupCount; ++i) {
			auto& group = _feats[i];
			group.resize(featureCount);
			for (int j = 0; j < featureCount; ++j) {
				group[j] = HaarFeature();
			}
		}

#if (DEBUG_HAAR_FEATURE_EXTRACTOR)
		{
			const int SIZE = 50;
			Mat dbgimg(SIZE, groupCount*SIZE, CV_8UC3);
			dbgimg.setTo(0);
			for (int i = 0; i < groupCount; ++i) {
				auto slot = dbgimg(Rect(i*SIZE, 0, SIZE, SIZE));
				auto& fg = _feats[i];
				for (int j = 0; j < featureCount; ++j) {
					auto& f = fg[j];
					auto& rng = theRNG();
					auto color = CV_RGB(rng.next() % 256, rng.next() % 256, rng.next() % 256);
					rectangle(slot, Point(f.a.x1*SIZE, f.a.y1*SIZE), Point(f.a.x2*SIZE, f.a.y2*SIZE), color, -1);
					rectangle(slot, Point(f.b.x1*SIZE, f.b.y1*SIZE), Point(f.b.x2*SIZE, f.b.y2*SIZE), color, 2);
				}
			}
			imshow("HAAR FEATURES", dbgimg); waitKey(10);

		}
#endif
	}

	inline HaarFeaturesExtractor::sample_type HaarFeaturesExtractor::calc(ImgTRefC img, BBRefC bb) const {
		sample_type sample;
		const size_t nFeats = _feats.size();
		sample.data.clear();
		for (int i = 0; i < _groupCount; ++i) {
			vector<float> feat0(_featureCount);
			for (int j = 0; j < _featureCount; ++j) {
				feat0[j] = _feats[i][j].calc(img, bb);
			}
			sample.data.push_back(move(feat0));
		}
		return sample;
	}

}