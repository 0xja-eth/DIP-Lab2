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
	MediaObject(Mat* videoData, long length);

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
	typedef Mat(*ProcessFuncType1)( // ����һ��ͼƬ�ĺ���ǩ��
		const Mat&, ProcessParam*);
	typedef Mat(*ProcessFuncType2)( // ��������ͼƬ����ֻ���һ��ͼƬ�ĺ���ǩ��
		const Mat&, const Mat&, ProcessParam*);
	typedef void(*ProcessFuncType3)( // ��������ͼƬ���������ͼƬ�ĺ���ǩ��
		const Mat&, const Mat&, Mat&, Mat&, ProcessParam*);
	typedef void(*ProcessFuncType4)( // ������Ƶ�������Ƶ�ĺ���ǩ��
		const Mat*, long, Mat*&, long&, ProcessParam*);

	static double progress() { return ImageProcess::progress; }

	static Mat qImage2Mat(const QImage& image);
	static QImage mat2QImage(const Mat& mat);
	static Mat qImage2Mat(const QImage* image);
	static QImage mat2QImage(const Mat* mat);

	static MediaObject* process(ProcessFuncType1 func, // ������
		const MediaObject* img, // ����ͼƬ
		ProcessParam* param = NULL); // ����
	static MediaObject* process(ProcessFuncType2 func, // ������
		const MediaObject* img1, const MediaObject* img2, // ����ͼƬ
		ProcessParam* param = NULL); // ����
	static void process(ProcessFuncType3 func, // ������
		const MediaObject* img1, const MediaObject* img2, // ����ͼƬ
		MediaObject* &out1, MediaObject* &out2, // ���ͼƬ
		ProcessParam* param = NULL); // ����
	static MediaObject* process(ProcessFuncType4 func, // ������
		const MediaObject* video, // ������Ƶ
		ProcessParam* param = NULL); // ����
};

