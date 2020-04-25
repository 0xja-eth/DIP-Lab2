#pragma once

#include <QImage>
#include <QString>

#include <opencv2/videoio.hpp>

#include <thread>

#include "ImageProcess.h"
#include "opencv/mycv.h"

#include "Debug.h"

using namespace mycv;

// ��װ�� QImage �� ��Ƶ����
class MediaObject {
public:
	MediaObject() {};

	// ���ļ��ж�ȡ
	MediaObject(string filename, bool isVideo = false);
	MediaObject(QString filename, bool isVideo = false);

	// �� Mat �ж�ȡ
	MediaObject(Mat image);
	MediaObject(Mat* image);

	// ����Ƶ��������ж�ȡ��һ�㲻�ã�
	MediaObject(VideoCapture video);
	MediaObject(VideoCapture* video);

	// ����Ƶ�����ж�ȡ
	MediaObject(Mat* videoData, long length, double fps);

	~MediaObject();

	// ��ȡ
	void load(string filename, bool isVideo = false);
	void load(QString filename, bool isVideo = false);
	// ����
	void save(string filename);
	void save(QString filename);
	// �ͷ�
	void release();

	// �ж��Ƿ�ΪͼƬ
	bool isImage() const;
	// �ж��Ƿ�Ϊ��Ƶ
	bool isVideo() const;
	// �ж��Ƿ�Ϊ��
	bool isEmpty() const;

	// ��ȡ��ȡ����
	double getProgress();
	bool isLoading();

	// ��ȡ�ļ���
	string getFilename() const;

	// ��ȡ��Ӧ�� Mat ���ݣ��������Ƶ����ȡ�� num ֡���ݣ�
	Mat* getData(long num = 0) const;
	// ��ȡ��Ƶ�������һ�㲻�ã�
	VideoCapture* getVideoCap() const;

	// ��ȡ��Ƶ֡��
	long getVideoLength() const;
	// ��ȡ��Ƶ֡��
	double getVideoFPS() const;
	// ��ȡ��Ƶ���ݣ�Mat ���飬��Ҫ��֡�����ʹ�ã�
	Mat* getVideoData() const;
	// ��ȡ�� num ֡����Ƶ����
	Mat* getVideoData(long num) const;

	// ��ȡ��Ӧ�� QImage���������Ƶ����ȡ�� num ֡�� QImage��
	QImage getQImage(long num = 0) const;

private:
	static const int FOURCC;

	double progress = -1;

	string filename = ""; // �ļ���

	Mat* image = NULL; // ͼƬ����
	VideoCapture* videoCap = NULL; // ��Ƶ����

	Mat* videoData = NULL; // ��Ƶ����
	long frameCnt = 0; // ��Ƶ֡��
	double fps = 0; // ֡��

	void loadImage();
	void loadVideo();
	void loadVideoData();

	void saveImage(string filename);
	void saveVideo(string filename);
};

static class QTCVUtils {
public:
	typedef void(*ProcessFuncType0)( // ����������ĺ���ǩ��
		ProcessParam*);
	typedef Mat(*ProcessFuncType1)( // ����һ��ͼƬ�ĺ���ǩ��
		const Mat&, ProcessParam*);
	typedef Mat(*ProcessFuncType2)( // ��������ͼƬ����ֻ���һ��ͼƬ�ĺ���ǩ��
		const Mat&, const Mat&, ProcessParam*);
	typedef void(*ProcessFuncType3)( // ��������ͼƬ���������ͼƬ�ĺ���ǩ��
		const Mat&, const Mat&, Mat&, Mat&, ProcessParam*);
	typedef void(*ProcessFuncType4)( // ������Ƶ�������Ƶ�ĺ���ǩ��
		const Mat*, long, Mat*&, long&, ProcessParam*);

	static void update();

	static double progress();
	static bool isProcessing();
	static bool isProcessStatusChanged();

	static Mat qImage2Mat(const QImage& image);
	static QImage mat2QImage(const Mat& mat);
	static Mat qImage2Mat(const QImage* image);
	static QImage mat2QImage(const Mat* mat);

	// �첽����
	static void process(ProcessFuncType0 func, // ������
		ProcessParam* param = NULL); // ִ�����⺯��
	static void process(ProcessFuncType1 func, // ������
		const MediaObject* inImg, // ����ͼƬ
		MediaObject* &outImg, // ���ͼƬ
		ProcessParam* param = NULL); // ����
	static void process(ProcessFuncType2 func, // ������
		const MediaObject* inImg1, const MediaObject* inImg2, // ����ͼƬ
		MediaObject* &outImg, // ���ͼƬ
		ProcessParam* param = NULL); // ����
	static void process(ProcessFuncType3 func, // ������
		const MediaObject* inImg1, const MediaObject* inImg2, // ����ͼƬ
		MediaObject* &outImg1, MediaObject* &outImg2, // ���ͼƬ
		ProcessParam* param = NULL); // ����
	static void process(ProcessFuncType4 func, // ������
		const MediaObject* inVideo, // ������Ƶ
		MediaObject* &outVideo, // �����Ƶ
		ProcessParam* param = NULL); // ����

	// ͬ������
	static void processSync(ProcessFuncType1 func, // ������
		const MediaObject* inImg, // ����ͼƬ
		MediaObject* &outImg, // ���ͼƬ
		ProcessParam* param = NULL); // ����

	static std::thread* processThread;

private:
	static bool processing;
	static bool lastProcessed;
};

