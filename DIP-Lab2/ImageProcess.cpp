#include <opencv2/imgproc/types_c.h>

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

	cv::rectangle(tmplImg1.img, tmplBB, Scalar::all(255.0));

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
