
#include "ImageProcess.h"
#include <qdebug.h>
const float ImageProcess::TwoNNRatio = 1.0 / 1.5;
//ͼ��ƴ���õ����ĸ���
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

	//�Ҷ�ͼת��  
	Mat image1, image2;
	cvtColor(Matrix2, image1, CV_RGB2GRAY);
	cvtColor(Matrix1, image2, CV_RGB2GRAY);


	//��ȡ������    
	KeyPoints keyPoint1, keyPoint2;
	algo->detect(image1, keyPoint1);
	algo->detect(image2, keyPoint2);
	//������������Ϊ�±ߵ�������ƥ����׼��    
	Mat imageDesc1, imageDesc2;
	algo->detectAndCompute(image1, Mat(), keyPoint1, imageDesc1);
	algo->detectAndCompute(image2, Mat(), keyPoint2, imageDesc2);
	if (imageDesc1.type() != CV_32F || imageDesc2.type() != CV_32F) {
		imageDesc1.convertTo(imageDesc1, CV_32F);
		imageDesc2.convertTo(imageDesc2, CV_32F);
	}
	FlannBasedMatcher matcher;
	vector<vector<DMatch>> matchePoints;
	vector<DMatch> GoodMatchePoints;

	vector<Mat> train_desc(1, imageDesc1);
	matcher.add(train_desc);
	matcher.train();

	matcher.knnMatch(imageDesc2, matchePoints, 2);

	// Lowe's algorithm,��ȡ����ƥ���
	for (int i = 0; i < matchePoints.size(); i++)
	{
		if (matchePoints[i][0].distance < 0.4 * matchePoints[i][1].distance)
		{
			GoodMatchePoints.push_back(matchePoints[i][0]);
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



	//��ȡͼ��1��ͼ��2��ͶӰӳ����� �ߴ�Ϊ3*3  
	Mat homo = findHomography(imagePoints1, imagePoints2, RANSAC);
	////Ҳ����ʹ��getPerspectiveTransform�������͸�ӱ任���󣬲���Ҫ��ֻ����4���㣬Ч���Բ�  
	//Mat   homo=getPerspectiveTransform(imagePoints1,imagePoints2);  

   //������׼ͼ���ĸ���������
	CalcCorners(homo, Matrix2);

	//ͼ����׼  
	Mat imageTransform1, imageTransform2;
	warpPerspective(Matrix2, imageTransform1, homo, Size(MAX(corners.right_top.x, corners.right_bottom.x), Matrix1.rows));
	//warpPerspective(Matrix1, imageTransform2, adjustMat*homo, Size(Matrix2.cols*1.3, Matrix2.rows*1.8));
	//����ƴ�Ӻ��ͼ,����ǰ����ͼ�Ĵ�С
	/*int dst_width = Matrix1.cols + Matrix2.cols - imageTransform1.cols;  //ȡ���ҵ�ĳ���Ϊƴ��ͼ�ĳ���
	int dst_height = Matrix1.rows + Matrix2.rows - imageTransform1.rows;*/
	int dst_width = imageTransform1.cols;  //ȡ���ҵ�ĳ���Ϊƴ��ͼ�ĳ���
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


//�Ż���ͼ�����Ӵ���ʹ��ƴ����Ȼ
void ImageProcess::OptimizeSeam(const Mat& img1, Mat& trans, Mat& dst)
{
	int start = MIN(corners.left_top.x, corners.left_bottom.x);//��ʼλ�ã����ص��������߽�  

	double processWidth = img1.cols - start;//�ص�����Ŀ��  
	int rows = dst.rows;
	int cols = img1.cols; //ע�⣬������*ͨ����
	double alpha = 1;//img1�����ص�Ȩ��  
	for (int i = 0; i < rows; i++)
	{
		const uchar* p = img1.ptr<uchar>(i);  //��ȡ��i�е��׵�ַ
		uchar* t = trans.ptr<uchar>(i);
		uchar* d = dst.ptr<uchar>(i);
		for (int j = start; j < cols; j++)
		{
			//�������ͼ��trans�������صĺڵ㣬����ȫ����img1�е�����
			if (t[j * 3] == 0 && t[j * 3 + 1] == 0 && t[j * 3 + 2] == 0)
			{
				alpha = 1;
			}
			else
			{
				//img1�����ص�Ȩ�أ��뵱ǰ�������ص�������߽�ľ�������ȣ�ʵ��֤�������ַ���ȷʵ��  
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
	double v2[] = { 0, 0, 1 };//���Ͻ�
	double v1[3];//�任�������ֵ
	Mat V2 = Mat(3, 1, CV_64FC1, v2);  //������
	Mat V1 = Mat(3, 1, CV_64FC1, v1);  //������

	V1 = H * V2;
	//���Ͻ�(0,0,1)
	corners.left_top.x = v1[0] / v1[2];
	corners.left_top.y = v1[1] / v1[2];

	//���½�(0,src.rows,1)
	v2[0] = 0;
	v2[1] = src.rows;
	v2[2] = 1;
	V2 = Mat(3, 1, CV_64FC1, v2);  //������
	V1 = Mat(3, 1, CV_64FC1, v1);  //������
	V1 = H * V2;
	corners.left_bottom.x = v1[0] / v1[2];
	corners.left_bottom.y = v1[1] / v1[2];

	//���Ͻ�(src.cols,0,1)
	v2[0] = src.cols;
	v2[1] = 0;
	v2[2] = 1;
	V2 = Mat(3, 1, CV_64FC1, v2);  //������
	V1 = Mat(3, 1, CV_64FC1, v1);  //������
	V1 = H * V2;
	corners.right_top.x = v1[0] / v1[2];
	corners.right_top.y = v1[1] / v1[2];

	//���½�(src.cols,src.rows,1)
	v2[0] = src.cols;
	v2[1] = src.rows;
	v2[2] = 1;
	V2 = Mat(3, 1, CV_64FC1, v2);  //������
	V1 = Mat(3, 1, CV_64FC1, v1);  //������
	V1 = H * V2;
	corners.right_bottom.x = v1[0] / v1[2];
	corners.right_bottom.y = v1[1] / v1[2];

}