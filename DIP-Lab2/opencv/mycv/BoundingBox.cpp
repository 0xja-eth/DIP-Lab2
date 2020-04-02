#pragma once
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;

namespace mycv {
	using namespace cv;


	typedef			Rect_<float>	BB;
	typedef			BB&				BBRef;
	typedef const	BB&				BBRefC;


	inline float BBOverlap(BBRefC bbA, BBRefC bbB) {
		BB bbAxB = bbA & bbB; // intersection
		float areaAxB = float(bbAxB.area());
		float areaA = float(bbA.area());
		float areaB = float(bbB.area());
		float overlap = areaAxB / (areaA + areaB - areaAxB);
		return overlap;
	}

	/*
	merge rectangles that overlap 0.6
	*/
	inline void BBCluster(const vector<BB>& iBB, vector<BB>& cBB, vector<int>& cCount) {
		const float SPACE_THR = 0.6f;
		cBB.clear(); cCount.clear();
		if (iBB.empty())
			return;

		const int N_BB = iBB.size();

		vector<int> T;
		vector<int>::iterator T_it;
		int nClusters = 1;
		switch (N_BB) {
		case 0: assert(0); break;// this is not possible
		case 1: T.resize(1, 0); break;
		case 2: T.resize(2, 0);
			if (mycv::BBOverlap(iBB[0], iBB[1]) < SPACE_THR) {
				T[1] = 1;
				nClusters = 2;
			}
			break;
		default:
			T = vector<int>(N_BB, 0);
			nClusters = cv::partition(iBB, T, [&](const Rect_<float>& r1, const Rect_<float>& r2) {
				return mycv::BBOverlap(r1, r2) > SPACE_THR;
			});
			break;
		}

		cBB.resize(nClusters);
		cCount.resize(nClusters, 1);
		for (int i = 0; i < nClusters; i++) {
			BB mean_bb;
			float mX = 0.0f, mY = 0.0f, mW = 0.0f, mH = 0.0f;
			int n(0);
			for (int j = 0; j < (int)T.size(); ++j) {
				if (T[j] == i) {
					mX += iBB[j].x; mY += iBB[j].y;
					mW += iBB[j].width; mH += iBB[j].height;
					n++;
				}
			}

			if (n > 0) {
				float nf = static_cast<float>(n);
				cBB[i].x = mX / nf; cBB[i].y = mY / nf;
				cBB[i].width = mW / nf; cBB[i].height = mH / nf;
				cCount[i] = n;
			}
		}
	}

	inline bool BBOut(BBRefC bb, const cv::Size& imsize) {
		bool bOut = ((bb.x < 0.0f) || (bb.y < 0.0f) ||
			(bb.width < 0.0f) || (bb.height < 0.0f) ||
			(bb.x + bb.width > (float)imsize.width) ||
			(bb.y + bb.height > (float)imsize.height));
		return bOut;
	}
};