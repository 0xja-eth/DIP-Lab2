#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include "HaarFeaturesExtractor.cpp"

namespace mycv {
	using namespace cv;
	using namespace std;

	template<typename TpTree>
	class ForestClassifier {
		int				_nTrees;
		int				_nFeatures;
		vector<TpTree>	_forest;

		struct {
			float	factor;
			int		count;
		} _bagging;

	public:

		ForestClassifier(int nTrees, int nFeatures, int baggingCount = 1, float baggingFactor = 1.0f);
		void	train(const vector<HaarSample>& samples);
		float	predict(HaarSampleRef sample) const;
		float	predict_prob(HaarSampleRef sample) const;
		int 	getFeatureCount() const { return _nFeatures; }
		int		getTreeCount() const { return _nTrees; }
	};

	template<typename TpTree>
	ForestClassifier<TpTree>::ForestClassifier(int nTrees, int nFeatures, int baggingCount, float baggingFactor) {
		_bagging.count = baggingCount;
		_bagging.factor = baggingFactor;
		_nTrees = nTrees;
		_nFeatures = nFeatures;
		_forest.reserve(nTrees);
		for (int i = 0; i < nTrees; ++i)
			_forest.push_back(TpTree(nFeatures));
	}

	template<typename TpTree>
	inline void ForestClassifier<TpTree>::train(const vector<HaarSample>& samples) {
		//[todo] implement bootstrap aggregation
		//http://en.wikipedia.org/wiki/Bootstrap_aggregating
		/*
		* from input dataset of size n, generate new m datasets of randomly
		* sampled data from the input dataset;
		*/
		auto& rng = theRNG();
		auto sampleCount = (int)samples.size();
		auto baggingCount = _bagging.count;
		auto baggingFactor = _bagging.factor;
		auto bagCount = int(float(sampleCount)*baggingFactor);

		vector<int> idx(sampleCount);
		for (int i = 0; i < sampleCount; ++i) {
			idx[i] = i;
		}
		for (size_t i = 0; i < baggingCount; ++i) {
			random_shuffle(begin(idx), end(idx));

#pragma parallel for
			for (int i = 0; i < _nTrees; ++i) {
				for (int j = 0; j < bagCount; ++j) {
					auto& data = samples[idx[j]].data[i];
					auto& tree = _forest[i];
					auto& label = samples[idx[j]].label;
					tree.train(data, label);
				}
			}

		}



	}

	template<typename TpTree>
	inline float ForestClassifier<TpTree>::predict(HaarSampleRef sample) const {
		float avgP = 0.0f;
		for (int i = 0; i < _nTrees; ++i)
			avgP += _forest[i].predict(sample.data[i]);
		avgP /= float(_nTrees);
		return avgP > 0.5f ? 1.0f : 0.0f;
	}

	template<typename TpTree>
	inline float ForestClassifier<TpTree>::predict_prob(HaarSampleRef sample) const {
		float avgP = 0.0f;
		for (int i = 0; i < _nTrees; ++i)
			avgP += _forest[i].predict(sample.data[i]);
		avgP /= float(_nTrees);
		return avgP;
	}

}