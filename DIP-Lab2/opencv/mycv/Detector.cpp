#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <vector>	
#include "Scanner.cpp"
#include "Img.cpp"
#include "BoundingBox.cpp"
#include <omp.h>

#include "../lib/PatchGenerator.h"

namespace mycv {
	using namespace cv;
	using namespace std;

	template<class Classifier, class FeatureExtractor>
	class Detector {
		shared_ptr<Scanner>				_scanner;
		shared_ptr<Classifier>			_classifier;
		shared_ptr<FeatureExtractor>	_featureExtractor;
		float							_classifierTresh;
		double							_varianceTresh;
	public:
		Detector(shared_ptr<Scanner> scanner, shared_ptr<Classifier> classifier, shared_ptr<FeatureExtractor> featureExtractor);
		void				detect(ImgTRefC img, vector<BB>& objs, vector<float>& probs) const;
		void				learn(ImgTRefC img, BBRefC bb, bool full = false, vector<BB>& pBB = vector<BB>(), vector<BB>& nBB = vector<BB>());
		double				getVarTresh() const { return _varianceTresh; }

	};

	template<class Classifier, class FeatureExtractor>
	Detector<Classifier, FeatureExtractor>::Detector(shared_ptr<Scanner> scanner, shared_ptr<Classifier> classifier, shared_ptr<FeatureExtractor> featureExtractor)
		: _scanner(scanner), _classifier(classifier), _featureExtractor(featureExtractor) {
		_classifierTresh = 0.5f;
		_varianceTresh = 0.0;
	}


	template<class Classifier, class FeatureExtractor>
	inline void Detector<Classifier, FeatureExtractor>::detect(ImgTRefC img, vector<BB>& objs, vector<float>& probs) const {
		objs.clear(); objs.reserve(100);
		auto windows = _scanner->getWindows();
		size_t windowsCount = windows.size();


#pragma omp parallel for
		for (int i = 0; i < windowsCount; ++i) {
			const auto& bb = windows[i];

			// variance treshold
			const double var = img.getVariance(bb.x, bb.y, bb.x + bb.width, bb.y + bb.height);
			if (var > _varianceTresh) {
				auto& sample = _featureExtractor->calc(img, bb);
				float prob = _classifier->predict_prob(sample);
				// confidance treshold
				if (prob > _classifierTresh) {
#pragma omp critical
					{
						objs.push_back(bb);
						probs.push_back(prob);
					}
				}
			}
		}
	}

	template<class Classifier, class FeatureExtractor>
	inline void Detector<Classifier, FeatureExtractor>::learn(ImgTRefC img, BBRefC bb, bool full, vector<BB>& pBB, vector<BB>& nBB) {

		const int		n_warps = full ? 30 : 20;
		const int		n_num = full ? 100 : 100;
		const double	noise_init = full ? 5.00 : 5.00;
		const double	angle_init = full ? 20.00 : 10.00;
		const double	shift_init = full ? 0.02 : 0.02;
		const double	scale_init = full ? 0.02 : 0.02;

		if (_scanner->empty()) {
			cerr << "detector->learn() failed! [scanner.empty()]" << endl;
			return;
		}

		if (full) {
			// estimate variance treshold
			double var = img.getVariance(bb.x, bb.y, bb.x + bb.width, bb.y + bb.height);
			_varianceTresh = var / 2.0;
			_classifierTresh = 0.5f;
		}

		// reserve: Ô¤Áô¿Õ¼ä
		pBB.reserve(100);
		nBB.reserve(200);
		vector< FeatureExtractor::sample_type > tDataset; tDataset.reserve(300);
		vector< FeatureExtractor::sample_type > vDataset; vDataset.reserve(100);

		const auto& windows = _scanner->getWindows();
		const int nWindows = (int)windows.size();

#pragma omp parallel for
		for (int i = 0; i < nWindows; ++i) {
			const auto& win = windows[i];
			const float overlap = BBOverlap(win, bb);
			// overlap filter
			if (overlap > 0.6f) {
#pragma omp critical
				pBB.push_back(win);
			} else if (overlap < 0.2f) {
				// variance filter
				double var = img.getVariance(win.x, win.y, win.x + win.width, win.y + win.height);
				if ((var > _varianceTresh)) {
#pragma omp critical
					nBB.push_back(win);
				}
			}
		}

		//assert( !pBB.empty() );
		//assert( !nBB.empty() );

		auto patchGen = PatchGenerator(
			0, 0, noise_init, true,
			1.0 - scale_init, 1.0 + scale_init,
			-angle_init * CV_PI / 180, angle_init*CV_PI / 180,
			-angle_init * CV_PI / 180, angle_init*CV_PI / 180);
#if 0
		if (!pBB.empty()) {
			// get maximum overlapping window with the input bb
			sort(begin(pBB), end(pBB), [&bb](BBRefC a, BBRefC b) {
				return BBOverlap(bb, a) < BBOverlap(bb, b);
			});
			auto& pBB0 = *max_element(begin(pBB), end(pBB), [&bb](const BB& a, const BB& b) {
				return BBOverlap(bb, a) > BBOverlap(bb, b);
			});

			// compute bounding box hull
			BB bbH = pBB[0];
			for_each(begin(pBB) + 1, end(pBB), [&bbH](const BB& a) {
				bbH = bbH | a;
			});
		}



		auto& bb0 = bb;
		Size sz = bb0.size();
		Point2f c(bb0.x + (bb0.width*0.5f), bb0.y + (bb0.height*0.5f));
		BB bbPatch(0, 0, sz.width, sz.height);

		int  npbb = pBB.size();
#pragma omp parallel for		
		for (int j = 0; j < npbb; ++j) {
			for (int i = 0; i < n_warps; ++i) {
				auto& r = theRNG();
				ImgT patch;
				if (i > 0)
					patchGen(img.img, c, patch.img, sz, r);
				else
					img.img(bb0).copyTo(patch.img);
				patch.computeIntegral();
				auto sample = _featureExtractor->calc(patch, bbPatch);
				sample.label = 1.0f;
#pragma omp critical					
				tDataset.push_back(sample);
			}
		}
#else

		sort(begin(pBB), end(pBB), [&bb](BBRefC a, BBRefC b) {
			return BBOverlap(bb, a) < BBOverlap(bb, b);
		});

		// compute bounding box hull
		if (pBB.size() > 0) {
			BB bbH = pBB[0];
			int keep = min(10, (int)pBB.size());
			for_each(begin(pBB) + 1, begin(pBB) + keep, [&bbH](const BB& a) {
				bbH = bbH | a;
			});

			auto& bb0 = bbH;
			Size sz = bb0.size();
			Point2f c(bb0.x + (bb0.width*0.5f), bb0.y + (bb0.height*0.5f));
			BB bbPatch(0, 0, sz.width, sz.height);
#pragma omp parallel for
			for (int i = 0; i < n_warps; ++i) {
				auto& r = theRNG();
				ImgT patch;
				if (i > 0)
					patchGen(img.img, c, patch.img, sz, r);
				else
					img.img(bb0).copyTo(patch.img);
				patch.computeIntegral();
				auto sample = _featureExtractor->calc(patch, bbPatch);
				sample.label = 1.0f;
#pragma omp critical					
				tDataset.push_back(sample);
			}
		}
#endif

		random_shuffle(begin(nBB), end(nBB));
		nBB.resize(n_num);

		int nNeg = (int)nBB.size();


#pragma omp parallel for
		for (int i = 0; i < nNeg / 2; ++i) {
			auto sample = _featureExtractor->calc(img, nBB[i]);
			sample.label = 0.0f;
#pragma omp critical					
			tDataset.push_back(sample);
		}

		_classifier->train(tDataset);

	}


}