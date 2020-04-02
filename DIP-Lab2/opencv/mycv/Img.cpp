#pragma once
#include <opencv2/opencv.hpp>

namespace mycv {
	using namespace cv;

#if 1

	typedef struct ImgT_ {
		Mat_<uchar>		img;
		Mat_<int>		sum;
		Mat_<double>	sqsum;
		ImgT_();
		ImgT_(const Mat_<uchar>& img_, bool copy = false);
		void set(const Mat_<uchar>& img_, bool copy = false);
		void computeIntegral();
		double getSum(int x1, int y1, int x2, int y2) const;
		double getSquareSum(int x1, int y1, int x2, int y2) const;
		double getArea(int x1, int y1, int x2, int y2) const;
		double getMean(int x1, int y1, int x2, int y2) const;
		double getSqareMean(int x1, int y1, int x2, int y2) const;
		double getVariance(int x1, int y1, int x2, int y2) const;
		double getStdDev(int x1, int y1, int x2, int y2) const;
	};
	typedef ImgT_				ImgT;
	typedef ImgT&				ImgTRef;
	typedef const ImgT&			ImgTRefC;

	inline double ImgT_::getSum(int x1, int y1, int x2, int y2) const {
		const double a = static_cast<double>(sum(y2, x2));
		const double b = static_cast<double>(sum(y2, x1));
		const double c = static_cast<double>(sum(y1, x2));
		const double d = static_cast<double>(sum(y1, x1));
		return a - b - c + d;
	}

	inline double ImgT_::getSquareSum(int x1, int y1, int x2, int y2) const {
		const double a = static_cast<double>(sqsum(y2, x2));
		const double b = static_cast<double>(sqsum(y2, x1));
		const double c = static_cast<double>(sqsum(y1, x2));
		const double d = static_cast<double>(sqsum(y1, x1));
		return a - b - c + d;
	}

	inline double ImgT_::getArea(int x1, int y1, int x2, int y2) const {
		const double area = static_cast<double>((x2 - x1) * (y2 - y1));
		return area;
	}

	inline double ImgT_::getMean(int x1, int y1, int x2, int y2) const {
		const double mean = getSum(x1, y1, x2, y2) / getArea(x1, y1, x2, y2);
		return mean;
	}

	inline double ImgT_::getSqareMean(int x1, int y1, int x2, int y2) const {
		const double sqmean = getSquareSum(x1, y1, x2, y2) / getArea(x1, y1, x2, y2);
		return sqmean;
	}

	inline double ImgT_::getVariance(int x1, int y1, int x2, int y2) const {
		const double mean = getMean(x1, y1, x2, y2);
		const double sqmean = getSqareMean(x1, y1, x2, y2);
		const double variance = sqmean - mean * mean;
		return variance;
	}

	inline double ImgT_::getStdDev(int x1, int y1, int x2, int y2) const {
		const double variance = getVariance(x1, y1, x2, y2);
		return sqrt(variance);
	}

	inline ImgT_::ImgT_() {}

	inline ImgT_::ImgT_(const Mat_<uchar>& img_, bool copy) {
		set(img_, copy);
	}

	inline void ImgT_::set(const Mat_<uchar>& img_, bool copy) {
		assert(img.channels() == 1);
		if (copy) { img_.copyTo(img); } else { img = img_; }
		computeIntegral();
	}

	inline void ImgT_::computeIntegral() {
		integral(img, sum, sqsum, DataType<int>::type);
	}

#else
	typedef Mat_<uchar>			ImgT;
	typedef ImgT&				ImgTRef;
	typedef const ImgT&			ImgTRefC;
	typedef Mat_<int>			SumImgT;
	typedef SumImgT&			SumImgTRef;
	typedef	const SumImgT&		SumImgTRefC;

	inline void computeSumImg(ImgTRefC img, SumImgTRef sum) {
		cv::integral(img, sum, CV_32S);
	}

	inline int computeSum(SumImgTRefC sum, int x1, int y1, int x2, int y2) {
		const int a = sum(y2, x2);
		const int b = sum(y2, x1);
		const int c = sum(y1, x2);
		const int d = sum(y1, x1);
		return a - b - c + d;
	}
#endif
}