#pragma once
//#include <opencv2/opencv.hpp>
#include <vector>
#include "Sample.cpp"

namespace mycv {
	//using namespace cv;
	using namespace std;

	class RandomFern {
		int					_nFeatures;
		int					_nNodes;
		vector<int>			_negCount;
		vector<int>			_posCount;
		int		getLeaf(SampleRefC smpl) const;

	public:
		RandomFern(int nFeatures);
		void	train(SampleRefC smpl);
		float	predict(SampleRefC smpl) const;
	};

	inline RandomFern::RandomFern(int nFeatures) {
		_nFeatures = nFeatures;
		_nNodes = 1 << _nFeatures; // 2 ^ nTests;
		_negCount.resize(_nNodes, 0);
		_posCount.resize(_nNodes, 0);
	}

	inline int RandomFern::getLeaf(SampleRefC smpl) const {
		int leaf = 0;
		for (int i = 0; i < _nFeatures; ++i)
			// Ïàµ±ÓÚ leaf %= (int(smpl.data[i] > 0.0f))*2^i
			leaf = leaf | int(smpl.data[i] > 0.0f) << i;
		return leaf;
	}

	inline float RandomFern::predict(SampleRefC smpl) const {
		const int leaf = getLeaf(smpl);
		const int pVotes = _posCount[leaf];
		const int nVotes = _negCount[leaf];
		return pVotes > nVotes ? 1.0f : 0.0f;
	}

	inline void RandomFern::train(SampleRefC smpl) {
		const int leaf = getLeaf(smpl);
		if (smpl.label == 1.0f) {
			_posCount[leaf] += 1;
		} else {
			_negCount[leaf] += 1;
		}
	}
}