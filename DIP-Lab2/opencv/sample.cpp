// һЩֱ��ִ�е�����

#include <opencv2\highgui\highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>

#include "mycv.h"

using namespace std;
using namespace cv;
using namespace mycv;
/*
int main(int argc, char *argv[]) {
	Mat img = imread("PrettyU.jpg");
	namedWindow("display");
	imshow("display", img);

	//*********************************** 1.�������������  ******************************
	// ��������������
	CascadeClassifier cascade;
	// ����ѵ���õ� �����������.xml��
	const string path = "./xml/haarcascade_frontalface_alt.xml";
	if (!cascade.load(path)) {
		cout << "cascade load failed!\n";
	}

	//��ʱ
	double t = 0;
	t = (double)getTickCount();
	//*********************************** 2.������� ******************************
	vector<Rect> faces(0);
	cascade.detectMultiScale(img, faces, 1.1, 2, 0, Size(30, 30));

	cout << "detect face number is :" << faces.size() << endl;
	//********************************  3.��ʾ�������ο� ******************************

	if (faces.size() > 0) {
		for (size_t i = 0; i < faces.size(); i++) {
			rectangle(img, faces[i], Scalar(150, 0, 0), 3, 8, 0);

		}
	} else cout << "δ��⵽����" << endl;

	t = (double)getTickCount() - t;  //getTickCount():  Returns the number of ticks per second.
	cout << "���������ʱ��" << t * 1000 / getTickFrequency() << "ms (���������ģ�͵�ʱ�䣩" << endl;

	namedWindow("face_detect");
	imshow("face_detect", img);
	while (waitKey(0) != 'k');
	destroyWindow("display");
	destroyWindow("face_detect");
	return 0;
}
*/
/*
using namespace cv;
using namespace ml;
using namespace std;

int main() {
	//1.����ѵ�����ṹ�����ָ��
	Ptr<TrainData> data_set = TrainData::loadFromCSV("agaricus-lepiota.data",//�ļ���
		0,//��0���Թ�
		0,
		1,//����[0,1)Ϊ�洢����Ӧ��
		"cat[0-22]",//0-22�о�Ϊ�������
		',',//���ݼ�ķָ�����Ϊ","
		'?');//��ʧ������"?"��ʾ

	//2.��֤���ݶ�ȡ����ȷ��
	int n_samples = data_set->getNSamples();
	if (n_samples == 0) {
		cerr << "Could not read file: mushroom.data" << endl;
		exit(-1);
	} else {
		cout << "Read " << n_samples << " samples from mushroom.data" << endl;
	}

	//3.�ָ�ѵ�����Ͳ��Լ�������Ϊ9:1�����������ݼ���˳��
	data_set->setTrainTestSplitRatio(0.90, false);
	int n_train_samples = data_set->getNTrainSamples();
	int n_test_samples = data_set->getNTestSamples();
	Mat trainMat = data_set->getTrainSamples();
	//4.������
	//4.1 ����
	Ptr<RTrees> dtree = RTrees::create();
	//4.2 ��������
	dtree->setMaxDepth(8); //����������
	dtree->setMinSampleCount(10); //�ڵ�����������Сֵ
	dtree->setRegressionAccuracy(0.01f);
	dtree->setUseSurrogates(false);//�Ƿ�����ʹ������ֲ�㴦��ʧ������
	dtree->setMaxCategories(15);//�����������Ԥ��������
	dtree->setCVFolds(0);//��� CVFolds>1 ��ô��ʹ��k-fold�����޽������� ����k=CVFolds
	dtree->setUse1SERule(true);//True ��ʾʹ�ø������ȵ��޼�����ᵼ�����Ĺ�ģ��С����׼ȷ�Ը�����ڽ�����������
	dtree->setTruncatePrunedTree(true);//�Ƿ�ɾ������֦�Ĳ���
	float _priors[] = { 1.0,10.0 };
	Mat priors(1, 2, CV_32F, _priors);
	dtree->setPriors(priors);//Ϊ���еĴ�����Ȩ��
	//4.3 ѵ��
	dtree->train(data_set);
	//4.4 ����ѵ�����
	Mat results;
	float train_performance = dtree->calcError(data_set,
		false,//true ��ʾʹ�ò��Լ�  false ��ʾʹ��ѵ����
		results);

	vector<String> names;
	data_set->getNames(names);

	Mat flags = data_set->getVarSymbolFlags();

	//5 ѵ�����Ľ������
	{
		Mat expected_responses = data_set->getResponses();
		int good = 0, bad = 0, total = 0;
		for (int i = 0; i < data_set->getNTrainSamples(); ++i) {
			float received = results.at<float>(i, 0);
			float expected = expected_responses.at<float>(i, 0);
			String r_str = names[(int)received];
			String e_str = names[(int)expected];
			if (received != expected) {
				bad++;
				cout << "Expected: " << e_str << " ,got: " << r_str << endl;
			} else good++;
			total++;
		}
		cout << "Correct answers: " << (float(good) / total) * 100 << "% " << endl;
		cout << "Incorrect answers: " << (float(bad) / total) * 100 << "% " << endl;
	}

	//6 ���Լ��Ľ������
	float test_performance = dtree->calcError(data_set, true, results);
	cout << "Performance on training data: " << (1-train_performance)*100 << "%" << endl;
	cout << "Performance on test data: " << (1-test_performance)*100 << "%" << endl;

	//6 ѵ�����Ľ������
	{
		Mat expected_responses = data_set->getTestResponses();
		int good = 0, bad = 0, total = 0;
		for (int i = 0; i < data_set->getNTestSamples(); ++i) {
			float received = results.at<float>(i, 0);
			float expected = expected_responses.at<float>(i, 0);
			String r_str = names[(int)received];
			String e_str = names[(int)expected];
			if (received != expected) {
				bad++;
				cout << "Expected: " << e_str << " ,got: " << r_str << endl;
			} else good++;
			total++;
		}
		cout << "Correct answers: " << (float(good) / total) * 100 << "% " << endl;
		cout << "Incorrect answers: " << (float(bad) / total) * 100 << "% " << endl;
	}

	//����
	dtree->save("dtree_01.xml");

	system("pause");

	return 0;
}
*/
/*
int main() {
	//����Ģ����������ɭ��
 //1.����ѵ�����Ͳ��Լ�
	Ptr<TrainData> data_set = TrainData::loadFromCSV("agaricus-lepiota.data",//�ļ���
		0,//��0���Թ�
		0,
		1,//����[0,1)Ϊ�洢����Ӧ��
		"cat[0-22]",//0-22�о�Ϊ�������
		',',//���ݼ�ķָ�����Ϊ","
		'?');//��ʧ������"?"��ʾ
//2.��֤���ݶ�ȡ����ȷ��
	int n_samples = data_set->getNSamples();
	if (n_samples == 0) {
		cerr << "Could not read file: mushroom.data" << endl;
		exit(-1);
	} else {
		cout << "Read " << n_samples << " samples from mushroom.data" << endl;
	}

	//3.�ָ�ѵ�����Ͳ��Լ�������Ϊ9:1���������ݼ���˳��
	data_set->setTrainTestSplitRatio(0.90, true);
	int n_train_samples = data_set->getNTrainSamples();
	int n_test_samples = data_set->getNTestSamples();

	//4.���ɭ��
	Ptr<RTrees> forest_mushroom = RTrees::create();
	forest_mushroom->setMaxDepth(10); //����������
	forest_mushroom->setRegressionAccuracy(0.01f);//���ûع龫��
	forest_mushroom->setMinSampleCount(10);//�ڵ����С��������
	forest_mushroom->setMaxCategories(15);//���Ԥ������
	forest_mushroom->setCalculateVarImportance(true);//�����������Ҫ��
	forest_mushroom->setActiveVarCount(4);//���ڵ����ѡ�������Ӽ��Ĵ�С
	forest_mushroom->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER +
		TermCriteria::EPS, 100, 0.01));//��ֹ��׼
//ѵ��ģ��
	forest_mushroom->train(data_set);
	//����ѵ�����Ͳ��Լ������
	float correct_Train_answer = 0;
	float correct_Test_answer = 0;
	//1.ѵ����
	Mat trainSample = data_set->getTrainSamples();
	Mat trainResMat = data_set->getTrainResponses();
	for (int i = 0; i < trainSample.rows; i++) {
		Mat sample = trainSample.row(i);
		float r = forest_mushroom->predict(sample);
		r = fabs((float)r - trainResMat.at<float>(i)) <= FLT_EPSILON ? 1 : 0;
		correct_Train_answer += r;
	}
	float r1 = correct_Train_answer / n_train_samples;
	//2.���Լ�
	Mat testSample = data_set->getTestSamples();
	Mat testResMat = data_set->getTestResponses();
	for (int i = 0; i < testSample.rows; i++) {
		Mat sample = testSample.row(i);
		float r = forest_mushroom->predict(sample);
		r = fabs((float)r - testResMat.at<float>(i)) <= FLT_EPSILON ? 1 : 0;
		correct_Test_answer += r;
	}
	float r2 = correct_Test_answer / n_test_samples;
	//3.������
	cout << "trainSet Accuracy: " << r1 * 100 << "%" << endl;
	cout << "testSet Accuracy:  " << r2 * 100 << "%" << endl;
	//4.����ģ��
	forest_mushroom->save("forest_mushroom.xml");

	system("pause");
}
*/
// obj.train.detect.cpp : Defines the entry point for the console application.
//


int fernsSample() {

	auto tmpl_img = ImgT(imread("sample/1.jpg", 0), true);
	auto test_img = ImgT(imread("sample/7.jpg", 0), true);

	auto tmpl_bb = BB(40, 32, 82, 88);
	//auto tmpl_bb = BB(115.0f, 145.0f, 120.0f, 160.0f);
	//auto tmpl_bb = BB(135.0f, 155.0f, 80.0f, 140.0f);

	rectangle(tmpl_img.img, tmpl_bb, Scalar::all(255.0));
	imshow("dbg", tmpl_img.img);

	auto featuresExtractor = make_shared<HaarFeaturesExtractor>(5, 10);
	auto classifier = make_shared<ForestClassifier<FernClassifier> >(10, 5);
	auto scanner = make_shared<Scanner>(Size(24, 24), Size(320, 240), 0.25, 1.2);
	auto detector = Detector<ForestClassifier<FernClassifier>, 
		HaarFeaturesExtractor>(scanner, classifier, featuresExtractor);

	detector.learn(tmpl_img, tmpl_bb, true);

	vector<BB> objs;
	vector<float> probs;

	detector.detect(test_img, objs, probs);

	for_each(begin(objs), end(objs), [&test_img](BBRefC obj) {
		rectangle(test_img.img, obj, Scalar::all(255));
	});

	imshow("result", test_img.img);
	waitKey();

	return 0;
}

