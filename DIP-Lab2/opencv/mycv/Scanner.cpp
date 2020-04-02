#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include "BoundingBox.cpp"

#define CV_AA 16

#define DEBUG_SCANNER 1

namespace mycv {
	using namespace cv;
	using namespace std;

	class Scanner {
		vector<BB>	_windows;
		Size		_baseSize;
		Size		_maxSize;
		float		_shiftFactor;
		float		_scaleFactor;
	public:
		Scanner(Size baseSize, Size maxSize, float shiftFactor = 0.25, float scaleFactor = 1.2f);
		const vector<BB>&	getWindows() const;
		const Size&			getBaseSize() const;
		const Size&			getMaxSize() const;
		bool				empty() const;
		void				generatePosAndNeg(BBRefC bb, vector<BB>& posBBs, vector<BB>& negBBs) const;
		float				getMaxOverlap(BBRefC bb, BBRef bb0) const;

	}; // class multi_scale_sliding_window

	inline const	vector<BB>&	Scanner::getWindows()	const { return _windows; }
	inline const	Size&		Scanner::getBaseSize()	const { return _baseSize; }
	inline const	Size&		Scanner::getMaxSize()	const { return _maxSize; }
	inline			bool		Scanner::empty()		const { return _windows.empty(); }

	inline Scanner::Scanner(Size baseSize, Size maxSize, float shiftFactor, float scaleFactor) {

#if DEBUG_SCANNER
		Mat dbgImg(maxSize, CV_8UC3); dbgImg = 0;
		vector<Size2f> dbgWindows;
#endif

		_baseSize = baseSize;
		_maxSize = maxSize;
		_shiftFactor = shiftFactor;
		_scaleFactor = scaleFactor;

		vector<BB> windows;
		int scaleIterationsCountWidth = (int)floor(log(1.0f / float(_baseSize.width)) / log(1.0f / _scaleFactor));
		int scaleIterationsCountHeight = (int)floor(log(1.0f / float(_baseSize.height)) / log(1.0f / _scaleFactor));
		int scaleIterationsCount = min(scaleIterationsCountWidth, scaleIterationsCountHeight);

		for (int scale = 0; scale < scaleIterationsCount; ++scale) {
			float windowWidth = floor(_baseSize.width		* pow(scaleFactor, scale));
			float windowHeight = floor(_baseSize.height	* pow(scaleFactor, scale));
			if (windowWidth >= float(maxSize.width) || windowHeight >= float(maxSize.height))
				break;
			float xStep = max(floor(windowWidth * _shiftFactor), 1.0f);
			float yStep = max(floor(windowHeight * _shiftFactor), 1.0f);

#if DEBUG_SCANNER
			{	dbgWindows.push_back(Size2f(windowWidth, windowHeight)); }
#endif

			for (int y = 0; y < (_maxSize.height - windowHeight); y += yStep) {
				for (int x = 0; x < (_maxSize.width - windowWidth); x += xStep) {
					windows.push_back(BB(x, y, windowWidth, windowHeight));
				}
			}
		}

		_windows = std::move(windows);

#if DEBUG_SCANNER
		{
			auto& rng = theRNG();
			for (int i = 0; i < dbgWindows.size(); ++i) {
				auto sz = dbgWindows[i];
				auto shift = (int(dbgWindows.size()) - i) * 2;
				auto dbgRect = Rect(shift, shift, sz.width, sz.height);
				rectangle(dbgImg, dbgRect, Scalar(rng(256) + 32, rng(256), rng(256)), 1, CV_AA);
			}
			imshow("debug scanner", dbgImg);
			waitKey(5);
		}
#endif
	}

	inline void Scanner::generatePosAndNeg(BBRefC bb, vector<BB>& posBBs, vector<BB>& negBBs) const {
		typedef pair<float, BB> BB2;

		vector<BB2> posBB2;

		vector<BB> posBB0;
		vector<BB> negBB0;

		const vector<BB>& windows = getWindows();
		size_t nWindows = (int)windows.size();

#pragma omp parallel for
		for (int i = 0; i < nWindows; ++i) {
			BBRefC win = windows[i];
			float overlap = BBOverlap(bb, win);
			if (overlap > 0.6f) {
#pragma omp critical
				posBB2.push_back(BB2(overlap, win));
			} else if (overlap < 0.2f) {
#pragma omp critical
				negBB0.push_back(win);
			}
		}

		sort(begin(posBB2), end(posBB2), [](BB2 &a, BB2 &b) {
			return a.first > b.first;
		});

		for_each(begin(posBB2), end(posBB2), [&posBB0](BB2 &bb_) {
			posBB0.push_back(bb_.second);
		});

		posBBs = move(posBB0);
		negBBs = move(negBB0);
	}

	inline float Scanner::getMaxOverlap(BBRefC bb, BBRef bb0) const {
		const vector<BB>& windows = getWindows();
		int nWindows = (int)windows.size();
		float max_overlap = -1.0f;
		for (int i = 0; i < nWindows; ++i) {
			BBRefC win = windows[i];
			float overlap = BBOverlap(bb, win);
			if (overlap > max_overlap) {
				max_overlap = overlap;
				bb0 = win;
			}
		}
		return max_overlap;
	}
}