
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
	bool auto_ = param->adType != ObjDetTrackParam::None; // �Ƿ��Զ����
	int duration = 0;

	Ptr<Tracker> tracker = NULL;

	// ÿ֡����
	for (int i = 0; i < inLen; ++i, ++duration) {
		progress = i * 1.0 / inLen;

		Mat frame = inVideo[i];
		Mat &outFrame = outVideo[i];
		outFrame = frame.clone();

		// ���Ϊ��һ֡������Ҫ�Զ���Ⲣ����һ֡û�и��ٵ����߼��֡������ָ����Ŀ
		// ��û�м���㷨ʱ����һ֡ҲҪ����һ�� doObjDet ���������ڻ��ƾ���
		if (i == 0 || (auto_ && (trackSucc == false || duration >= DetDuration))) {
			duration = 0;
			outFrame = doObjDet(frame, param);
			newDet = !param->getRect().empty();
		}

		if (i == 0) continue; // ����ǵ�һ֡�������и���

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
		if (trackSucc) LOG("�� " << i << " ֡���ٳɹ���");
		else LOG("�� " << i << " ֡����ʧ�ܣ�");

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
	// ����
	static CascadeClassifier cascade;
	if (cascade.empty()) 
		// ����ѵ���õ� �����������.xml��
		if (!cascade.load(FaceDetPath)) LOG("�������������ʧ��");

	if (cascade.empty()) return Rect(0, 0, 0, 0);
	
	vector<Rect> faces(0);
	cascade.detectMultiScale(data, faces, 1.1, 2, 0, Size(30, 30));

	if (faces.size() > 0) {
		LOG("��⵽ " << faces.size() << "�����������ص�һ��");
		return faces[0];
	}
	
	cout << "δ��⵽����" << endl;
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

	BB avgRect; // ���ж��ƥ�䣬���ֵ
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
	else LOG("����ʧ�ܣ�");
}

bool ImageProcess::_trackerTrack(Ptr<Tracker> &tracker, bool &newDet,
	const Mat &frame, Mat &out, ObjDetTrackParam* param) {
	Rect2d rect = param->getRect();

	if (newDet) { // ���¼��ظ�����
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
    //������
    KeyPoints key1, key2;
    //������ȡ������
    algo->detect(data1, key1);
    algo->detect(data2, key2);

    //��������
    Mat keyPointImg1;
    Mat keyPointImg2;
    drawKeypoints(data1, key1, keyPointImg1,
        Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
    drawKeypoints(data2, key2, keyPointImg2,
        Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

    //��ʾ������
    imshow("KeyPoints of img1", keyPointImg1);
    imshow("KeyPoints of img2", keyPointImg2);
    
    //ANMSȥ��Ұ��
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
    
    //������ƥ��
    Mat desc1, desc2;
    //��ȡ�����㲢��������������
    algo->detectAndCompute(data1, Mat(), key1, desc1);
    algo->detectAndCompute(data2, Mat(), key2, desc2);

    //Struct for DMatch: query descriptor index, train descriptor index, train image index and distance between descriptors.
    //int queryIdx �C>�ǲ���ͼ�����������������descriptor�����±꣬ͬʱҲ����������Ӧ�����㣨keypoint)���±ꡣ
    //int trainIdx �C> ������ͼ������������������±꣬ͬ��Ҳ����Ӧ����������±ꡣ
    //int imgIdx �C>�������Ƕ���ͼ��Ļ����á�
    //float distance �C>������һ��ƥ�������������������������������ŷ�Ͼ��룬��ֵԽСҲ��˵������������Խ����

    //�������flannBased���� ��ôdespͨ��orb�ĵ������Ͳ�ͬ��Ҫ��ת������
    if (desc1.type() != CV_32F || desc2.type() != CV_32F) {
        desc1.convertTo(desc1, CV_32F);
        desc2.convertTo(desc2, CV_32F);
    }
    
    /*-- ����ƥ�� --*/
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
    
    /*-- ȥ��Ұ�� --*/
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

/*-- ����ƥ�� --*/
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
    
    //�洢ɾѡ���ƥ��
    DMatches goodMatches = matches;
    
    //���ʣ��ƥ���Ѿ��㹻������ɾ��
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

/*-- Ұ��ȥ�� --*/
ImageProcess::DMatches ImageProcess::_NNRemove(
    const ImageProcess::DMatches& matches,
    const Mat& queryDesc, const Mat& trainDesc){
    //�洢ɸѡ���ƥ��
    DMatches goodMatches;

    //�����������������ֵ
    double maxDist = 0;
    for (int i = 0; i < queryDesc.rows; i++) {
        double dist = matches[i].distance;
        if (dist > maxDist) maxDist = dist;
    }

    //��ѡ�õ�ƥ���
    for (int i = 0; i < queryDesc.rows; i++)
        if (matches[i].distance < 0.5*maxDist)
            goodMatches.push_back(matches[i]);

    return goodMatches;
}

ImageProcess::DMatches ImageProcess::_NNDRRemove(
    const vector<ImageProcess::DMatches>& matches,
    const Mat& queryDesc, const Mat& trainDesc){
    //�洢ɸѡ���ƥ��
    DMatches goodMatches;
    goodMatches.reserve(matches.size());
    
    //һ�����0.8��Ϊ��ֵ 1NN / 2NN < 0.8�������ȡopencv����ֵ
    for (size_t i = 0; i < matches.size(); i++) {
        const DMatch& closestMatch = matches[i][0];
        const DMatch& secondClosestMatch = matches[i][1];

        float distanceRatio = closestMatch.distance / secondClosestMatch.distance;

        //ֻ�е���ֵС��minRatio����Ϊ�Ǻõ�ƥ��
        if (distanceRatio < TwoNNRatio)
            goodMatches.push_back(closestMatch);
    }
    return goodMatches;
}

ImageProcess::DMatches ImageProcess::_HomographyRemove(const DMatches& matches,
    const KeyPoints& queryKeypoints, const KeyPoints& trainKeypoints){
    //�洢ɾѡ���ƥ��
    DMatches goodMatches = matches;
    
    //���ʣ��ƥ���Ѿ��㹻������ɾ��
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

	//������
	KeyPoints key1, key2;
	//������ȡ������
	algo->detect(data1, key1);
	algo->detect(data2, key2);

	//��������
	Mat keyPointImg1;
	Mat keyPointImg2;
	drawKeypoints(data1, key1, keyPointImg1, 
		Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	drawKeypoints(data2, key2, keyPointImg2, 
		Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

	//��ʾ������
	imshow("KeyPoints of img1", keyPointImg1);
	imshow("KeyPoints of img2", keyPointImg2);

	//������ƥ��
	Mat desc1, desc2;
	//��ȡ�����㲢��������������
	algo->detectAndCompute(data1, Mat(), key1, desc1);
	algo->detectAndCompute(data2, Mat(), key2, desc2);

	//Struct for DMatch: query descriptor index, train descriptor index, train image index and distance between descriptors.
	//int queryIdx �C>�ǲ���ͼ�����������������descriptor�����±꣬ͬʱҲ����������Ӧ�����㣨keypoint)���±ꡣ
	//int trainIdx �C> ������ͼ������������������±꣬ͬ��Ҳ����Ӧ����������±ꡣ
	//int imgIdx �C>�������Ƕ���ͼ��Ļ����á�
	//float distance �C>������һ��ƥ�������������������������������ŷ�Ͼ��룬��ֵԽСҲ��˵������������Խ����
	DMatches goodMatches;

	//�������flannBased���� ��ôdespͨ��orb�ĵ������Ͳ�ͬ��Ҫ��ת������
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

// Ұ��ȥ��-------------------------------------------------------------
ImageProcess::DMatches ImageProcess::_2NNMatch(
	const Ptr<DescriptorMatcher> matcher,
	const InputArray& queryDesc, const InputArray& trainDesc) {

	vector<DMatches> knnMatches;
	matcher->knnMatch(queryDesc, trainDesc, knnMatches, 2);

	DMatches matches;
	matches.reserve(knnMatches.size());
	//һ�����0.8��Ϊ��ֵ 1NN / 2NN < 0.8�������ȡopencv����ֵ
	for (size_t i = 0; i < knnMatches.size(); i++) {
		const DMatch& closestMatch = knnMatches[i][0];
		const DMatch& secondClosestMatch = knnMatches[i][1];

		float distanceRatio = closestMatch.distance / secondClosestMatch.distance;

		//ֻ�е���ֵС��minRatio����Ϊ�Ǻõ�ƥ��
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

	//�����������������ֵ 
	double maxDist = 0;
	for (int i = 0; i < queryDesc.rows; i++) {
		double dist = matches[i].distance;
		if (dist > maxDist) maxDist = dist;
	}

	//��ѡ�õ�ƥ���
	for (int i = 0; i < queryDesc.rows; i++)
		if (matches[i].distance < 0.5*maxDist)
			goodMatches.push_back(matches[i]);

	return goodMatches;
}*/
