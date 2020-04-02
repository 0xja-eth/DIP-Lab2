#pragma once
#include <opencv2/opencv.hpp>
#include "RandomHaarFeature.cpp"
#include "Sample.cpp"

namespace mycv {
	using namespace cv;

	class RandomHaarFeaturesExtractor {
		int							_featureCount;
		vector<RandomHaarFeature>	_feats;
	public:
		typedef Sample sample_type;
		RandomHaarFeaturesExtractor(int featureCount);
		sample_type	calc(ImgTRefC img, BBRefC bb) const;
	};

	inline RandomHaarFeaturesExtractor::RandomHaarFeaturesExtractor(int featureCount) {
		_featureCount = featureCount;
		_feats.resize(featureCount);
		for_each(begin(_feats), end(_feats), [](RandomHaarFeature& f) {
			f = RandomHaarFeature();
		});
	}

	inline RandomHaarFeaturesExtractor::sample_type RandomHaarFeaturesExtractor::calc(ImgTRefC img, BBRefC bb) const {
		sample_type smpl;
		const int nFeats = int(_feats.size());
		smpl.data.resize(nFeats);
		for (int i = 0; i < nFeats; ++i) {
			smpl.data[i] = _feats[i].calc(img, bb);
		}
		return smpl;
	}

}