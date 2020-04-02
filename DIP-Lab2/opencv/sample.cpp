// 一些直接执行的例子

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

	//*********************************** 1.加载人脸检测器  ******************************
	// 建立级联分类器
	CascadeClassifier cascade;
	// 加载训练好的 人脸检测器（.xml）
	const string path = "./xml/haarcascade_frontalface_alt.xml";
	if (!cascade.load(path)) {
		cout << "cascade load failed!\n";
	}

	//计时
	double t = 0;
	t = (double)getTickCount();
	//*********************************** 2.人脸检测 ******************************
	vector<Rect> faces(0);
	cascade.detectMultiScale(img, faces, 1.1, 2, 0, Size(30, 30));

	cout << "detect face number is :" << faces.size() << endl;
	//********************************  3.显示人脸矩形框 ******************************

	if (faces.size() > 0) {
		for (size_t i = 0; i < faces.size(); i++) {
			rectangle(img, faces[i], Scalar(150, 0, 0), 3, 8, 0);

		}
	} else cout << "未检测到人脸" << endl;

	t = (double)getTickCount() - t;  //getTickCount():  Returns the number of ticks per second.
	cout << "检测人脸用时：" << t * 1000 / getTickFrequency() << "ms (不计算加载模型的时间）" << endl;

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
	//1.生成训练集结构体对象指针
	Ptr<TrainData> data_set = TrainData::loadFromCSV("agaricus-lepiota.data",//文件名
		0,//第0行略过
		0,
		1,//区间[0,1)为存储了响应列
		"cat[0-22]",//0-22行均为类别数据
		',',//数据间的分隔符号为","
		'?');//丢失数据用"?"表示

	//2.验证数据读取的正确性
	int n_samples = data_set->getNSamples();
	if (n_samples == 0) {
		cerr << "Could not read file: mushroom.data" << endl;
		exit(-1);
	} else {
		cout << "Read " << n_samples << " samples from mushroom.data" << endl;
	}

	//3.分割训练集和测试集，比例为9:1，不打乱数据集的顺序
	data_set->setTrainTestSplitRatio(0.90, false);
	int n_train_samples = data_set->getNTrainSamples();
	int n_test_samples = data_set->getNTestSamples();
	Mat trainMat = data_set->getTrainSamples();
	//4.决策树
	//4.1 创建
	Ptr<RTrees> dtree = RTrees::create();
	//4.2 参数设置
	dtree->setMaxDepth(8); //树的最大深度
	dtree->setMinSampleCount(10); //节点样本数的最小值
	dtree->setRegressionAccuracy(0.01f);
	dtree->setUseSurrogates(false);//是否允许使用替代分叉点处理丢失的数据
	dtree->setMaxCategories(15);//决策树的最大预分类数量
	dtree->setCVFolds(0);//如果 CVFolds>1 那么就使用k-fold交叉修建决策树 其中k=CVFolds
	dtree->setUse1SERule(true);//True 表示使用更大力度的修剪，这会导致树的规模更小，但准确性更差，用于解决过拟合问题
	dtree->setTruncatePrunedTree(true);//是否删掉被减枝的部分
	float _priors[] = { 1.0,10.0 };
	Mat priors(1, 2, CV_32F, _priors);
	dtree->setPriors(priors);//为所有的答案设置权重
	//4.3 训练
	dtree->train(data_set);
	//4.4 计算训练误差
	Mat results;
	float train_performance = dtree->calcError(data_set,
		false,//true 表示使用测试集  false 表示使用训练集
		results);

	vector<String> names;
	data_set->getNames(names);

	Mat flags = data_set->getVarSymbolFlags();

	//5 训练集的结果分析
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

	//6 测试集的结果分析
	float test_performance = dtree->calcError(data_set, true, results);
	cout << "Performance on training data: " << (1-train_performance)*100 << "%" << endl;
	cout << "Performance on test data: " << (1-test_performance)*100 << "%" << endl;

	//6 训练集的结果分析
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

	//保存
	dtree->save("dtree_01.xml");

	system("pause");

	return 0;
}
*/
/*
int main() {
	//建立蘑菇分类的随机森林
 //1.构建训练集和测试集
	Ptr<TrainData> data_set = TrainData::loadFromCSV("agaricus-lepiota.data",//文件名
		0,//第0行略过
		0,
		1,//区间[0,1)为存储了响应列
		"cat[0-22]",//0-22行均为类别数据
		',',//数据间的分隔符号为","
		'?');//丢失数据用"?"表示
//2.验证数据读取的正确性
	int n_samples = data_set->getNSamples();
	if (n_samples == 0) {
		cerr << "Could not read file: mushroom.data" << endl;
		exit(-1);
	} else {
		cout << "Read " << n_samples << " samples from mushroom.data" << endl;
	}

	//3.分割训练集和测试集，比例为9:1，打乱数据集的顺序
	data_set->setTrainTestSplitRatio(0.90, true);
	int n_train_samples = data_set->getNTrainSamples();
	int n_test_samples = data_set->getNTestSamples();

	//4.随机森林
	Ptr<RTrees> forest_mushroom = RTrees::create();
	forest_mushroom->setMaxDepth(10); //树的最大深度
	forest_mushroom->setRegressionAccuracy(0.01f);//设置回归精度
	forest_mushroom->setMinSampleCount(10);//节点的最小样本数量
	forest_mushroom->setMaxCategories(15);//最大预分类数
	forest_mushroom->setCalculateVarImportance(true);//计算变量的重要性
	forest_mushroom->setActiveVarCount(4);//树节点随机选择特征子集的大小
	forest_mushroom->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER +
		TermCriteria::EPS, 100, 0.01));//终止标准
//训练模型
	forest_mushroom->train(data_set);
	//计算训练集和测试集的误差
	float correct_Train_answer = 0;
	float correct_Test_answer = 0;
	//1.训练集
	Mat trainSample = data_set->getTrainSamples();
	Mat trainResMat = data_set->getTrainResponses();
	for (int i = 0; i < trainSample.rows; i++) {
		Mat sample = trainSample.row(i);
		float r = forest_mushroom->predict(sample);
		r = fabs((float)r - trainResMat.at<float>(i)) <= FLT_EPSILON ? 1 : 0;
		correct_Train_answer += r;
	}
	float r1 = correct_Train_answer / n_train_samples;
	//2.测试集
	Mat testSample = data_set->getTestSamples();
	Mat testResMat = data_set->getTestResponses();
	for (int i = 0; i < testSample.rows; i++) {
		Mat sample = testSample.row(i);
		float r = forest_mushroom->predict(sample);
		r = fabs((float)r - testResMat.at<float>(i)) <= FLT_EPSILON ? 1 : 0;
		correct_Test_answer += r;
	}
	float r2 = correct_Test_answer / n_test_samples;
	//3.输出结果
	cout << "trainSet Accuracy: " << r1 * 100 << "%" << endl;
	cout << "testSet Accuracy:  " << r2 * 100 << "%" << endl;
	//4.保存模型
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

