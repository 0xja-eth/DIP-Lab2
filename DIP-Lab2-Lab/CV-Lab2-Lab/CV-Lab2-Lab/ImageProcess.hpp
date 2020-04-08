//
//  ImageProcess.hpp
//  CV-Lab2-Lab
//
//  Created by 陈以勒 on 2020/4/8.
//  Copyright © 2020 FoBird. All rights reserved.
//

#ifndef ImageProcess_hpp
#define ImageProcess_hpp

#include <stdio.h>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/tracking.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <opencv2/xfeatures2d/nonfree.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include <vector>

using namespace std;
using namespace cv;

// 处理参数父类
class ProcessParam {};

// 特征检测参数
class FeatDetParam : public ProcessParam {
public:
    // 特征检测算法
    enum Algo {
        SIFT, SURF, ORB
    };
    // 野点去除方式(Remove Type)-删除错误匹配
    enum RType {
        NoneR, NN, TwoNN, Homography
    };
    // 特征点匹配方式(Matcher Type)
    //typedef DescriptorMatcher::MatcherType MType;
    enum MType {
        NoneM, BruteForce, BFL1, FLANN, RANSAC
    };

    Algo algo; // 特征检测算法
    bool isANMSRemove; //是否使用ANMS野点去除
    RType rType; // 错误匹配去除方式
    MType mType; // 特征匹配方式

    FeatDetParam(Algo algo = SIFT, RType rType = NoneR,
        MType mType = NoneM, bool isANMSRemoveFlag=false) :
        algo(algo), rType(rType), mType(mType) {
            isANMSRemove = isANMSRemoveFlag;
        }
};

static class ImageProcess{
public:
    typedef vector<KeyPoint> KeyPoints;
    typedef vector<DMatch> DMatches;
    
    // 点对映射（特征检测）
    static Mat doFeatDet(const Mat &data1, const Mat &data2, int &resnum, ProcessParam* _param = NULL);
    
private:
    /*-- 特征检测 --*/
    static Mat _featureDectect(Ptr<Feature2D> algo,
        const Mat &data1, const Mat &data2, int &resnum,
        FeatDetParam::RType rType, FeatDetParam::MType mType, bool isANMSRemove);

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
    static constexpr float TwoNNRatio=0.8; // 比率
    static DMatches _NNDRRemove(
        const vector<DMatches>& matches,
        const Mat& queryDesc, const Mat& trainDesc);
    // Homography
    static const int minNumberMatches=20; //最小匹配对数，当小于此数后不再去除
    static const float HomographyThreshold; // 阈值
    static DMatches _HomographyRemove(const DMatches& matches,
        const KeyPoints& queryKeypoints, const KeyPoints& trainKeypoints);
};

#endif /* ImageProcess_hpp */
