#pragma once

#include <opencv2/imgproc/types_c.h>
#include <opencv2/tracking.hpp>
#include <opencv2/xfeatures2d.hpp>

#include "opencv/mycv.h"

#include "Debug.h"

// 处理参数父类
class ProcessParam {};

// 矩形参数
class RectParam : public ProcessParam {
public:
	int x, y, w, h;
	Scalar color;

	RectParam(int x = 0, int y = 0, int w = 0, int h = 0,
		Scalar color = Scalar::all(255)) :
		x(x), y(y), w(w), h(h), color(color) {}
	RectParam(Rect rect, Scalar color = Scalar::all(255)) :
		color(color) { setRect(rect); }

	void setRect(Rect rect) { 
		x = rect.x; y = rect.y;
		w = rect.width; h = rect.height;
	}
	Rect getRect() const {
		return Rect(x, y, w, h);
	}
};

// 目标检测跟踪参数
class ObjDetTrackParam : public RectParam {
public:
	// 目标跟踪算法
	enum Algo {
		FERNS, BOOSTING, KCF, TLD,
		MEDIANFLOW
	};
	// 自动检测方式(Auto Detect Type)
	enum ADType {
		None, Face
	};

	Algo algo; // 目标跟踪算法
	ADType adType; // 自动检测方式

	ObjDetTrackParam(int x = 0, int y = 0, int w = 0, int h = 0,
		Algo algo = FERNS, ADType adType = None,
		Scalar color = Scalar::all(255)) :
		RectParam(x, y, w, h, color),
		algo(algo), adType(adType) {}
	ObjDetTrackParam(Rect rect,
		Algo algo = FERNS, ADType adType = None,
		Scalar color = Scalar::all(255)) :
		RectParam(rect, color), algo(algo), adType(adType) {}
};

// 特征检测参数
class FeatDetParam : public ProcessParam {
public:
	// 特征检测算法 
	enum Algo { 
		SIFT, SURF, ORB 
	};
	// 野点去除方式(Remove Type)
	enum RType {
		None, TwoNN
	};
	// 特征点匹配方式(Matcher Type)
	typedef DescriptorMatcher::MatcherType MType;

	Algo algo; // 特征检测算法
	RType rType; // 野点去除方式
	MType mType; // 特征匹配方式

	FeatDetParam(Algo algo = SIFT, RType rType = None, 
		MType mType = DescriptorMatcher::BRUTEFORCE) :
		algo(algo), rType(rType), mType(mType) {}
};

static class ImageProcess {
public:
	typedef vector<KeyPoint> KeyPoints;
	typedef vector<DMatch> DMatches;

	// 处理进度（0-1）
	static double progress;

	// 绘制矩形
	static Mat drawRect(const Mat &data, ProcessParam* _param = NULL);

	// 目标检测
	static Mat doObjDet(const Mat &data, ProcessParam* _param = NULL);
	// 目标跟踪
	static void doObjTrack(const Mat &data1, const Mat &data2,
		Mat &out1, Mat &out2, ProcessParam* _param = NULL);

	// 目标检测及跟踪（视频）
	static const int DetDuration; // 检测间隔帧数
	static void doObjDetTrack(const Mat* inVideo, long inLen,
		Mat* &outVideo, long &outLen, ProcessParam* _param = NULL);

	// 点对映射（特征检测）
	static Mat doFeatDet(const Mat &data1, const Mat &data2,
		ProcessParam* _param = NULL);

	//ORB特征检测（视频）
	static void doVideoFeatDet(const Mat *inVideo, long inLen,
		Mat* outVideo, long &outLen, ProcessParam* _param = NULL);

private:
	// 人脸检测
	static const string FaceDetPath; // 模型路径
	static Rect _faceDet(const Mat &data);

	// FERNS 跟踪
	static void _FERNSTrack(const Mat &data1, const Mat &data2,
		Mat &out1, Mat &out2, ObjDetTrackParam* param = NULL);
	static void _FERNSTrack(const Mat &data1, const Mat &data2,
		Mat &out, ObjDetTrackParam* param = NULL);

	// 使用 opencv 跟踪器进行目标跟踪
	static void _trackerTrack(const Mat &data1, const Mat &data2,
		Mat &out1, Mat &out2, ObjDetTrackParam* param = NULL);
	static bool _trackerTrack(Ptr<Tracker> &tracker, bool &newDet,
		const Mat &frame, Mat &out, ObjDetTrackParam* param);

	static Ptr<Tracker> _getTracker(ObjDetTrackParam::Algo algo);

	// 特征检测
	static Mat _featureDectect(Ptr<Feature2D> algo,
		const Mat &data1, const Mat &data2,
		FeatDetParam::RType rType, FeatDetParam::MType mType);

	// 2NN匹配
	static const float TwoNNRatio; // 比率
	static DMatches _2NNMatch(
		const Ptr<DescriptorMatcher> matcher,
		const InputArray& queryDesc, const InputArray& trainDesc);
	// 默认匹配
	static DMatches _defaultMatch(
		const Ptr<DescriptorMatcher> matcher,
		const Mat& queryDesc, const Mat& trainDesc);
};

