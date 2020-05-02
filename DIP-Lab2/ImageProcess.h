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

// Ŀ����ٲ���
class ObjTrackParam : public RectParam {
public:
	// Ŀ������㷨
	enum Algo {
		FERNS, BOOSTING, KCF, TLD,
		MEDIANFLOW, GOTURN, STRUCK
	};

	Algo algo; // Ŀ������㷨

	ObjTrackParam(int x = 0, int y = 0, int w = 0, int h = 0,
		Algo algo = FERNS, Scalar color = Scalar::all(255)) :
		RectParam(x, y, w, h, color), algo(algo) {}
	ObjTrackParam(cv::Rect rect,
		Algo algo = FERNS, Scalar color = Scalar::all(255)) :
		RectParam(x, y, w, h, color), algo(algo) {}
	ObjTrackParam(Algo algo = KCF) : algo(algo) {}

};

// Ŀ������ٲ���
class ObjDetTrackParam : public ObjTrackParam {
public:
	// �Զ���ⷽʽ(Auto Detect Type)
	enum ADType {
		None, Face
	};

	ADType adType; // �Զ���ⷽʽ

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

// ����������
class FeatDetParam : public ProcessParam {
public:
	// ��������㷨 
	enum Algo { 
		SIFT, SURF, ORB 
	};
	// Ұ��ȥ����ʽ(Remove Type)-ɾ������ƥ��
    enum RType {
        NN, TwoNN, Homography
    };
    // ������ƥ�䷽ʽ(Matcher Type)
    //typedef DescriptorMatcher::MatcherType MType;
    enum MType {
        BruteForce, BFL1, FLANN, RANSAC
    };

	Algo algo; // ��������㷨
	RType rType; // ����ƥ��ȥ����ʽ
	MType mType; // ����ƥ�䷽ʽ

	bool isANMSRemove; //Ұ��ȥ������(�Ƿ�ʹ��ANMSҰ��ȥ��)

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

	// ������ȣ�0-1��
	static double progress;

	// ���ƾ���
	static Mat drawRect(const Mat &data, ProcessParam* _param = NULL);

	// Ŀ����
	static Mat doObjDet(const Mat &data, ProcessParam* _param = NULL);
	// Ŀ�����
	static void doObjTrack(const Mat &data1, const Mat &data2,
		Mat &out1, Mat &out2, ProcessParam* _param = NULL);

	// Ŀ����٣�����cv::Rect���ڶ���;��
	static Rect2d doObjTrack(const Mat &data,
		Ptr<cv::Tracker> &tracker, bool &newDet, ObjTrackParam* param = NULL);

	// Ŀ����٣�STRUCK���٣�����cv::Rect���ڶ���;��
	static Rect2d doObjTrack(const Mat &data, ::Tracker &tracker);

	// Ŀ����٣�GOTURN���٣�����cv::Rect���ڶ���;��
	static Rect2d doObjTrack(const Mat &data1, const Mat &data2,
		cv::dnn::Net &tracker, const cv::Rect prevRect);

	// Ŀ���⼰���٣���Ƶ��
	static const int DetDuration; // �����֡��
	static void doObjDetTrack(const Mat* inVideo, long inLen,
		Mat* &outVideo, long &outLen, ProcessParam* _param = NULL);

	// ���ӳ�䣨������⣩
	static Mat doFeatDet(const Mat &data1, const Mat &data2,
		ProcessParam* _param = NULL);

	// ͼ��У��
	static Mat doImgCorr(const Mat &data1, ProcessParam* _param = NULL);

	// ƴ��ͼ��
	static Mat comMatR(const Mat &Matrix1, const Mat &Matrix2,
		ProcessParam* _param = NULL);

	//ORB������⣨��Ƶ��
	static void doVideoFeatDet(const Mat *inVideo, long inLen,
		Mat* &outVideo, long &outLen, ProcessParam* _param = NULL);

	// ���� GOTURN ������
	static cv::dnn::Net createGOTURN();

private:
	// �������
	static const string FaceDetPath; // ģ��·��
	static cv::Rect _faceDet(const Mat &data);

	// FERNS ����
	static void _FERNSTrack(const Mat &data1, const Mat &data2,
		Mat &out1, Mat &out2, ObjDetTrackParam* param = NULL);
	static void _FERNSTrack(const Mat &data1, const Mat &data2,
		Mat &out, ObjDetTrackParam* param = NULL);

	// STRUCK ����
	static void _STRUCKTrack(const Mat &data1, const Mat &data2,
		Mat &out1, Mat &out2, ObjDetTrackParam* param = NULL);
	static cv::Rect _STRUCKTrack(::Tracker &tracker, const Mat &frame);

	// GOTURN ����
	static const string GOTURNPrototxt;
	static const string GOTURNModel;
	static const int InputSize;

	static void _GOTURNTrack(const Mat &data1, const Mat &data2,
		Mat &out1, Mat &out2, ObjDetTrackParam* param = NULL);
	static cv::Rect _GOTURNTrack(cv::dnn::Net &tracker, const Mat &data1,
		const Mat &data2, const cv::Rect prevRect);

	// ʹ�� opencv ����������Ŀ�����
	static void _trackerTrack(const Mat &data1, const Mat &data2,
		Mat &out1, Mat &out2, ObjDetTrackParam* param = NULL);
	static bool _trackerTrack(const Mat &data1, const Mat &data2,
		ObjTrackParam::Algo algo, Rect2d& rect);

	static bool _trackerTrack(Ptr<cv::Tracker> &tracker, bool &newDet,
		const Mat &frame, Mat &out, ObjDetTrackParam* param);
	static bool _trackerTrack(Ptr<cv::Tracker> &tracker, bool &newDet,
		const Mat &frame, ObjTrackParam::Algo algo, Rect2d& rect);

	static Ptr<cv::Tracker> _getTracker(ObjTrackParam::Algo algo);

	/*-- ������� --*/
    static Mat _featureDectect(Ptr<Feature2D> algo,
        const Mat &data1, const Mat &data2,
        FeatDetParam::RType rType, FeatDetParam::MType mType, 
		bool isANMSRemove);

    /*-- ����ƥ�� --*/
    // BruteForceƥ��
    static vector<DMatches> _BFMatch(const InputArray& queryDesc,
        const InputArray& trainDesc, FeatDetParam::RType rType);
    // BruteForce-L1ƥ��
    static vector<DMatches> _BFL1Match(const InputArray& queryDesc,
        const InputArray& trainDesc, FeatDetParam::RType rType);
    // FLANNƥ��
    static vector<DMatches> _FLANNMatch(const InputArray& queryDesc,
        const InputArray& trainDesc, FeatDetParam::RType rType);
    // RANSACƥ��
    static vector<DMatches> _RANSACMatch(
        const InputArray& queryDesc, const InputArray& trainDesc,
        const KeyPoints& queryKeypoints, const KeyPoints& trainKeypoints,
        FeatDetParam::RType rType);
    
    /*-- Ұ��ȥ�� --*/
    // ANMSȥ��Ұ��
    static const int ANMSSaveNum=200; //�������������
    static KeyPoints _ANMSRemove(const KeyPoints& keys);

    /*-- ƥ��ȥ�� --*/
    // NN
    static DMatches _NNRemove(
        const DMatches& matches,
        const Mat& queryDesc, const Mat& trainDesc);
    // NNDR
    static constexpr float TwoNNRatio = 0.8; // ����
    static DMatches _NNDRRemove(
        const vector<DMatches>& matches,
        const Mat& queryDesc, const Mat& trainDesc);
    // Homography
    static const int minNumberMatches = 20; //��Сƥ���������С�ڴ�������ȥ��
    static const float HomographyThreshold; // ��ֵ
    static DMatches _HomographyRemove(const DMatches& matches,
        const KeyPoints& queryKeypoints, const KeyPoints& trainKeypoints);

	/*// �������
	static Mat _featureDectect(Ptr<Feature2D> algo,
		const Mat &data1, const Mat &data2,
		FeatDetParam::RType rType, FeatDetParam::MType mType);

	// 2NNƥ��
	static const float TwoNNRatio; // ����
	static DMatches _2NNMatch(
		const Ptr<DescriptorMatcher> matcher,
		const InputArray& queryDesc, const InputArray& trainDesc);
	// Ĭ��ƥ��
	static DMatches _defaultMatch(
		const Ptr<DescriptorMatcher> matcher,
		const Mat& queryDesc, const Mat& trainDesc);
	*/

	//�����Ż�ͼ��ƴ�ӵĲ���Ȼ
	static void OptimizeSeam(const Mat& img1, Mat& trans, Mat& dst);
	static void CalcCorners(const Mat& H, const Mat& src);
};

