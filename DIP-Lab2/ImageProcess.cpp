
#include "ImageProcess.h"

const float ImageProcess::TwoNNRatio = 1.0 / 1.5;

double ImageProcess::progress = 0;

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
	return _featureDectect(algo, data1, data2, param->rType, param->mType);
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
    Mat homography = findHomography(srcPoints, dstPoints, CV_FM_RANSAC, inliersMask);
    
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
