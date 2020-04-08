
#include "ImageProcess.h"
#include <qdebug.h>

//const float ImageProcess::TwoNNRatio = 1.0 / 1.5;

//图像拼接用到的四个角
typedef struct
{
	Point2f left_top;
	Point2f left_bottom;
	Point2f right_top;
	Point2f right_bottom;
}four_corners_t;

double ImageProcess::progress = 0;
four_corners_t corners;

Mat ImageProcess::drawRect(const Mat &data, ProcessParam* _param /*= NULL*/) {
	if (_param == NULL) return data;
	auto param = (RectParam*)_param;

	Mat out = data.clone();
	Rect rect = param->getRect();
	
	if (!rect.empty()) rectangle(out, rect, param->color);

	return out;
}

Mat ImageProcess::doObjDet(const Mat &data, ProcessParam* _param /*= NULL*/) {
	if (_param == NULL) return data;
	auto param = (ObjDetTrackParam*)_param;

	Rect rect = param->getRect();

	switch (param->adType) {
	case ObjDetTrackParam::Face:
		rect = _faceDet(data); break;
	}

	param->setRect(rect);

	return drawRect(data, param);
}

void ImageProcess::doObjTrack(const Mat &data1, const Mat &data2,
	Mat &out1, Mat &out2, ProcessParam* _param /*= NULL*/) {
	out1 = data1.clone(); out2 = data2.clone();

	if (_param == NULL) return;
	auto param = (ObjDetTrackParam*)_param;

	switch (param->algo) {
	case ObjDetTrackParam::FERNS: 
		_FERNSTrack(data1, data2, out1, out2, param); break;
	default: 
		_trackerTrack(data1, data2, out1, out2, param); break;
	}
}

const int ImageProcess::DetDuration = 60;

void ImageProcess::doObjDetTrack(const Mat* inVideo, long inLen, 
	Mat* &outVideo, long &outLen, ProcessParam* _param /*= NULL*/) {

	if (_param == NULL) return;

	outVideo = new Mat[outLen = inLen];

	auto param = (ObjDetTrackParam*)_param;

	bool trackSucc = false, newDet = false;
	bool auto_ = param->adType != ObjDetTrackParam::None; // 是否自动监测
	int duration = 0;

	Ptr<Tracker> tracker = NULL;

	// 每帧处理
	for (int i = 0; i < inLen; ++i, ++duration) {
		progress = i * 1.0 / inLen;

		Mat frame = inVideo[i];
		Mat &outFrame = outVideo[i];
		outFrame = frame.clone();

		// 如果为第一帧或者需要自动检测并且上一帧没有跟踪到或者间隔帧数大于指定数目
		// 当没有检测算法时，第一帧也要进行一次 doObjDet 操作，用于绘制矩形
		if (i == 0 || (auto_ && (trackSucc == false || duration >= DetDuration))) {
			duration = 0;
			outFrame = doObjDet(frame, param);
			newDet = !param->getRect().empty();
		}

		if (i == 0) continue; // 如果是第一帧，不进行跟踪

		Mat lastFrame = inVideo[i - 1];

		switch (param->algo) {
		case ObjDetTrackParam::FERNS:
			trackSucc = false;
			/*
			_FERNSTrack(lastFrame, frame, outFrame, param);
			trackSucc = !param->getRect().empty();
			*/
			break;
		default:
			trackSucc = _trackerTrack(tracker, newDet, 
				frame, outFrame, param);
			break;
		}
		if (trackSucc) LOG("第 " << i << " 帧跟踪成功！");
		else LOG("第 " << i << " 帧跟踪失败！");

		if (!outFrame.empty()) imshow("video", outFrame);
	}
}

Mat ImageProcess::doFeatDet(const Mat &data1, const Mat &data2, 
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
	return _featureDectect(algo, data1, data2, param->rType, 
		param->mType, param->isANMSRemove);
}

void ImageProcess::doVideoFeatDet(const Mat *inVideo, long inLen,
	Mat* &outVideo, long &outLen, ProcessParam* _param) {

	outVideo = new Mat[outLen = inLen];
	int duration = 0;

	// 每帧处理
	for (int i = 0; i < inLen; ++i) {
		progress = i * 1.0 / inLen;

		Mat frame = inVideo[i];
		Mat &outFrame = outVideo[i];
		outFrame = frame.clone();

		//初始化
		vector<KeyPoint> keypoints;
		Mat descriptors;
		Ptr<FeatureDetector> detector = cv::ORB::create();
		Ptr<DescriptorExtractor> descriptor = cv::ORB::create();

		//检测 Oriented FAST 角点位置
		detector->detect(frame, keypoints);

		//根据角点位置计算 BRIEF 描述子
		descriptor->compute(frame, keypoints, descriptors);

		//绘制特征点
		drawKeypoints(frame, keypoints, outFrame, Scalar::all(-1), DrawMatchesFlags::DEFAULT);

		if (!outFrame.empty()) imshow("ORB_Video", outFrame);
		//waitKey(duration);
	}
}

const std::string ImageProcess::FaceDetPath = "./xml/haarcascade_frontalface_alt.xml";

Rect ImageProcess::_faceDet(const Mat &data) {
	// 加载
	static CascadeClassifier cascade;
	if (cascade.empty()) 
		// 加载训练好的 人脸检测器（.xml）
		if (!cascade.load(FaceDetPath)) LOG("人脸检测器加载失败");

	if (cascade.empty()) return Rect(0, 0, 0, 0);
	
	vector<Rect> faces(0);
	cascade.detectMultiScale(data, faces, 1.1, 2, 0, Size(30, 30));

	if (faces.size() > 0) {
		LOG("检测到 " << faces.size() << "个人脸，返回第一个");
		return faces[0];
	}
	
	cout << "未检测到人脸" << endl;
	return Rect(0, 0, 0, 0);
}

void ImageProcess::_FERNSTrack(const Mat &data1, const Mat &data2, 
	Mat &out1, Mat &out2, ObjDetTrackParam* param /*= NULL*/) {

	Rect tmplBB = param->getRect();
	rectangle(out1, tmplBB, param->color);

	_FERNSTrack(data1, data2, out2, param);
}

void ImageProcess::_FERNSTrack(const Mat &data1, const Mat &data2, Mat &out, 
	ObjDetTrackParam* param /*= NULL*/) {
	Mat tmp = data1.clone();
	
	cvtColor(tmp, tmp, COLOR_BGR2GRAY);
	cvtColor(out, out, COLOR_BGR2GRAY);

	auto tmplImg1 = ImgT(tmp), tmplImg2 = ImgT(out);
	BB tmplBB = param->getRect();

	auto featuresExtractor = make_shared<HaarFeaturesExtractor>(5, 10);
	auto classifier = make_shared<ForestClassifier<FernClassifier> >(10, 5);
	auto scanner = make_shared<Scanner>(Size(24, 24), Size(320, 240), 0.25, 1.2);
	auto detector = mycv::Detector<ForestClassifier<FernClassifier>,
		HaarFeaturesExtractor>(scanner, classifier, featuresExtractor);

	detector.learn(tmplImg1, tmplBB, true);

	vector<BB> objs;
	vector<float> probs;

	detector.detect(tmplImg2, objs, probs);

	BB avgRect; // 若有多个匹配，求均值
	for_each(begin(objs), end(objs), [&out, &param, &avgRect](BBRefC obj) {
		rectangle(out, obj, param->color);
		avgRect.x += obj.x; avgRect.y += obj.y;
		avgRect.width += obj.width; 
		avgRect.height += obj.height;
	});
	 
	avgRect.x /= objs.size();
	avgRect.y /= objs.size();
	avgRect.width /= objs.size();
	avgRect.height /= objs.size();

	param->setRect(avgRect);
}

void ImageProcess::_trackerTrack(const Mat &data1, const Mat &data2, Mat &out1, Mat &out2, 
	ObjDetTrackParam* param /*= NULL*/) {
	auto tracker = _getTracker(param->algo);
	
	Rect2d rect = param->getRect();

	rectangle(out1, rect, param->color);
	
	tracker->init(data1, rect);
	bool ok = tracker->update(data2, rect);

	if (ok) rectangle(out2, rect, param->color);
	else LOG("跟踪失败！");
}

bool ImageProcess::_trackerTrack(Ptr<Tracker> &tracker, bool &newDet,
	const Mat &frame, Mat &out, ObjDetTrackParam* param) {
	Rect2d rect = param->getRect();

	if (newDet) { // 重新加载跟踪器
		tracker = _getTracker(param->algo);
		tracker->init(frame, rect);
		newDet = false;
	}

	bool succ = (!tracker.empty() && tracker->update(frame, rect));
	if (succ) rectangle(out, rect, param->color);

	param->setRect(rect);
	return succ;
}

Ptr<Tracker> ImageProcess::_getTracker(ObjDetTrackParam::Algo algo) {
	switch (algo) {
	case ObjDetTrackParam::BOOSTING: return TrackerBoosting::create();
	//case ObjDetTrackParam::MIL: return TrackerMIL::create();
	case ObjDetTrackParam::KCF: return TrackerKCF::create();
	case ObjDetTrackParam::TLD: return TrackerTLD::create();
	case ObjDetTrackParam::MEDIANFLOW: return TrackerMedianFlow::create();
	//case ObjDetTrackParam::GOTURN: return TrackerGOTURN::create();
	//case ObjDetTrackParam::MOSSE: return TrackerMOSSE::create();
	}
}

Mat ImageProcess::_featureDectect(Ptr<Feature2D> algo, const Mat &data1, const Mat &data2, FeatDetParam::RType rType, FeatDetParam::MType mType, bool isANMSRemove) {
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

    //显示特征点
    imshow("KeyPoints of img1", keyPointImg1);
    imshow("KeyPoints of img2", keyPointImg2);
    
    //ANMS去除野点
    if(isANMSRemove){
        key1 = _ANMSRemove(key1);
        key2 = _ANMSRemove(key2);
    
        drawKeypoints(data1, key1, keyPointImg1,
            Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        drawKeypoints(data2, key2, keyPointImg2,
            Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    
        imshow("KeyPoints of img1 after ANMS", keyPointImg1);
        imshow("KeyPoints of img2 after ANMS", keyPointImg2);
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
            break;
        case FeatDetParam::BFL1:
            matches = _BFL1Match(desc1, desc2, rType);
            break;
        case FeatDetParam::FLANN:
            matches = _FLANNMatch(desc1, desc2, rType);
            break;
        case FeatDetParam::RANSAC:
            matches = _RANSACMatch(desc1, desc2, key1, key2, rType);
            break;
        default:
            matches = _BFMatch(desc1, desc2, rType);
            break;
    }
    
    /*-- 去除野点 --*/
    DMatches goodMatches;
    switch (rType) {
        case FeatDetParam::TwoNN:
            if(mType==FeatDetParam::RANSAC) goodMatches=matches[0];
            else goodMatches = _NNDRRemove(matches, desc1, desc2);
            break;
        case FeatDetParam::Homography:
            goodMatches = _HomographyRemove(matches[0], key1, key2);
            break;
        default:
            goodMatches = _NNRemove(matches[0], desc1, desc2);
            break;
    }

    /*Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(mType);

    switch (rType) {
    case FeatDetParam::TwoNN:
        goodMatches = _2NNMatch(matcher, desc1, desc2);
    default:
        goodMatches = _defaultMatch(matcher, desc1, desc2);
    }*/

    Mat imageOutput;
    drawMatches(data1, key1, data2, key2, goodMatches, imageOutput);

    return imageOutput;
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
    
    //Mat homography = findHomography(srcPoints, dstPoints, inliersMask);
    Mat homography = findHomography(srcPoints, dstPoints, FM_RANSAC);
    
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

/*Mat ImageProcess::_featureDectect(Ptr<Feature2D> algo,
	const Mat &data1, const Mat &data2,
	FeatDetParam::RType rType, FeatDetParam::MType mType) {

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

	//显示特征点
	imshow("KeyPoints of img1", keyPointImg1);
	imshow("KeyPoints of img2", keyPointImg2);

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
	DMatches goodMatches;

	//如果采用flannBased方法 那么desp通过orb的到的类型不同需要先转换类型
	if (desc1.type() != CV_32F || desc2.type() != CV_32F) {
		desc1.convertTo(desc1, CV_32F);
		desc2.convertTo(desc2, CV_32F);
	}

	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(mType);

	switch (rType) {
	case FeatDetParam::TwoNN:
		goodMatches = _2NNMatch(matcher, desc1, desc2);
	default:
		goodMatches = _defaultMatch(matcher, desc1, desc2);
	}

	Mat imageOutput;
	drawMatches(data1, key1, data2, key2, goodMatches, imageOutput);

	return imageOutput;
}

// 野点去除-------------------------------------------------------------
ImageProcess::DMatches ImageProcess::_2NNMatch(
	const Ptr<DescriptorMatcher> matcher,
	const InputArray& queryDesc, const InputArray& trainDesc) {

	vector<DMatches> knnMatches;
	matcher->knnMatch(queryDesc, trainDesc, knnMatches, 2);

	DMatches matches;
	matches.reserve(knnMatches.size());
	//一般采用0.8作为阀值 1NN / 2NN < 0.8，这里采取opencv的阈值
	for (size_t i = 0; i < knnMatches.size(); i++) {
		const DMatch& closestMatch = knnMatches[i][0];
		const DMatch& secondClosestMatch = knnMatches[i][1];

		float distanceRatio = closestMatch.distance / secondClosestMatch.distance;

		//只有当比值小于minRatio才认为是好的匹配
		if (distanceRatio < TwoNNRatio)
			matches.push_back(closestMatch);
	}
	return matches;
}

ImageProcess::DMatches ImageProcess::_defaultMatch(
	const Ptr<DescriptorMatcher> matcher,
	const Mat& queryDesc, const Mat& trainDesc) {

	DMatches matches, goodMatches;
	matcher->match(queryDesc, trainDesc, matches);

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
}*/



//comMatR(conbine matrix as row):combine  Matrix1 and Matrix2 to MatrixCom as row ,just as the matlab expression :MatrixCom=[Matrix1 Matrix1]
Mat ImageProcess::comMatR(const Mat &Matrix1, const Mat &Matrix2, ProcessParam* _param)
{
	Ptr<Feature2D> algo;
	if (_param == NULL) return Matrix1;
	auto param = (FeatDetParam*)_param;
	switch (param->algo) {
	case FeatDetParam::SIFT: algo = xfeatures2d::SIFT::create(); break;
	case FeatDetParam::SURF: algo = xfeatures2d::SURF::create(); break;
	case FeatDetParam::ORB: algo = cv::ORB::create(); break;
	}

	//灰度图转换  
	Mat image1, image2;
	cvtColor(Matrix2, image1, CV_BGR2GRAY);
	cvtColor(Matrix1, image2, CV_BGR2GRAY);
	Mat imageDesc1, imageDesc2;
	KeyPoints keyPoint1, keyPoint2;
	vector<DMatch> GoodMatchePoints;
	vector<vector<DMatch>> matchePoints;
	if (param->algo == FeatDetParam::ORB)
	{
		//提取特征点    
		Ptr<ORB>  surfDetector = ORB::create(3000);
		surfDetector->detect(image1, keyPoint1);
		surfDetector->detect(image2, keyPoint2);

		//特征点描述，为下边的特征点匹配做准备    
		Ptr<ORB>  SurfDescriptor = ORB::create(3000);

		SurfDescriptor->compute(image1, keyPoint1, imageDesc1);
		SurfDescriptor->compute(image2, keyPoint2, imageDesc2);

		flann::Index flannIndex(imageDesc1, flann::LshIndexParams(12, 20, 2), cvflann::FLANN_DIST_HAMMING);

		Mat macthIndex(imageDesc2.rows, 2, CV_32SC1), matchDistance(imageDesc2.rows, 2, CV_32FC1);
		flannIndex.knnSearch(imageDesc2, macthIndex, matchDistance, 2, flann::SearchParams());
		// Lowe's algorithm,获取优秀匹配点
		for (int i = 0; i < matchDistance.rows; i++)
		{
			if (matchDistance.at<float>(i, 0) < 0.4 * matchDistance.at<float>(i, 1))
			{
				DMatch dmatches(i, macthIndex.at<int>(i, 0), matchDistance.at<float>(i, 0));
				GoodMatchePoints.push_back(dmatches);
			}
		}
	}
	else
	{
		//提取特征点    
		algo->detect(image1, keyPoint1);
		algo->detect(image2, keyPoint2);
		//特征点描述，为下边的特征点匹配做准备    
		algo->detectAndCompute(image1, Mat(), keyPoint1, imageDesc1);
		algo->detectAndCompute(image2, Mat(), keyPoint2, imageDesc2);
		if (imageDesc1.type() != CV_32F || imageDesc2.type() != CV_32F) {
			imageDesc1.convertTo(imageDesc1, CV_32F);
			imageDesc2.convertTo(imageDesc2, CV_32F);
		}
		FlannBasedMatcher matcher;

		vector<Mat> train_desc(1, imageDesc1);
		matcher.add(train_desc);
		matcher.train();

		matcher.knnMatch(imageDesc2, matchePoints, 2);
		// Lowe's algorithm,获取优秀匹配点
		for (int i = 0; i < matchePoints.size(); i++)
		{
			if (matchePoints[i][0].distance < 0.4 * matchePoints[i][1].distance)
			{
				GoodMatchePoints.push_back(matchePoints[i][0]);
			}
		}
	}


	Mat first_match;
	drawMatches(Matrix1, keyPoint2, Matrix2, keyPoint1, GoodMatchePoints, first_match);
	vector<Point2f> imagePoints1, imagePoints2;

	for (int i = 0; i < GoodMatchePoints.size(); i++)
	{
		imagePoints2.push_back(keyPoint2[GoodMatchePoints[i].queryIdx].pt);
		imagePoints1.push_back(keyPoint1[GoodMatchePoints[i].trainIdx].pt);
	}


	//获取图像1到图像2的投影映射矩阵 尺寸为3*3  
	Mat homo = findHomography(imagePoints1, imagePoints2, RANSAC);
	////也可以使用getPerspectiveTransform方法获得透视变换矩阵，不过要求只能有4个点，效果稍差  
	//Mat   homo=getPerspectiveTransform(imagePoints1,imagePoints2);  

   //计算配准图的四个顶点坐标
	CalcCorners(homo, Matrix2);

	//图像配准  
	Mat imageTransform1, imageTransform2;
	warpPerspective(Matrix2, imageTransform1, homo, Size(MAX(corners.right_top.x, corners.right_bottom.x), Matrix1.rows));
	//warpPerspective(Matrix1, imageTransform2, adjustMat*homo, Size(Matrix2.cols*1.3, Matrix2.rows*1.8));
	//创建拼接后的图,需提前计算图的大小
	/*int dst_width = Matrix1.cols + Matrix2.cols - imageTransform1.cols;  //取最右点的长度为拼接图的长度
	int dst_height = Matrix1.rows + Matrix2.rows - imageTransform1.rows;*/
	int dst_width = imageTransform1.cols;  //取最右点的长度为拼接图的长度
	int dst_height = Matrix1.rows;

	Mat dst(dst_height, dst_width, CV_8UC3);
	dst.setTo(0);
	/*imageTransform1.copyTo(dst(Rect(dst_width-imageTransform1.cols, dst_height-imageTransform1.rows,
		imageTransform1.cols, imageTransform1.rows)));*/
	imageTransform1.copyTo(dst(Rect(0, 0,
		imageTransform1.cols, imageTransform1.rows)));
	Matrix1.copyTo(dst(Rect(0, 0, Matrix1.cols, Matrix1.rows)));

	OptimizeSeam(Matrix1, imageTransform1, dst);

	return dst;
}


//优化两图的连接处，使得拼接自然
void ImageProcess::OptimizeSeam(const Mat& img1, Mat& trans, Mat& dst)
{
	int start = MIN(corners.left_top.x, corners.left_bottom.x);//开始位置，即重叠区域的左边界  

	double processWidth = img1.cols - start;//重叠区域的宽度  
	int rows = dst.rows;
	int cols = img1.cols; //注意，是列数*通道数
	double alpha = 1;//img1中像素的权重  
	for (int i = 0; i < rows; i++)
	{
		const uchar* p = img1.ptr<uchar>(i);  //获取第i行的首地址
		uchar* t = trans.ptr<uchar>(i);
		uchar* d = dst.ptr<uchar>(i);
		for (int j = start; j < cols; j++)
		{
			//如果遇到图像trans中无像素的黑点，则完全拷贝img1中的数据
			if (t[j * 3] == 0 && t[j * 3 + 1] == 0 && t[j * 3 + 2] == 0)
			{
				alpha = 1;
			}
			else
			{
				//img1中像素的权重，与当前处理点距重叠区域左边界的距离成正比，实验证明，这种方法确实好  
				alpha = (processWidth - (j - start)) / processWidth;
			}

			d[j * 3] = p[j * 3] * alpha + t[j * 3] * (1 - alpha);
			d[j * 3 + 1] = p[j * 3 + 1] * alpha + t[j * 3 + 1] * (1 - alpha);
			d[j * 3 + 2] = p[j * 3 + 2] * alpha + t[j * 3 + 2] * (1 - alpha);

		}
	}
}

void ImageProcess::CalcCorners(const Mat& H, const Mat& src)
{
	double v2[] = { 0, 0, 1 };//左上角
	double v1[3];//变换后的坐标值
	Mat V2 = Mat(3, 1, CV_64FC1, v2);  //列向量
	Mat V1 = Mat(3, 1, CV_64FC1, v1);  //列向量

	V1 = H * V2;
	//左上角(0,0,1)
	corners.left_top.x = v1[0] / v1[2];
	corners.left_top.y = v1[1] / v1[2];

	//左下角(0,src.rows,1)
	v2[0] = 0;
	v2[1] = src.rows;
	v2[2] = 1;
	V2 = Mat(3, 1, CV_64FC1, v2);  //列向量
	V1 = Mat(3, 1, CV_64FC1, v1);  //列向量
	V1 = H * V2;
	corners.left_bottom.x = v1[0] / v1[2];
	corners.left_bottom.y = v1[1] / v1[2];

	//右上角(src.cols,0,1)
	v2[0] = src.cols;
	v2[1] = 0;
	v2[2] = 1;
	V2 = Mat(3, 1, CV_64FC1, v2);  //列向量
	V1 = Mat(3, 1, CV_64FC1, v1);  //列向量
	V1 = H * V2;
	corners.right_top.x = v1[0] / v1[2];
	corners.right_top.y = v1[1] / v1[2];

	//右下角(src.cols,src.rows,1)
	v2[0] = src.cols;
	v2[1] = src.rows;
	v2[2] = 1;
	V2 = Mat(3, 1, CV_64FC1, v2);  //列向量
	V1 = Mat(3, 1, CV_64FC1, v1);  //列向量
	V1 = H * V2;
	corners.right_bottom.x = v1[0] / v1[2];
	corners.right_bottom.y = v1[1] / v1[2];

}
