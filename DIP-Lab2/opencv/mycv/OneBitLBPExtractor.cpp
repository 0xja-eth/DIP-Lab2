#pragma once
#include <opencv2/opencv.hpp>
#include "OneBitLBP.cpp"
#include "Sample.cpp"


namespace mycv {
	using namespace cv;

	typedef struct OneBitLBPSample_ {
		vector<vector<float> >	data;
		float					label;
	};
	typedef OneBitLBPSample_			OneBitLBPSampl;
	typedef OneBitLBPSample_&			OneBitLBPSamplRef;
	typedef const OneBitLBPSample_&		OneBitLBPSamplRefC;

	class OneBitLBPExtractor {
		int							_featureCount;
		int							_groupCount;
		vector<vector<OneBitLBP> >	_feats;
	public:
		typedef OneBitLBPSampl sample_type;
		OneBitLBPExtractor(int featureCount, int groupCount);
		sample_type	calc(ImgTRefC img, BBRefC bb) const;

	};

	inline OneBitLBPExtractor::OneBitLBPExtractor(int featureCount, int groupCount) {
		_featureCount = featureCount;
		_groupCount = groupCount;
		_feats.resize(groupCount);
		for (size_t i = 0; i < groupCount; ++i) {
			auto& group = _feats[i];
			group.resize(featureCount);
			for (int j = 0; j < featureCount; ++j) {
				group[j] = OneBitLBP();
			}
		}
	}

	inline OneBitLBPExtractor::sample_type OneBitLBPExtractor::calc(ImgTRefC img, BBRefC bb) const {
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