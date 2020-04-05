#include <opencv2/imgproc/types_c.h>

#include "QTCVUtils.h"

const int MediaObject::FOURCC = VideoWriter::fourcc('M', 'J', 'P', 'G');

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

MediaObject::MediaObject(Mat* videoData, long length) {
	release();
	this->videoData = videoData;
	frameCnt = length;
}

MediaObject::~MediaObject() {
	release();
}

void MediaObject::load(string filename, bool isVideo) {
	if (!isEmpty()) release();
	this->filename = filename;
	try {
		if (isVideo) loadVideo();
		else loadImage();
	} catch (exception e) {
		LOG("��ȡý�����" << e.what());
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
	// ��ȡÿһ֡����
	long index = 0;
	while (videoCap->read(videoData[index]))
		if (++index >= frameCnt) break;
}

void MediaObject::save(string filename) {
	if (isEmpty()) return;
	try {
		if (isVideo()) saveVideo(filename);
		else saveImage(filename);
	} catch (exception e) {
		LOG("����ý�����" << e.what());
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

MediaObject* QTCVUtils::process(ProcessFuncType1 func, /* ������ */
	const MediaObject* img, /* ����ͼƬ */ ProcessParam* param /*= NULL ����*/) {

	auto data = img->getData();
	if (data == NULL) return new MediaObject();

	return new MediaObject(func(*data, param));
}

MediaObject* QTCVUtils::process(ProcessFuncType2 func, /* ������ */
	const MediaObject* img1, const MediaObject* img2, /* ����ͼƬ */
	ProcessParam* param /*= NULL ����*/) {

	auto data1 = img1->getData(), data2 = img2->getData();
	if (data1 == NULL || data2 == NULL) return new MediaObject();

	return new MediaObject(func(*data1, *data2, param));
}

void QTCVUtils::process(ProcessFuncType3 func, /* ������ */ 
	const MediaObject* img1, const MediaObject* img2, /* ����ͼƬ */
	MediaObject* &out1, MediaObject* &out2, /* ���ͼƬ */
	ProcessParam* param /*= NULL ����*/) {

	auto data1 = img1->getData(), data2 = img2->getData();

	Mat outData1, outData2;
	func(*data1, *data2, outData1, outData2, param);

	out1 = new MediaObject(outData1);
	out2 = new MediaObject(outData2);
}

MediaObject* QTCVUtils::process(ProcessFuncType4 func, /* ������ */ 
	const MediaObject* video, /* ������Ƶ */ ProcessParam* param /*= NULL ����*/) {

	Mat* inData = video->getVideoData();
	long inLen = video->getVideoLength();

	Mat* outData; long outLen;
	func(inData, inLen, outData, outLen, param);

	return new MediaObject(outData, outLen);
}
