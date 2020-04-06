#include <opencv2/imgproc/types_c.h>

#include "QTCVUtils.h"

const int MediaObject::FOURCC = VideoWriter::fourcc('M', 'J', 'P', 'G');

std::thread* QTCVUtils::processThread = NULL;
bool QTCVUtils::processing = false;
bool QTCVUtils::lastProcessed = false;

MediaObject::MediaObject(string filename, bool isVideo) {
	load(filename, isVideo);
}

MediaObject::MediaObject(QString filename, bool isVideo /*= false*/) {
	load(filename, isVideo);
}

MediaObject::MediaObject(Mat image) {
	release();
	this->image = new Mat(image);
}

MediaObject::MediaObject(Mat* image) {
	release();
	this->image = image;
}

MediaObject::MediaObject(VideoCapture video) {
	release();
	this->videoCap = new VideoCapture(video);
	loadVideoData();
}

MediaObject::MediaObject(VideoCapture* video) {
	release();
	this->videoCap = video;
	loadVideoData();
}

MediaObject::MediaObject(Mat* videoData, long length, double fps) {
	release();
	this->videoData = videoData;
	this->fps = fps;
	frameCnt = length;
}

MediaObject::~MediaObject() {
	release();
}

void MediaObject::load(string filename, bool isVideo) {
	if (!isEmpty()) release();
	this->filename = filename;
	try {
		progress = 0;
		if (isVideo) loadVideo();
		else loadImage();
		progress = 1;
	} catch (exception e) {
		LOG("读取媒体错误：" << e.what());
		release();
	}
}

void MediaObject::load(QString filename, bool isVideo) {
	load(filename.toStdString(), isVideo);
}

void MediaObject::loadImage() {
	image = new Mat(imread(filename));
}
void MediaObject::loadVideo() {
	videoCap = new VideoCapture(filename);
	loadVideoData();
}

void MediaObject::loadVideoData() {
	if (videoCap == NULL) return;

	frameCnt = videoCap->get(CAP_PROP_FRAME_COUNT);
	fps = videoCap->get(CAP_PROP_FPS);
	videoData = new Mat[frameCnt];
	// 读取每一帧数据
	long index = 0;
	while (videoCap->read(videoData[index])) {
		progress = index * 1.0 / frameCnt;
		if (++index >= frameCnt) break;
	}
}

void MediaObject::save(string filename) {
	if (isEmpty()) return;
	try {
		progress = 0;
		if (isVideo()) saveVideo(filename);
		else saveImage(filename);
		progress = 1;
	} catch (exception e) {
		LOG("保存媒体错误：" << e.what());
	}
}

void MediaObject::save(QString filename) {
	save(filename.toStdString());
}

void MediaObject::saveImage(string filename) {
	imwrite(filename, *image);
}

void MediaObject::saveVideo(string filename) {
	Mat* first = getVideoData(0);
	Size size(first->cols, first->rows);
	VideoWriter writer(filename, FOURCC, fps, size, 1);

	for (int i = 0; i < frameCnt; ++i) {
		auto data = getVideoData(i);
		progress = i * 1.0 / frameCnt;
		if (data == NULL || data->empty()) break;
		writer.write(*data);
	}
}

void MediaObject::release() {
	delete image, videoCap;
	delete[] videoData;

	filename = "";
	image = NULL;
	videoCap = NULL;
	videoData = NULL;
	frameCnt = 0;
}

bool MediaObject::isImage() const { 
	return image != NULL && !image->empty(); 
}
bool MediaObject::isVideo() const { 
	return videoData != NULL && frameCnt > 0; 
}

bool MediaObject::isEmpty() const { 
	return !isVideo() && !isImage(); 
}

double MediaObject::getProgress() { 
	return progress; 
}

bool MediaObject::isLoading() {
	return progress >= 0 && progress < 1;
}

string MediaObject::getFilename() const { return filename; }

Mat* MediaObject::getData(long num) const {
	if (isVideo()) return getVideoData(num);
	else if (isImage()) return image;
	else return NULL;
}

VideoCapture* MediaObject::getVideoCap() const { return videoCap; }

long MediaObject::getVideoLength() const { return frameCnt; }

double MediaObject::getVideoFPS() const { return fps; }

Mat* MediaObject::getVideoData() const { return videoData; }

Mat* MediaObject::getVideoData(long num) const {
	if (num >= getVideoLength()) return NULL;
	return &videoData[num];
}

QImage MediaObject::getQImage(long num) const {
	return QTCVUtils::mat2QImage(getData(num));
}

void QTCVUtils::update() {
	lastProcessed = processing;
}

double QTCVUtils::progress() {
	if (!processing) return 1;
	return ImageProcess::progress;
}

bool QTCVUtils::isProcessing() {
	return processing;
}

bool QTCVUtils::isProcessStatusChanged() {
	return processing != lastProcessed;
}

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

Mat QTCVUtils::qImage2Mat(const QImage* image) {
	if (image == NULL) return Mat();
	return qImage2Mat(*image);
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

QImage QTCVUtils::mat2QImage(const Mat* mat) {
	if (mat == NULL) return QImage();
	return mat2QImage(*mat);
}

/*
MediaObject* QTCVUtils::process(ProcessFuncType1 func,
	const MediaObject* img, ProcessParam* param) {

	auto data = img->getData();
	if (data == NULL) return new MediaObject();

	return new MediaObject(func(*data, param));
}

MediaObject* QTCVUtils::process(ProcessFuncType2 func,
	const MediaObject* img1, const MediaObject* img2,
	ProcessParam* param) {

	auto data1 = img1->getData(), data2 = img2->getData();
	if (data1 == NULL || data2 == NULL) return new MediaObject();

	return new MediaObject(func(*data1, *data2, param));
}

void QTCVUtils::process(ProcessFuncType3 func,
	const MediaObject* img1, const MediaObject* img2, 
	MediaObject* &out1, MediaObject* &out2,
	ProcessParam* param/) {

	auto data1 = img1->getData(), data2 = img2->getData();

	Mat outData1, outData2;
	func(*data1, *data2, outData1, outData2, param);

	out1 = new MediaObject(outData1);
	out2 = new MediaObject(outData2);
}

MediaObject* QTCVUtils::process(ProcessFuncType4 func,
	const MediaObject* video, ProcessParam* param ) {

	Mat* inData = video->getVideoData();
	long inLen = video->getVideoLength();

	Mat* outData; long outLen;
	func(inData, inLen, outData, outLen, param);

	return new MediaObject(outData, outLen);
}
*/

void QTCVUtils::process(ProcessFuncType1 func, /* 处理函数 */ 
	const MediaObject* inImg, /* 输入图片 */ 
	MediaObject* &outImg, /* 输出图片 */ 
	ProcessParam* param /*= NULL*/) {
	if (processing || inImg == NULL || inImg->isEmpty()) return;

	auto f = [&func, &inImg, &outImg, &param]() {
		processing = true;

		auto inData = inImg->getData();
		auto outData = func(*inData, param);
		outImg = new MediaObject(outData);

		processing = false;
	};
	processThread = new std::thread(f);
}

void QTCVUtils::process(ProcessFuncType2 func, /* 处理函数 */ 
	const MediaObject* inImg1, const MediaObject* inImg2, /* 输入图片 */ 
	MediaObject* &outImg, /* 输出图片 */ 
	ProcessParam* param /*= NULL*/) {
	if (processing || inImg1 == NULL || inImg2 == NULL || 
		inImg1->isEmpty() || inImg2->isEmpty()) return;

	auto f = [&func, &inImg1, &inImg2, &outImg, &param]() {
		processing = true;

		auto inData1 = inImg1->getData(), inData2 = inImg2->getData();
		auto outData = func(*inData1, *inData2, param);
		outImg = new MediaObject(outData);

		processing = false;
	};
	processThread = new std::thread(f);
}

void QTCVUtils::process(ProcessFuncType3 func, /* 处理函数 */ 
	const MediaObject* inImg1, const MediaObject* inImg2, /* 输入图片 */ 
	MediaObject* &outImg1, MediaObject* &outImg2, /* 输出图片 */ 
	ProcessParam* param /*= NULL*/) {
	if (processing || inImg1 == NULL || inImg2 == NULL ||
		inImg1->isEmpty() || inImg2->isEmpty()) return;

	auto f = [&func, &inImg1, &inImg2, &outImg1, &outImg2, &param]() {
		processing = true;

		auto inData1 = inImg1->getData(), inData2 = inImg2->getData();
		Mat outData1, outData2;
		func(*inData1, *inData2, outData1, outData2, param);

		outImg1 = new MediaObject(outData1);
		outImg2 = new MediaObject(outData2);

		processing = false;
	};
	processThread = new std::thread(f);
}

void QTCVUtils::process(ProcessFuncType4 func, /* 处理函数 */ 
	const MediaObject* inVideo, /* 输入视频 */ 
	MediaObject* &outVideo, /* 输出视频 */ 
	ProcessParam* param /*= NULL*/) {
	if (processing || inVideo == NULL || !inVideo->isVideo()) return;

	auto f = [&func, &inVideo, &outVideo, &param]() {
		processing = true;
		auto inData = inVideo->getVideoData();
		long inLen = inVideo->getVideoLength();
		double fps = inVideo->getVideoFPS();

		Mat* outData; long outLen;
		func(inData, inLen, outData, outLen, param);
		outVideo = new MediaObject(outData, outLen, fps);

		processing = false;
	};
	processThread = new std::thread(f);
}

void QTCVUtils::processSync(ProcessFuncType1 func, /* 处理函数 */
	const MediaObject* inImg, /* 输入图片 */ 
	MediaObject* &outImg, /* 输出图片 */ 
	ProcessParam* param /*= NULL*/) {
	if (inImg == NULL || inImg->isEmpty()) return;

	auto inData = inImg->getData();

	outImg = new MediaObject(func(*inData, param));
}
