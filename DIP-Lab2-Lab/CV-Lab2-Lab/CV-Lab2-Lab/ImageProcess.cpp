//
//  ImageProcess.cpp
//  CV-Lab2-Lab
//
//  Created by 陈以勒 on 2020/4/8.
//  Copyright © 2020 FoBird. All rights reserved.
//

#include "ImageProcess.hpp"

Mat ImageProcess::doFeatDet(const Mat &data1, const Mat &data2, int &resnum,
    ProcessParam* _param /*= NULL*/) {

    if (_param == NULL) return data1;
    auto param = (FeatDetParam*)_param;

    Ptr<Feature2D> algo;
    switch (param->algo) {
    case FeatDetParam::SIFT: algo = xfeatures2d::SIFT::create(); break;
    case FeatDetParam::SURF: algo = xfeatures2d::SURF::create(); break;
    case FeatDetParam::ORB: algo = cv::ORB::create(); break;
    default: return data1;
    }
    return _featureDectect(algo, data1, data2, resnum, param->rType, param->mType, param->isANMSRemove);
}

Mat ImageProcess::_featureDectect(Ptr<Feature2D> algo, const Mat &data1, const Mat &data2, int &resnum, FeatDetParam::RType rType, FeatDetParam::MType mType, bool isANMSRemove) {
    //特征点
    KeyPoints key1, key2;
    //单独提取特征点
    algo->detect(data1, key1);
    algo->detect(data2, key2);

    //画特征点
    Mat keyPointImg1;
    Mat keyPointImg2;
    drawKeypoints(data1, key1, keyPointImg1,
        Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    drawKeypoints(data2, key2, keyPointImg2,
        Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    resnum = key1.size()+key2.size();

    //显示特征点
    //imshow("KeyPoints of img1", keyPointImg1);
    //imshow("KeyPoints of img2", keyPointImg2);
    Mat out;
    hconcat(keyPointImg1, keyPointImg2, out);
    
    //ANMS去除野点
    if(isANMSRemove){
        key1 = _ANMSRemove(key1);
        key2 = _ANMSRemove(key2);
    
        drawKeypoints(data1, key1, keyPointImg1,
            Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        drawKeypoints(data2, key2, keyPointImg2,
            Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        
        resnum = key1.size()+key2.size();
    
        //imshow("KeyPoints of img1 after ANMS", keyPointImg1);
        //imshow("KeyPoints of img2 after ANMS", keyPointImg2);
        Mat out1;
        hconcat(keyPointImg1, keyPointImg2, out1);
        vconcat(out, out1, out);
    }
    
    //特征点匹配
    Mat desc1, desc2;
    //提取特征点并计算特征描述子
    algo->detectAndCompute(data1, Mat(), key1, desc1);
    algo->detectAndCompute(data2, Mat(), key2, desc2);

    //Struct for DMatch: query descriptor index, train descriptor index, train image index and distance between descriptors.
    //int queryIdx –>是测试图像的特征点描述符（descriptor）的下标，同时也是描述符对应特征点（keypoint)的下标。
    //int trainIdx –> 是样本图像的特征点描述符的下标，同样也是相应的特征点的下标。
    //int imgIdx –>当样本是多张图像的话有用。
    //float distance –>代表这一对匹配的特征点描述符（本质是向量）的欧氏距离，数值越小也就说明两个特征点越相像。

    //如果采用flannBased方法 那么desp通过orb的到的类型不同需要先转换类型
    if (desc1.type() != CV_32F || desc2.type() != CV_32F) {
        desc1.convertTo(desc1, CV_32F);
        desc2.convertTo(desc2, CV_32F);
    }
    
    /*-- 特征匹配 --*/
    vector<DMatches> matches;
    switch (mType){
        case FeatDetParam::BruteForce:
            matches = _BFMatch(desc1, desc2, rType);
            resnum = matches[0].size();
            break;
        case FeatDetParam::BFL1:
            matches = _BFL1Match(desc1, desc2, rType);
            resnum = matches[0].size();
            break;
        case FeatDetParam::FLANN:
            matches = _FLANNMatch(desc1, desc2, rType);
            resnum = matches[0].size();
            break;
        case FeatDetParam::RANSAC:
            matches = _RANSACMatch(desc1, desc2, key1, key2, rType);
            resnum = matches[0].size();
            break;
        default:
            return out;
            break;
    }
    
    Mat outMatches;
    drawMatches(data1, key1, data2, key2, matches, outMatches);
    vconcat(out, outMatches, out);
    
    /*-- 去除野点 --*/
    DMatches goodMatches;
    switch (rType) {
        case FeatDetParam::NN:
            goodMatches = _NNRemove(matches[0], desc1, desc2);
            resnum = goodMatches.size();
            break;
        case FeatDetParam::TwoNN:
            if(mType==FeatDetParam::RANSAC) goodMatches=matches[0];
            else goodMatches = _NNDRRemove(matches, desc1, desc2);
            resnum = goodMatches.size();
            break;
        case FeatDetParam::Homography:
            goodMatches = _HomographyRemove(matches[0], key1, key2);
            resnum = goodMatches.size();
            break;
        default:
            return out;
            break;
    }

    Mat imageOutput;
    drawMatches(data1, key1, data2, key2, goodMatches, imageOutput);
    vconcat(out, imageOutput, out);

    return out;
}

/*-- 特征匹配 --*/
vector<ImageProcess::DMatches> ImageProcess::_BFMatch(const InputArray& queryDesc,
    const InputArray& trainDesc, FeatDetParam::RType rType){
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::BRUTEFORCE);
    
    vector<DMatches> matches;
    if(rType == FeatDetParam::TwoNN){
        matcher->knnMatch(queryDesc, trainDesc, matches, 2);
    }else{
        DMatches omatches;
        matcher->match(queryDesc, trainDesc, omatches);
        matches.push_back(omatches);
    }

    return matches;
}

vector<ImageProcess::DMatches> ImageProcess::_BFL1Match(const InputArray& queryDesc, const InputArray& trainDesc, FeatDetParam::RType rType){
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::BRUTEFORCE_L1);
    
    vector<DMatches> matches;
    if(rType == FeatDetParam::TwoNN){
        matcher->knnMatch(queryDesc, trainDesc, matches, 2);
    }else{
        DMatches omatches;
        matcher->match(queryDesc, trainDesc, omatches);
        matches.push_back(omatches);
    }

    return matches;
}

vector<ImageProcess::DMatches> ImageProcess::_FLANNMatch(const InputArray& queryDesc,
    const InputArray& trainDesc, FeatDetParam::RType rType){
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
    
    vector<DMatches> matches;
    if(rType == FeatDetParam::TwoNN){
        matcher->knnMatch(queryDesc, trainDesc, matches, 2);
    }else{
        DMatches omatches;
        matcher->match(queryDesc, trainDesc, omatches);
        matches.push_back(omatches);
    }

    return matches;
}

vector<ImageProcess::DMatches> ImageProcess::_RANSACMatch(
    const InputArray& queryDesc, const InputArray& trainDesc,
    const KeyPoints& queryKeypoints, const KeyPoints& trainKeypoints,
    FeatDetParam::RType rType){
    vector<DMatches> resmatches;
    
    Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::BRUTEFORCE);
    DMatches matches;
    matcher->match(queryDesc, trainDesc, matches);
    
    /*BFMatcher matcher(NORM_HAMMING, true);
    
    DMatches matches;
    matcher.match(queryDesc, trainDesc, matches);*/
    
    //存储删选后的匹配
    DMatches goodMatches = matches;
    
    //如果剩余匹配已经足够少则不再删减
    if(matches.size()<minNumberMatches){
        resmatches.push_back(matches);
        return resmatches;
    }
    
    vector<Point2f> srcPoints(matches.size());
    vector<Point2f> dstPoints(matches.size());
    for (size_t i = 0; i < matches.size(); i++){
        srcPoints[i] = trainKeypoints[matches[i].trainIdx].pt;
        dstPoints[i] = queryKeypoints[matches[i].queryIdx].pt;
    }
    
    vector<unsigned char> inliersMask(srcPoints.size());
    
    Mat homography = findHomography(srcPoints, dstPoints, inliersMask);
    //Mat homography = findHomography(srcPoints, dstPoints, FM_RANSAC);
    
    DMatches inliers;
    for (size_t i = 0; i<inliersMask.size(); i++)
    {
        if (inliersMask[i])
            inliers.push_back(matches[i]);
    }
    goodMatches.swap(inliers);
    
    resmatches.push_back(goodMatches);
    return resmatches;
}

/*-- 野点去除 --*/
ImageProcess::DMatches ImageProcess::_NNRemove(
    const ImageProcess::DMatches& matches,
    const Mat& queryDesc, const Mat& trainDesc){
    //存储筛选后的匹配
    DMatches goodMatches;

    //计算特征点距离的最大值
    double maxDist = 0;
    for (int i = 0; i < queryDesc.rows; i++) {
        double dist = matches[i].distance;
        if (dist > maxDist) maxDist = dist;
    }

    //挑选好的匹配点
    for (int i = 0; i < queryDesc.rows; i++)
        if (matches[i].distance < 0.5*maxDist)
            goodMatches.push_back(matches[i]);

    return goodMatches;
}

ImageProcess::DMatches ImageProcess::_NNDRRemove(
    const vector<ImageProcess::DMatches>& matches,
    const Mat& queryDesc, const Mat& trainDesc){
    //存储筛选后的匹配
    DMatches goodMatches;
    goodMatches.reserve(matches.size());
    
    //一般采用0.8作为阀值 1NN / 2NN < 0.8，这里采取opencv的阈值
    for (size_t i = 0; i < matches.size(); i++) {
        const DMatch& closestMatch = matches[i][0];
        const DMatch& secondClosestMatch = matches[i][1];

        float distanceRatio = closestMatch.distance / secondClosestMatch.distance;

        //只有当比值小于minRatio才认为是好的匹配
        if (distanceRatio < TwoNNRatio)
            goodMatches.push_back(closestMatch);
    }
    return goodMatches;
}

ImageProcess::DMatches ImageProcess::_HomographyRemove(const DMatches& matches,
    const KeyPoints& queryKeypoints, const KeyPoints& trainKeypoints){
    //存储删选后的匹配
    DMatches goodMatches = matches;
    
    //如果剩余匹配已经足够少则不再删减
    if(matches.size()<minNumberMatches) return matches;
    
    vector<Point2f> srcPoints(matches.size());
    vector<Point2f> dstPoints(matches.size());
    for (size_t i = 0; i < matches.size(); i++){
        srcPoints[i] = trainKeypoints[matches[i].trainIdx].pt;
        dstPoints[i] = queryKeypoints[matches[i].queryIdx].pt;
    }
    
    vector<unsigned char> inliersMask(srcPoints.size());
    
    Mat homography = findHomography(srcPoints, dstPoints, inliersMask);
    //Mat homography = findHomography(srcPoints, dstPoints, CV_FM_RANSAC, inliersMask);
    
    DMatches inliers;
    for (size_t i = 0; i<inliersMask.size(); i++)
    {
        if (inliersMask[i])
            inliers.push_back(matches[i]);
    }
    goodMatches.swap(inliers);
    
    return goodMatches;
}

double computeR(Point2i x1, Point2i x2)
{
    return norm(x1 - x2);
}
template < typename T>
vector< size_t>  sort_indexes(const vector< T>  & v) {

    // initialize original index locations
    vector< size_t>  idx(v.size());
    for (size_t i = 0; i != idx.size(); ++i) idx[i] = i;

    // sort indexes based on comparing values in v
    sort(idx.begin(), idx.end(),
        [&v](size_t i1, size_t i2) {return v[i1] >  v[i2]; });

    return idx;
}

ImageProcess::KeyPoints ImageProcess::_ANMSRemove(const KeyPoints& keys){
    int sz = (int)keys.size();
    double maxmum = 0;
    vector<double> roblocalmax(keys.size());
    vector<double> raduis(keys.size(), INFINITY);
    
    for (size_t i = 0; i < sz; i++){
        auto rp = keys[i].response;
        if (rp > maxmum) maxmum = rp;
        roblocalmax[i] = rp*0.9;
    }
    auto max_response = maxmum*0.9;
    for (size_t i = 0; i < sz; i++){
        double rep = keys[i].response;
        Point2i p = keys[i].pt;
        auto& rd = raduis[i];
        if (rep>max_response){
            rd = INFINITY;
        }else{
            for (size_t j = 0; j < sz; j++){
                if (roblocalmax[j] > rep){
                    auto d = computeR(keys[j].pt, p);
                    if (rd > d)rd = d;
                }
            }
        }
    }
    auto sorted = sort_indexes(raduis);
    vector<KeyPoint> rpts;
        
    for (size_t i = 0; i < ANMSSaveNum; i++){
        rpts.push_back(keys[sorted[i]]);
        
    }
    
    return rpts;
}
