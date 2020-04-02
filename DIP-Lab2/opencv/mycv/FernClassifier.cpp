#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include "Sample.cpp"

namespace mycv {
	using namespace cv;
	using namespace std;

	class FernClassifier {
		int					_nFeatures;
		int					_nNodes;
		vector<int>			_totalCount;
		vector<int>			_postiveCount;
		vector<float>		_posteriorProbs;
		int		getLeaf(const vector<float>& sample) const;

	public:
		FernClassifier(int nFeatures);
		void	train(const vector<float>& sample, float label);
		float	predict(const vector<float>& sample) const;
	};

	inline FernClassifier::FernClassifier(int nFeatures) {
		_nFeatures = nFeatures;
		_nNodes = 1 << _nFeatures; // 2 ^ nTests;
		_totalCount.resize(_nNodes, 0);
		_postiveCount.resize(_nNodes, 0);
		_posteriorProbs.resize(_nNodes, .0f);
	}

	inline int FernClassifier::getLeaf(const vector<float>& sample) const {
		int leaf = 0;
		for (int i = 0; i < _nFeatures; ++i)
			leaf = leaf | int(sample[i] > 0.0f) << i;
		return leaf;
	}

	inline float FernClassifier::predict(const vector<float>& sample) const {
		return _posteriorProbs[getLeaf(sample)];
	}

	inline void FernClassifier::train(const vector<float>& sample, float label) {
		int leaf = getLeaf(sample);
		if (label == 1.0f)
			_postiveCount[leaf] += 1;
		_totalCount[leaf] += 1;
		_posteriorProbs[leaf] = float(_postiveCount[leaf]) / float(_totalCount[leaf]);
	}
}