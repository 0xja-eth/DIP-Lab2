#pragma once

#include <QImage>
#include <QString>

#include <opencv2/videoio.hpp>

#include <thread>

#include "ImageProcess.h"
#include "opencv/mycv.h"

#include "Debug.h"

using namespace mycv;

// 封装了 QImage 和 视频的类
class MediaObject {
public:
	MediaObject() {};

	// 从文件中读取
	MediaObject(string filename, bool isVideo = false);
	MediaObject(QString filename, bool isVideo = false);

	// 从 Mat 中读取
	MediaObject(Mat image);
	MediaObject(Mat* image);

	// 从视频输入对象中读取（一般不用）
	MediaObject(VideoCapture video);
	MediaObject(VideoCapture* video);

	// 从视频数据中读取
	MediaObject(Mat* videoData, long length, double fps);

	~MediaObject();

	// 读取
	void load(string filename, bool isVideo = false);
	void load(QString filename, bool isVideo = false);
	// 保存
	void save(string filename);
	void save(QString filename);
	// 释放
	void release();

	// 判断是否为图片
	bool isImage() const;
	// 判断是否为视频
	bool isVideo() const;
	// 判断是否为空
	bool isEmpty() const;

	// 获取读取进度
	double getProgress();
	bool isLoading();

	// 获取文件名
	string getFilename() const;

	// 获取对应的 Mat 数据（如果是视频，获取第 num 帧数据）
	Mat* getData(long num = 0) const;
	// 获取视频输入对象（一般不用）
	VideoCapture* getVideoCap() const;

	// 获取视频帧数
	long getVideoLength() const;
	// 获取视频帧率
	double getVideoFPS() const;
	// 获取视频数据（Mat 数组，需要和帧数结合使用）
	Mat* getVideoData() const;
	// 获取第 num 帧的视频数据
	Mat* getVideoData(long num) const;

	// 获取对应的 QImage（如果是视频，获取第 num 帧的 QImage）
	QImage getQImage(long num = 0) const;

private:
	static const int FOURCC;

	double progress = -1;

	string filename = ""; // 文件名

	Mat* image = NULL; // 图片对象
	VideoCapture* videoCap = NULL; // 视频对象

	Mat* videoData = NULL; // 视频数据
	long frameCnt = 0; // 视频帧数
	double fps = 0; // 帧率

	void loadImage();
	void loadVideo();
	void loadVideoData();

	void saveImage(string filename);
	void saveVideo(string filename);
};

static class QTCVUtils {
public:
	typedef void(*ProcessFuncType0)( // 仅传入参数的函数签名
		ProcessParam*);
	typedef Mat(*ProcessFuncType1)( // 处理一张图片的函数签名
		const Mat&, ProcessParam*);
	typedef Mat(*ProcessFuncType2)( // 处理两张图片，但只输出一张图片的函数签名
		const Mat&, const Mat&, ProcessParam*);
	typedef void(*ProcessFuncType3)( // 处理两张图片，输出两张图片的函数签名
		const Mat&, const Mat&, Mat&, Mat&, ProcessParam*);
	typedef void(*ProcessFuncType4)( // 处理视频，输出视频的函数签名
		const Mat*, long, Mat*&, long&, ProcessParam*);

	static void update();

	static double progress();
	static bool isProcessing();
	static bool isProcessStatusChanged();

	static Mat qImage2Mat(const QImage& image);
	static QImage mat2QImage(const Mat& mat);
	static Mat qImage2Mat(const QImage* image);
	static QImage mat2QImage(const Mat* mat);

	// 异步处理
	static void process(ProcessFuncType0 func, // 处理函数
		ProcessParam* param = NULL); // 执行任意函数
	static void process(ProcessFuncType1 func, // 处理函数
		const MediaObject* inImg, // 输入图片
		MediaObject* &outImg, // 输出图片
		ProcessParam* param = NULL); // 参数
	static void process(ProcessFuncType2 func, // 处理函数
		const MediaObject* inImg1, const MediaObject* inImg2, // 输入图片
		MediaObject* &outImg, // 输出图片
		ProcessParam* param = NULL); // 参数
	static void process(ProcessFuncType3 func, // 处理函数
		const MediaObject* inImg1, const MediaObject* inImg2, // 输入图片
		MediaObject* &outImg1, MediaObject* &outImg2, // 输出图片
		ProcessParam* param = NULL); // 参数
	static void process(ProcessFuncType4 func, // 处理函数
		const MediaObject* inVideo, // 输入视频
		MediaObject* &outVideo, // 输出视频
		ProcessParam* param = NULL); // 参数

	// 同步处理
	static void processSync(ProcessFuncType1 func, // 处理函数
		const MediaObject* inImg, // 输入图片
		MediaObject* &outImg, // 输出图片
		ProcessParam* param = NULL); // 参数

	static std::thread* processThread;

private:
	static bool processing;
	static bool lastProcessed;
};

