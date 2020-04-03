
#include "ImageProcess.h"

void ImageProcess::doFERNS(QImage img1, QImage img2, QImage &out1, QImage &out2, 
	int x, int y, int w, int h) {
	auto data1 = QTCVUtils::qImage2Mat(img1);
	auto data2 = QTCVUtils::qImage2Mat(img2);

	cvtColor(data1, data1, COLOR_BGR2GRAY);
	cvtColor(data2, data2, COLOR_BGR2GRAY);

	auto tmplImg1 = ImgT(data1);
	auto tmplImg2 = ImgT(data2);
	auto tmplBB = BB(x, y, w, h);

	rectangle(tmplImg1.img, tmplBB, Scalar::all(255.0));

	out1 = QTCVUtils::mat2QImage(tmplImg1.img);

	auto featuresExtractor = make_shared<HaarFeaturesExtractor>(5, 10);
	auto classifier = make_shared<ForestClassifier<FernClassifier> >(10, 5);
	auto scanner = make_shared<Scanner>(Size(24, 24), Size(320, 240), 0.25, 1.2);
	auto detector = Detector<ForestClassifier<FernClassifier>,
		HaarFeaturesExtractor>(scanner, classifier, featuresExtractor);

	detector.learn(tmplImg1, tmplBB, true);

	vector<BB> objs;
	vector<float> probs;

	detector.detect(tmplImg2, objs, probs);

	for_each(begin(objs), end(objs), [&tmplImg2](BBRefC obj) {
		rectangle(tmplImg2.img, obj, Scalar::all(255));
	});

	out2 = QTCVUtils::mat2QImage(tmplImg2.img);
}

QImage ImageProcess::doSIFT(QImage img1, QImage img2) {
	auto data1 = QTCVUtils::qImage2Mat(img1);
	auto data2 = QTCVUtils::qImage2Mat(img2);

	Ptr<xfeatures2d::SIFT> algo = xfeatures2d::SIFT::create();

	Mat res = _featureDectect(algo, data1, data2);

	return QTCVUtils::mat2QImage(res);
}

QImage ImageProcess::doSURF(QImage img1, QImage img2) {
	auto data1 = QTCVUtils::qImage2Mat(img1);
	auto data2 = QTCVUtils::qImage2Mat(img2);

	Ptr<xfeatures2d::SURF> algo = xfeatures2d::SURF::create();
	Mat res = _featureDectect(algo, data1, data2);

	return QTCVUtils::mat2QImage(res);
}

QImage ImageProcess::doORB(QImage img1, QImage img2) {
	auto data1 = QTCVUtils::qImage2Mat(img1);
	auto data2 = QTCVUtils::qImage2Mat(img2);

	Ptr<ORB> algo = ORB::create();
	Mat res = _featureDectect(algo, data1, data2);

	return QTCVUtils::mat2QImage(res);
}

Mat ImageProcess::_featureDectect(Feature2D* algo, Mat &data1, Mat &data2) {

	//������
	KeyPoints key1, key2;
	//������ȡ������
	algo->detect(data1, key1);
	algo->detect(data2, key2);

	//��������
	Mat keyPointImageL;
	Mat keyPointImageR;
	drawKeypoints(data1, key1, keyPointImageL, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
	drawKeypoints(data2, key2, keyPointImageR, Scalar::all(-1), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

	//��ʾ����
	namedWindow("KeyPoints of imageL");
	namedWindow("KeyPoints of imageR");

	//��ʾ������
	imshow("KeyPoints of imageL", keyPointImageL);
	imshow("KeyPoints of imageR", keyPointImageR);

	//������ƥ��
	Mat despL, despR;
	//��ȡ�����㲢��������������
	algo->detectAndCompute(data1, Mat(), key1, despL);
	algo->detectAndCompute(data2, Mat(), key2, despR);

	//Struct for DMatch: query descriptor index, train descriptor index, train image index and distance between descriptors.
	//int queryIdx �C>�ǲ���ͼ�����������������descriptor�����±꣬ͬʱҲ����������Ӧ�����㣨keypoint)���±ꡣ
	//int trainIdx �C> ������ͼ������������������±꣬ͬ��Ҳ����Ӧ����������±ꡣ
	//int imgIdx �C>�������Ƕ���ͼ��Ļ����á�
	//float distance �C>������һ��ƥ�������������������������������ŷ�Ͼ��룬��ֵԽСҲ��˵������������Խ����
	vector<DMatch> matches;

	//�������flannBased���� ��ô despͨ��orb�ĵ������Ͳ�ͬ��Ҫ��ת������
	if (despL.type() != CV_32F || despR.type() != CV_32F) {
		despL.convertTo(despL, CV_32F);
		despR.convertTo(despR, CV_32F);
	}

	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("FlannBased");
	matcher->match(despL, despR, matches);

	//�����������������ֵ 
	double maxDist = 0;
	for (int i = 0; i < despL.rows; i++) {
		double dist = matches[i].distance;
		if (dist > maxDist)
			maxDist = dist;
	}

	//��ѡ�õ�ƥ���
	vector< DMatch > good_matches;
	for (int i = 0; i < despL.rows; i++) {
		if (matches[i].distance < 0.5*maxDist) {
			good_matches.push_back(matches[i]);
		}
	}

	Mat imageOutput;
	drawMatches(data1, key1, data2, key2, good_matches, imageOutput);

	return imageOutput;
}
