#pragma once

#include <opencv2/imgproc/types_c.h>
#include <opencv2/tracking.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/features2d.hpp>
#include "opencv/mycv.h"
#include "opencv2/calib3d/calib3d.hpp"

#include "Debug.h"

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

// Ŀ������ٲ���
class ObjDetTrackParam : public RectParam {
public:
	// Ŀ������㷨
	enum Algo {
		FERNS, BOOSTING, KCF, TLD,
		MEDIANFLOW
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
	ObjDetTrackParam(Rect rect,
		Algo algo = FERNS, ADType adType = None,
		Scalar color = Scalar::all(255)) :
		RectParam(rect, color), algo(algo), adType(adType) {}
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

	// Ŀ���⼰���٣���Ƶ��
	static const int DetDuration; // �����֡��
	static void doObjDetTrack(const Mat* inVideo, long inLen,
		Mat* &outVideo, long &outLen, ProcessParam* _param = NULL);

	// ���ӳ�䣨������⣩
	static Mat doFeatDet(const Mat &data1, const Mat &data2,
		ProcessParam* _param = NULL);

	// ƴ��ͼ��
	static Mat comMatR(const Mat &Matrix1, const Mat &Matrix2,
		ProcessParam* _param = NULL);

	//ORB������⣨��Ƶ��
	static void doVideoFeatDet(const Mat *inVideo, long inLen,
		Mat* &outVideo, long &outLen, ProcessParam* _param = NULL);

private:
	// �������
	static const string FaceDetPath; // ģ��·��
	static Rect _faceDet(const Mat &data);

	// FERNS ����
	static void _FERNSTrack(const Mat &data1, const Mat &data2,
		Mat &out1, Mat &out2, ObjDetTrackParam* param = NULL);
	static void _FERNSTrack(const Mat &data1, const Mat &data2,
		Mat &out, ObjDetTrackParam* param = NULL);

	// ʹ�� opencv ����������Ŀ�����
	static void _trackerTrack(const Mat &data1, const Mat &data2,
		Mat &out1, Mat &out2, ObjDetTrackParam* param = NULL);
	static bool _trackerTrack(Ptr<Tracker> &tracker, bool &newDet,
		const Mat &frame, Mat &out, ObjDetTrackParam* param);

	static Ptr<Tracker> _getTracker(ObjDetTrackParam::Algo algo);

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

