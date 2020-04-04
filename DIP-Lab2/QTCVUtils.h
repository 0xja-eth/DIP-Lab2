#pragma once

#include <QImage>
#include <QString>

#include <opencv2/videoio.hpp>

#include "ImageProcess.h"
#include "opencv/mycv.h"

#include "Debug.h"

using namespace mycv;

// 封装了 QImage 和 视频的类
class MediaObject {
public:
	MediaObject() {};

	MediaObject(string filename, bool isVideo = false);
	MediaObject(QString filename, bool isVideo = false);

	MediaObject(Mat image);
	MediaObject(Mat* image);

	MediaObject(VideoCapture video);
	MediaObject(VideoCapture* video);
	MediaObject(Mat* videoData, long length);

	~MediaObject();

	void load(string filename, bool isVideo = false);
	void release();

	bool isImage() const;
	bool isVideo() const;
	bool isEmpty() const;

	string getFilename() const;

	Mat* getData() const;
	VideoCapture* getVideo() const;

	long getVideoLength() const;
	Mat* getVideoData() const;
	Mat* getVideoData(long num) const;

	QImage getQImage(long num = 0) const;

private:
	string filename = ""; // 文件名

	Mat* image = NULL; // 图片对象
	VideoCapture* video = NULL; // 视频对象

	Mat* videoData = NULL; // 视频数据
	long frameCnt = 0; // 视频帧数

	void loadImage();
	void loadVideo();
	void loadVideoData();
};

static class QTCVUtils {
public:
	typedef Mat(*ProcessFuncType1)( // 处理一张图片的函数签名
		const Mat&, const ProcessParam*);
	typedef Mat(*ProcessFuncType2)( // 处理两张图片，但只输出一张图片的函数签名
		const Mat&, const Mat&, const ProcessParam*);
	typedef void(*ProcessFuncType3)( // 处理两张图片，输出两张图片的函数签名
		const Mat&, const Mat&, Mat&, Mat&, const ProcessParam*);
	typedef void(*ProcessFuncType4)( // 处理视频，输出视频的函数签名
		const Mat*, long, Mat*&, long&, const ProcessParam*);

	static Mat qImage2Mat(const QImage& image);
	static QImage mat2QImage(const Mat& mat);
	static Mat qImage2Mat(const QImage* image);
	static QImage mat2QImage(const Mat* mat);

	static MediaObject* process(ProcessFuncType1 func, // 处理函数
		const MediaObject* img, // 输入图片
		const ProcessParam* param = NULL); // 参数
	static MediaObject* process(ProcessFuncType2 func, // 处理函数
		const MediaObject* img1, const MediaObject* img2, // 输入图片
		const ProcessParam* param = NULL); // 参数
	static void process(ProcessFuncType3 func, // 处理函数
		const MediaObject* img1, const MediaObject* img2, // 输入图片
		MediaObject* &out1, MediaObject* &out2, // 输出图片
		const ProcessParam* param = NULL); // 参数
	static MediaObject* process(ProcessFuncType4 func, // 处理函数
		const MediaObject* video, // 输入视频
		const ProcessParam* param = NULL); // 参数
};

