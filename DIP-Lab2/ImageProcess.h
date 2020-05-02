#pragma once

#include <opencv2/imgproc/types_c.h>
#include <opencv2/tracking.hpp>

#include <opencv2/dnn.hpp>

#include <opencv2/xfeatures2d.hpp>
#include <opencv2/features2d.hpp>
#include "opencv/mycv.h"
#include "opencv2/calib3d/calib3d.hpp"

#include "Debug.h"

#include "lib/STRUCK/Tracker.h"
#include "lib/STRUCK/Config.h"

using namespace cv::dnn;

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
	RectParam(cv::Rect rect, Scalar color = Scalar::all(255)) :
		color(color) { setRect(rect); }

	void setRect(cv::Rect rect) { 
		x = rect.x; y = rect.y;
		w = rect.width; h = rect.height;
	}
	cv::Rect getRect() const {
		return cv::Rect(x, y, w, h);
	}
};

// 目标跟踪参数
class ObjTrackParam : public RectParam {
public:
	// 目标跟踪算法
	enum Algo {
		FERNS, BOOSTING, KCF, TLD,
		MEDIANFLOW, GOTURN, STRUCK
	};

	Algo algo; // 目标跟踪算法

	ObjTrackParam(int x = 0, int y = 0, int w = 0, int h = 0,
		Algo algo = FERNS, Scalar color = Scalar::all(255)) :
		RectParam(x, y, w, h, color), algo(algo) {}
	ObjTrackParam(cv::Rect rect,
		Algo algo = FERNS, Scalar color = Scalar::all(255)) :
		RectParam(x, y, w, h, color), algo(algo) {}
	ObjTrackParam(Algo algo = KCF) : algo(algo) {}

};

// 目标检测跟踪参数
class ObjDetTrackParam : public ObjTrackParam {
public:
	// 自动检测方式(Auto Detect Type)
	enum ADType {
		None, Face
	};

	ADType adType; // 自动检测方式

	ObjDetTrackParam(int x = 0, int y = 0, int w = 0, int h = 0,
		Algo algo = FERNS, ADType adType = None,
		Scalar color = Scalar::all(255)) :
		ObjTrackParam(x, y, w, h, algo, color), adType(adType) {}
	ObjDetTrackParam(cv::Rect rect,
		Algo algo = FERNS, ADType adType = None,
		Scalar color = Scalar::all(255)) :
		ObjTrackParam(rect, algo, color), adType(adType) {}

	ObjDetTrackParam(Algo algo = KCF) : ObjTrackParam(algo) {}
};

// 特征检测参数
class FeatDetParam : public ProcessParam {
public:
	// 特征检测算法 
	enum Algo { 
		SIFT, SURF, ORB 
	};
	// 野点去除方式(Remove Type)-删除错误匹配
    enum RType {
        NN, TwoNN, Homography
    };
    // 特征点匹配方式(Matcher Type)
    //typedef DescriptorMatcher::MatcherType MType;
    enum MType {
        BruteForce, BFL1, FLANN, RANSAC
    };

	Algo algo; // 特征检测算法
	RType rType; // 错误匹配去除方式
	MType mType; // 特征匹配方式

	bool isANMSRemove; //野点去除方法(是否使用ANMS野点去除)

	FeatDetParam(Algo algo = SIFT, RType rType = NN,
        MType mType = BruteForce, bool isANMSRemoveFlag = true) :
        algo(algo), rType(rType), mType(mType) {
		isANMSRemove = isANMSRemoveFlag;
    }
	/*FeatDetParam(Algo algo = SIFT, RType rType = None, 
		MType mType = DescriptorMatcher::BRUTEFORCE) :
		algo(algo), rType(rType), mType(mType) {}*/
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

	// 目标跟踪（返回cv::Rect用于多用途）
	static Rect2d doObjTrack(const Mat &data,
		Ptr<cv::Tracker> &tracker, bool &newDet, ObjTrackParam* param = NULL);

	// 目标跟踪（STRUCK跟踪，返回cv::Rect用于多用途）
	static Rect2d doObjTrack(const Mat &data, ::Tracker &tracker);

	// 目标跟踪（GOTURN跟踪，返回cv::Rect用于多用途）
	static Rect2d doObjTrack(const Mat &data1, const Mat &data2,
		cv::dnn::Net &tracker, const cv::Rect prevRect);

	// 目标检测及跟踪（视频）
	static const int DetDuration; // 检测间隔帧数
	static void doObjDetTrack(const Mat* inVideo, long inLen,
		Mat* &outVideo, long &outLen, ProcessParam* _param = NULL);

	// 点对映射（特征检测）
	static Mat doFeatDet(const Mat &data1, const Mat &data2,
		ProcessParam* _param = NULL);

	// 图形校正
	static Mat doImgCorr(const Mat &data1, ProcessParam* _param = NULL);

	// 拼接图像
	static Mat comMatR(const Mat &Matrix1, const Mat &Matrix2,
		ProcessParam* _param = NULL);

	//ORB特征检测（视频）
	static void doVideoFeatDet(const Mat *inVideo, long inLen,
		Mat* &outVideo, long &outLen, ProcessParam* _param = NULL);

	// 创建 GOTURN 跟踪器
	static cv::dnn::Net createGOTURN();

private:
	// 人脸检测
	static const string FaceDetPath; // 模型路径
	static cv::Rect _faceDet(const Mat &data);

	// FERNS 跟踪
	static void _FERNSTrack(const Mat &data1, const Mat &data2,
		Mat &out1, Mat &out2, ObjDetTrackParam* param = NULL);
	static void _FERNSTrack(const Mat &data1, const Mat &data2,
		Mat &out, ObjDetTrackParam* param = NULL);

	// STRUCK 跟踪
	static void _STRUCKTrack(const Mat &data1, const Mat &data2,
		Mat &out1, Mat &out2, ObjDetTrackParam* param = NULL);
	static cv::Rect _STRUCKTrack(::Tracker &tracker, const Mat &frame);

	// GOTURN 跟踪
	static const string GOTURNPrototxt;
	static const string GOTURNModel;
	static const int InputSize;

	static void _GOTURNTrack(const Mat &data1, const Mat &data2,
		Mat &out1, Mat &out2, ObjDetTrackParam* param = NULL);
	static cv::Rect _GOTURNTrack(cv::dnn::Net &tracker, const Mat &data1,
		const Mat &data2, const cv::Rect prevRect);

	// 使用 opencv 跟踪器进行目标跟踪
	static void _trackerTrack(const Mat &data1, const Mat &data2,
		Mat &out1, Mat &out2, ObjDetTrackParam* param = NULL);
	static bool _trackerTrack(const Mat &data1, const Mat &data2,
		ObjTrackParam::Algo algo, Rect2d& rect);

	static bool _trackerTrack(Ptr<cv::Tracker> &tracker, bool &newDet,
		const Mat &frame, Mat &out, ObjDetTrackParam* param);
	static bool _trackerTrack(Ptr<cv::Tracker> &tracker, bool &newDet,
		const Mat &frame, ObjTrackParam::Algo algo, Rect2d& rect);

	static Ptr<cv::Tracker> _getTracker(ObjTrackParam::Algo algo);

	/*-- 特征检测 --*/
    static Mat _featureDectect(Ptr<Feature2D> algo,
        const Mat &data1, const Mat &data2,
        FeatDetParam::RType rType, FeatDetParam::MType mType, 
		bool isANMSRemove);

    /*-- 特征匹配 --*/
    // BruteForce匹配
    static vector<DMatches> _BFMatch(const InputArray& queryDesc,
        const InputArray& trainDesc, FeatDetParam::RType rType);
    // BruteForce-L1匹配
    static vector<DMatches> _BFL1Match(const InputArray& queryDesc,
        const InputArray& trainDesc, FeatDetParam::RType rType);
    // FLANN匹配
    static vector<DMatches> _FLANNMatch(const InputArray& queryDesc,
        const InputArray& trainDesc, FeatDetParam::RType rType);
    // RANSAC匹配
    static vector<DMatches> _RANSACMatch(
        const InputArray& queryDesc, const InputArray& trainDesc,
        const KeyPoints& queryKeypoints, const KeyPoints& trainKeypoints,
        FeatDetParam::RType rType);
    
    /*-- 野点去除 --*/
    // ANMS去除野点
    static const int ANMSSaveNum=200; //存留特征点点数
    static KeyPoints _ANMSRemove(const KeyPoints& keys);

    /*-- 匹配去除 --*/
    // NN
    static DMatches _NNRemove(
        const DMatches& matches,
        const Mat& queryDesc, const Mat& trainDesc);
    // NNDR
    static constexpr float TwoNNRatio = 0.8; // 比率
    static DMatches _NNDRRemove(
        const vector<DMatches>& matches,
        const Mat& queryDesc, const Mat& trainDesc);
    // Homography
    static const int minNumberMatches = 20; //最小匹配对数，当小于此数后不再去除
    static const float HomographyThreshold; // 阈值
    static DMatches _HomographyRemove(const DMatches& matches,
        const KeyPoints& queryKeypoints, const KeyPoints& trainKeypoints);

	/*// 特征检测
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
	*/

	//用于优化图像拼接的不自然
	static void OptimizeSeam(const Mat& img1, Mat& trans, Mat& dst);
	static void CalcCorners(const Mat& H, const Mat& src);
};

