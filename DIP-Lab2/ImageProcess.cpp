
#include "ImageProcess.h"

const float ImageProcess::TwoNNRatio = 1.0 / 1.5;

Mat ImageProcess::drawRect(const Mat &data, const ProcessParam* _param /*= NULL*/) {
	if (_param == NULL) return data;
	auto param = (RectParam*)_param;

	Mat out = data.clone();
	Rect rect(param->x, param->y, param->w, param->h);
	rectangle(out, rect, param->color);

	return out;
}

void ImageProcess::doFERNS(const Mat &data1, const Mat &data2, 
	Mat &out1, Mat &out2, const ProcessParam* _param /*= NULL*/) {
	out1 = data1.clone(); out2 = data2.clone();

	if (_param == NULL) return;
	auto param = (RectParam*)_param;

	cvtColor(out1, out1, COLOR_BGR2GRAY);
	cvtColor(out2, out2, COLOR_BGR2GRAY);

	auto tmplImg1 = ImgT(out1), tmplImg2 = ImgT(out2);
	auto tmplBB = BB(param->x, param->y, param->w, param->h);

	rectangle(out1, tmplBB, param->color);

	auto featuresExtractor = make_shared<HaarFeaturesExtractor>(5, 10);
	auto classifier = make_shared<ForestClassifier<FernClassifier> >(10, 5);
	auto scanner = make_shared<Scanner>(Size(24, 24), Size(320, 240), 0.25, 1.2);
	auto detector = Detector<ForestClassifier<FernClassifier>,
		HaarFeaturesExtractor>(scanner, classifier, featuresExtractor);

	detector.learn(tmplImg1, tmplBB, true);

	vector<BB> objs;
	vector<float> probs;

	detector.detect(tmplImg2, objs, probs);

	for_each(begin(objs), end(objs), [&out2, &param](BBRefC obj) {
		rectangle(out2, obj, param->color);
	});
}

Mat ImageProcess::doFeatDet(const Mat &data1, const Mat &data2, const ProcessParam* _param /*= NULL*/) {

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

Mat ImageProcess::_featureDectect(Ptr<Feature2D> algo,
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
}
