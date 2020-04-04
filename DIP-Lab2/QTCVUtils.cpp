#include <opencv2/imgproc/types_c.h>

#include "QTCVUtils.h"

Mat QTCVUtils::qImage2Mat(const QImage& image) {
	Mat mat;
	switch (image.format()) {
	case QImage::Format_ARGB32:
	case QImage::Format_RGB32:
	case QImage::Format_ARGB32_Premultiplied:
		mat = Mat(image.height(), image.width(), CV_8UC4, (void*)image.bits(), image.bytesPerLine());
		break;
	case QImage::Format_RGB888:
		mat = Mat(image.height(), image.width(), CV_8UC3, (void*)image.bits(), image.bytesPerLine());
		cvtColor(mat, mat, CV_BGR2RGB);
		break;
	case QImage::Format_Indexed8:
		mat = Mat(image.height(), image.width(), CV_8UC1, (void*)image.bits(), image.bytesPerLine());
		break;
	}

	return mat;
}

QImage QTCVUtils::mat2QImage(const Mat& mat) {
	// 8-bits unsigned, NO. OF CHANNELS = 1

	if (mat.type() == CV_8UC1) {
		QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
		// Set the color table (used to translate colour indexes to qRgb values)
		image.setColorCount(256);
		for (int i = 0; i < 256; i++) {
			image.setColor(i, qRgb(i, i, i));
		}
		// Copy input Mat
		uchar *pSrc = mat.data;
		for (int row = 0; row < mat.rows; row++) {
			uchar *pDest = image.scanLine(row);
			memcpy(pDest, pSrc, mat.cols);
			pSrc += mat.step;
		}
		return image;
	}
	// 8-bits unsigned, NO. OF CHANNELS = 3
	else if (mat.type() == CV_8UC3) {
		// Copy input Mat
		const uchar *pSrc = (const uchar*)mat.data;
		// Create QImage with same dimensions as input Mat
		QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
		return image.rgbSwapped();
	} else if (mat.type() == CV_8UC4) {
		// Copy input Mat
		const uchar *pSrc = (const uchar*)mat.data;
		// Create QImage with same dimensions as input Mat
		QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
		return image.copy();
	} else {
		return QImage();
	}
}

QImage QTCVUtils::process(ProcessFuncType1 func, /* 处理函数 */ 
	const QImage &img, /* 输入图片 */ 
	const ProcessParam* param /*= NULL 参数*/) {

	return mat2QImage(func(qImage2Mat(img), param));
}

QImage QTCVUtils::process(ProcessFuncType2 func, /* 处理函数 */ 
	const QImage &img1, const QImage &img2, /* 输入图片 */ 
	const ProcessParam* param /*= NULL 参数*/) {

	auto data1 = qImage2Mat(img1), data2 = qImage2Mat(img2);

	return mat2QImage(func(data1, data2, param));
}

void QTCVUtils::process(ProcessFuncType3 func, /* 处理函数 */ 
	const QImage &img1, const QImage &img2, /* 输入图片 */ 
	QImage &out1, QImage &out2, /* 输出图片 */ 
	const ProcessParam* param /*= NULL 参数*/) {

	auto data1 = qImage2Mat(img1), data2 = qImage2Mat(img2);

	Mat outData1, outData2;
	func(data1, data2, outData1, outData2, param);

	out1 = mat2QImage(outData1);
	out2 = mat2QImage(outData2);
}
