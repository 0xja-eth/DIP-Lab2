#pragma once

#include <QImage>

#include "ImageProcess.h"
#include "opencv/mycv.h"

using namespace cv;
using namespace mycv;

static class QTCVUtils {
public:
	typedef Mat(*ProcessFunc1)(
		const Mat&, const ProcessParam*);
	typedef Mat(*ProcessFunc2)(
		const Mat&, const Mat&, const ProcessParam*);
	typedef void(*ProcessFunc3)(
		const Mat&, const Mat&, Mat&, Mat&, const ProcessParam*);

	static Mat qImage2Mat(const QImage& image);
	static QImage mat2QImage(const Mat& mat);

	static QImage process(ProcessFunc1 func, // ������
		const QImage &img, // ����ͼƬ
		const ProcessParam* param = NULL); // ����
	static QImage process(ProcessFunc2 func, // ������
		const QImage &img1, const QImage &img2, // ����ͼƬ
		const ProcessParam* param = NULL); // ����
	static void process(ProcessFunc3 func, // ������
		const QImage &img1, const QImage &img2, // ����ͼƬ
		QImage &out1, QImage &out2, // ���ͼƬ
		const ProcessParam* param = NULL); // ����
};

