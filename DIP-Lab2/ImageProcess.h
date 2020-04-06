#pragma once

#include <opencv2/imgproc/types_c.h>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/features2d.hpp>
#include "opencv/mycv.h"
#include "opencv2/calib3d/calib3d.hpp"

// 处理参数父类
class ProcessParam {};

// 随机厥参数
class RectParam : public ProcessParam {
public:
	int x, y, w, h;
	Scalar color;

	RectParam(int x = 0, int y = 0, int w = 0, int h = 0,
		Scalar color = Scalar::all(255)) :
		x(x), y(y), w(w), h(h), color(color) {}
};

// 特征检测参数
class FeatDetParam : public ProcessParam {
public:
	enum Algo { 
		SIFT, SURF, ORB 
	};
	enum RType {
		None, TwoNN
	};
	typedef DescriptorMatcher::MatcherType MType;

	Algo algo; // 特征检测算法
	RType rType; // 特征匹配模式
	MType mType; // 特征匹配模式

	FeatDetParam(Algo algo = SIFT, RType rType = None, 
		MType mType = DescriptorMatcher::BRUTEFORCE) :
		algo(algo), rType(rType), mType(mType) {}
};


static class ImageProcess {
public:
	typedef vector<KeyPoint> KeyPoints;
	typedef vector<DMatch> DMatches;

	static const float TwoNNRatio;

	static Mat drawRect(const Mat &data,
		const ProcessParam* _param = NULL);

	static void doFERNS(const Mat &data1, const Mat &data2,
		Mat &out1, Mat &out2, const ProcessParam* _param = NULL);

	static Mat doFeatDet(const Mat &data1, const Mat &data2,
		const ProcessParam* _param = NULL);

	static Mat comMatR(const Mat &Matrix1, const Mat &Matrix2,
		const ProcessParam* _param = NULL);//按行合并矩阵


private:
	static Mat _featureDectect(Ptr<Feature2D> algo,
		const Mat &data1, const Mat &data2,
		FeatDetParam::RType rType, FeatDetParam::MType mType);

	static DMatches _2NNMatch(
		const Ptr<DescriptorMatcher> matcher,
		const InputArray& queryDesc, const InputArray& trainDesc);
	static DMatches _defaultMatch(
		const Ptr<DescriptorMatcher> matcher,
		const Mat& queryDesc, const Mat& trainDesc);
	//用于优化图像拼接的不自然
	static void OptimizeSeam(const Mat& img1, Mat& trans, Mat& dst);
	static void CalcCorners(const Mat& H, const Mat& src);
};

