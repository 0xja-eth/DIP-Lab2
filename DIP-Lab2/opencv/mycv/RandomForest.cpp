#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include "RandomFern.cpp"
#include "Sample.cpp"

namespace mycv {
	using namespace cv;
	using namespace std;

	template<typename TreeTp = RandomFern>
	class RandomForest {
		int				_nTrees;
		int				_nFeatures;
		vector<TreeTp>	_forest;
		int 	getFeatureCount() const { return _nFeatures; }
		int		getTreeCount() const { return _nTrees; }

	public:

		RandomForest(int nTrees, int nFeatures);
		void	train(const vector<Sample>& dataset);
		float	predict(SampleRefC smpl) const;
		float	predict_prob(SampleRefC smpl) const;

	};

	template<typename TreeTp>
	RandomForest<TreeTp>::RandomForest(int nTrees, int nFeatures) {
		_nTrees = nTrees;
		_nFeatures = nFeatures;
		_forest.reserve(nTrees);
		for (int i = 0; i < nTrees; ++i) {
			_forest.push_back(TreeTp(nFeatures));
		}
	}

	template<typename TreeTp>
	inline void RandomForest<TreeTp>::train(const vector<Sample>& dataset) {
		//[todo] implement bootstrap aggregation
		//http://en.wikipedia.org/wiki/Bootstrap_aggregating
		/*
		* from input dataset of size n, generate new m datasets of randomly
		* sampled data from the input dataset;
		*/
		auto& rng = theRNG();
		int nSamples = int(dataset.size());

#pragma omp parallel for
		for (int i = 0; i < _nTrees; ++i) {
			for (int j = 0; j < nSamples; ++j) {
				for (int k = 0; k < 3; ++k) {
					//auto& smpl = dataset[ rng.uniform( 0, nSamples ) ];					
					auto& smpl = dataset[j];
					_forest[i].train(smpl);
				}
			}
		}
	}

	/*
	*This method returns the cumulative result from all the trees in the forest
	*the class that receives the majority of votes.
	*/
	template<typename TreeTp>
	inline float RandomForest<TreeTp>::predict(SampleRefC smpl) const {
		int pVotes(0), nVotes(0);
		for_each(begin(_forest), end(_forest), [&pVotes, &nVotes, &smpl](const TreeTp& tree) {
			auto lbl = tree.predict(smpl);
			if (lbl == 1.0f) {
				pVotes += 1;
			} else {
				nVotes += 1;
			}
		});
		return pVotes > nVotes ? 1.0f : 0.0f;
	}

	/*
	*Returns a number between 0 and 1.
	*This number represents probability or confidence of the sample belonging to the positive class.
	*It is calculated as the proportion of decision trees that classified the sample as positive.
	*/
	template<typename TreeTp>
	inline float RandomForest<TreeTp>::predict_prob(SampleRefC smpl) const {
		int pVotes(0);
		for_each(begin(_forest), end(_forest), [&pVotes, &smpl](const TreeTp& tree) {
			auto lbl = tree.predict(smpl);
			if (lbl == 1.0f) { pVotes += 1; }
		});
		return float(pVotes) / float(_forest.size());
	}

}