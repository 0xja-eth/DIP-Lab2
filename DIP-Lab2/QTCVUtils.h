#pragma once

#include <QImage>
#include <QString>

#include <opencv2/videoio.hpp>

#include "ImageProcess.h"
#include "opencv/mycv.h"

#include "Debug.h"

using namespace mycv;

// ��װ�� QImage �� ��Ƶ����
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
	string filename = ""; // �ļ���

	Mat* image = NULL; // ͼƬ����
	VideoCapture* video = NULL; // ��Ƶ����

	Mat* videoData = NULL; // ��Ƶ����
	long frameCnt = 0; // ��Ƶ֡��

	void loadImage();
	void loadVideo();
	void loadVideoData();
};

static class QTCVUtils {
public:
	typedef Mat(*ProcessFuncType1)( // ����һ��ͼƬ�ĺ���ǩ��
		const Mat&, const ProcessParam*);
	typedef Mat(*ProcessFuncType2)( // ��������ͼƬ����ֻ���һ��ͼƬ�ĺ���ǩ��
		const Mat&, const Mat&, const ProcessParam*);
	typedef void(*ProcessFuncType3)( // ��������ͼƬ���������ͼƬ�ĺ���ǩ��
		const Mat&, const Mat&, Mat&, Mat&, const ProcessParam*);
	typedef void(*ProcessFuncType4)( // ������Ƶ�������Ƶ�ĺ���ǩ��
		const Mat*, long, Mat*&, long&, const ProcessParam*);

	static Mat qImage2Mat(const QImage& image);
	static QImage mat2QImage(const Mat& mat);
	static Mat qImage2Mat(const QImage* image);
	static QImage mat2QImage(const Mat* mat);

	static MediaObject* process(ProcessFuncType1 func, // ������
		const MediaObject* img, // ����ͼƬ
		const ProcessParam* param = NULL); // ����
	static MediaObject* process(ProcessFuncType2 func, // ������
		const MediaObject* img1, const MediaObject* img2, // ����ͼƬ
		const ProcessParam* param = NULL); // ����
	static void process(ProcessFuncType3 func, // ������
		const MediaObject* img1, const MediaObject* img2, // ����ͼƬ
		MediaObject* &out1, MediaObject* &out2, // ���ͼƬ
		const ProcessParam* param = NULL); // ����
	static MediaObject* process(ProcessFuncType4 func, // ������
		const MediaObject* video, // ������Ƶ
		const ProcessParam* param = NULL); // ����
};

