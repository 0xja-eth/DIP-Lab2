#pragma once

#include <opencv2/imgproc/types_c.h>
#include <opencv2/xfeatures2d.hpp>

#include "opencv/mycv.h"

// �����������
class ProcessParam {};

// ���β���
class RectParam : public ProcessParam {
public:
	int x, y, w, h;
	Scalar color;

	RectParam(int x = 0, int y = 0, int w = 0, int h = 0,
		Scalar color = Scalar::all(255)) :
		x(x), y(y), w(w), h(h), color(color) {}
};

// Ŀ������ٲ���
class ObjDetTrackParam : public RectParam {
public:
	// Ŀ������㷨
	enum Algo {
		FERNS, BOOSTING, MIL, KCF, TLD,
		MEDIANFLOW, GOTURN, MOSSE
	};
	// �Զ���ⷽʽ(Auto Detect Type)
	enum ADType {
		None, Face
	};

	Algo algo; // Ŀ������㷨
	ADType adType; // �Զ���ⷽʽ

	ObjDetTrackParam(int x = 0, int y = 0, int w = 0, int h = 0,
		Algo algo = FERNS, ADType adType = None,
		Scalar color = Scalar::all(255)) :
		RectParam(x, y, w, h, color), 
		algo(algo), adType(adType) {}
};

// ����������
class FeatDetParam : public ProcessParam {
public:
	// ��������㷨 
	enum Algo { 
		SIFT, SURF, ORB 
	};
	// Ұ��ȥ����ʽ(Remove Type)
	enum RType {
		None, TwoNN
	};
	// ������ƥ�䷽ʽ(Matcher Type)
	typedef DescriptorMatcher::MatcherType MType;

	Algo algo; // ��������㷨
	RType rType; // Ұ��ȥ����ʽ
	MType mType; // ����ƥ�䷽ʽ

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
};

