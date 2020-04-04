#pragma once

#include <QImage>

#include "ImageProcess.h"
#include "opencv/mycv.h"

using namespace cv;
using namespace mycv;

static class QTCVUtils {
public:
	typedef Mat(*ProcessFuncType1)(
		const Mat&, const ProcessParam*);
	typedef Mat(*ProcessFuncType2)(
		const Mat&, const Mat&, const ProcessParam*);
	typedef void(*ProcessFuncType3)(
		const Mat&, const Mat&, Mat&, Mat&, const ProcessParam*);

	static Mat qImage2Mat(const QImage& image);
	static QImage mat2QImage(const Mat& mat);

	static QImage process(ProcessFuncType1 func, // 处理函数
		const QImage &img, // 输入图片
		const ProcessParam* param = NULL); // 参数
	static QImage process(ProcessFuncType2 func, // 处理函数
		const QImage &img1, const QImage &img2, // 输入图片
		const ProcessParam* param = NULL); // 参数
	static void process(ProcessFuncType3 func, // 处理函数
		const QImage &img1, const QImage &img2, // 输入图片
		QImage &out1, QImage &out2, // 输出图片
		const ProcessParam* param = NULL); // 参数
};

