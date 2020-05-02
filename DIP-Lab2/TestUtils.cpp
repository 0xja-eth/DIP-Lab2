#include "TestUtils.h"

const std::string TestUtils::OtbRectSpecFile = "groundtruth_rect.txt";

const std::string TestUtils::VotRectSpecFile = "groundtruth.txt";

const std::string TestUtils::ImagesDir = "img/";

const double TestUtils::MaxDistanceTreshold = 50;

const double TestUtils::MaxOSTreshold = 1;

const double TestUtils::DeltaTreshold = 0.01;

bool TestUtils::showImg = false;

std::string TestUtils::format = "%04d.jpg";

std::string TestUtils::path = "";

Mat* TestUtils::frames = NULL;

vector<cv::Rect> TestUtils::truthRects;

void TestUtils::openOtbDataset(string path, string format) {
	TestUtils::path = path;
	TestUtils::format = format;

	auto rectFile = path + "/" + OtbRectSpecFile;
	auto imgDir = path + "/" + ImagesDir;

	_loadRects(rectFile); _loadFrames(imgDir);
}

void TestUtils::openVotDataset(string path, string format /*= "%04d.jpg"*/) {

	TestUtils::path = path;
	TestUtils::format = format;

	auto rectFile = path + "/" + VotRectSpecFile;
	auto imgDir = path + "/";

}

void TestUtils::run(ProcessParam *param_) {
	auto param = (ObjTrackParam*)param_;
	ofstream opt(path + "/CustomOTBResult.csv");
	run(param, opt);
}

void TestUtils::run(ObjTrackParam *param, ofstream &opt, int frames_num, int rect_type) {
	long start_frame = frames_num*truthRects.size()/20;

	// ���?��ȡÿ֡������
	Rect2d *detRects; double *dists, *oss;
	_runDetect(detRects, dists, oss, param, opt, start_frame, rect_type);

	OutTable distRates, osRates;
	long len = truthRects.size() - start_frame;

	for (double t = 0; t < 1; t += DeltaTreshold) {

		ImageProcess::progress = 0.9 + t / 1 * 0.1;

		auto distT = t * MaxDistanceTreshold;
		auto osT = t * MaxOSTreshold;

		double distRate, osRate;
		long distCnt = 0, osCnt = 0;
		for (int i = 1; i < len; ++i) {
			double dist = dists[i], os = oss[i];

			if (dist <= distT) distCnt++;
			if (os >= osT) osCnt++;
		}

		if (len > 1) {
			distRate = distCnt * 1.0 / (len - 1);
			osRate = osCnt * 1.0 / (len - 1);
		} else distRate = osRate = 0;

		distRates[distT] = distRate;
		osRates[osT] = osRate;
	}

	//opt << "Distance" << endl;
	//opt << "Treshold" << "," << "Ratio" << endl;
	opt << "#" << endl;
	_saveToFile(distRates, opt);

	//opt << "OverlapSpace" << endl;
	//opt << "Treshold" << "," << "Ratio" << endl;
	opt << "#" << endl;
	_saveToFile(osRates, opt);
}

void TestUtils::_saveToFile(OutTable out, ofstream &opt) {
	OutTable::iterator oit = out.begin();
	for (; oit != out.end(); ++oit) {
		auto pair = *oit;
		opt << pair.first << "," << pair.second << endl;
	}
}

void TestUtils::_runDetect(Rect2d* &rects, double* &dists, double* &oss, 
	ObjTrackParam *param, ofstream &opt, long start_frame, int rect_type) {

	long len = truthRects.size() - start_frame;
	rects = new Rect2d[len];
	dists = new double[len];
	oss = new double[len];

	if (param->algo == ObjTrackParam::STRUCK)
		__runSTRUCKDetect(rects, dists, oss, param, opt, start_frame, rect_type);
	else if (param->algo == ObjTrackParam::GOTURN)
		__runGOTURNDetect(rects, dists, oss, param, opt, start_frame, rect_type);
	else
		__runStdDetect(rects, dists, oss, param, opt, start_frame, rect_type);
}

void TestUtils::__runStdDetect(Rect2d* &rects, double* &dists, double* &oss,
	ObjTrackParam *param, ofstream &opt, long start_frame, int rect_type) {
	double start, end, run_time;

	bool newDet = true;
	Ptr<cv::Tracker> tracker = NULL;

	long len = truthRects.size() - start_frame;

	for (int i = 0; i < len; ++i) {
		ImageProcess::progress = i * 1.0 / len * 0.9;
		start = static_cast<double>(getTickCount());

		auto truth = truthRects[i+start_frame];
		auto frame = frames[i+start_frame];

		if (i == 0) {
			param->setRect(rects[i] = _initRect(truth, rect_type));
			ImageProcess::doObjTrack(frame, tracker, newDet, param);
			continue;
		}

		auto det = ImageProcess::doObjTrack(frame, tracker, newDet, param);

		// ƥ�䲻��
		if (newDet) det = Rect2d();
		rects[i] = det;

		double dist = dists[i] = __calcDistance(det, truth);
		double os = oss[i] = __calcOS(det, truth);

		end = static_cast<double>(getTickCount());
		run_time = (end-start)/getTickFrequency();

		opt<<i+start_frame+1<<","<<dist<<","<<os<<","<<run_time<<endl;

		// չʾ
		if (!showImg) continue;

		__showProcessingImage(dist, os, frame, det, param, truth);
	}
}

void TestUtils::__runSTRUCKDetect(Rect2d* &rects, double* &dists, double* &oss, 
	ObjTrackParam *param, ofstream &opt, long start_frame, int rect_type) {
	double start, end, run_time;

	Config conf; ::Tracker tracker(conf);

	long len = truthRects.size() - start_frame;

	for (int i = 0; i < len; ++i) {
		ImageProcess::progress = i * 1.0 / len * 0.9;
		start = static_cast<double>(getTickCount());

		auto truth = truthRects[i + start_frame];
		auto frame = frames[i + start_frame];

		if (i == 0) {
			Mat scaledFrame;

			float scaleW = (float)conf.frameWidth / frame.cols;
			float scaleH = (float)conf.frameHeight / frame.rows;

			auto rect = _initRect(truth, rect_type);
			param->setRect(rects[i] = rect);

			resize(frame, scaledFrame, Size(conf.frameWidth, conf.frameHeight));
			FloatRect initBB = FloatRect(rect.x*scaleW, rect.y*scaleH,
				rect.width*scaleW, rect.height*scaleH);

			tracker.Initialise(scaledFrame, initBB);

			continue;
		}

		auto det = rects[i] = ImageProcess::doObjTrack(frame, tracker);

		double dist = dists[i] = __calcDistance(det, truth);
		double os = oss[i] = __calcOS(det, truth);

		end = static_cast<double>(getTickCount());
		run_time = (end - start) / getTickFrequency();

		opt << i + start_frame + 1 << "," << dist << "," << os << "," << run_time << endl;

		// չʾ
		if (!showImg) continue;

		__showProcessingImage(dist, os, frame, det, param, truth);
	}
}

void TestUtils::__runGOTURNDetect(Rect2d* &rects, double* &dists, double* &oss,
	ObjTrackParam *param, ofstream &opt, long start_frame, int rect_type) {
	double start, end, run_time;

	auto tracker = ImageProcess::createGOTURN();

	long len = truthRects.size() - start_frame;
	Rect2d det; // ��һ֡���

	for (int i = 0; i < len; ++i) {
		ImageProcess::progress = i * 1.0 / len * 0.9;
		start = static_cast<double>(getTickCount());

		auto truth = truthRects[i + start_frame];

		if (i == 0) {
			auto rect = _initRect(truth, rect_type);
			param->setRect(det = rects[i] = rect);
			continue;
		}

		auto frame = frames[i + start_frame];
		auto prevFrame = frames[i + start_frame - 1];

		det = rects[i] = ImageProcess::doObjTrack(
			prevFrame, frame, tracker, det);

		double dist = dists[i] = __calcDistance(det, truth);
		double os = oss[i] = __calcOS(det, truth);

		end = static_cast<double>(getTickCount());
		run_time = (end - start) / getTickFrequency();

		opt << i + start_frame + 1 << "," << dist << "," << os << "," << run_time << endl;

		// չʾ
		if (!showImg) continue;

		__showProcessingImage(dist, os, frame, det, param, truth);
	}
}

void TestUtils::__showProcessingImage(double dist, double os,
	Mat &frame, Rect2d &det, ObjTrackParam * param, cv::Rect &truth) {
	Mat drawFrame = frame.clone();

	char showText[256];

	sprintf(showText, "Dist: %.4f, OS: %.4f", dist, os);

	putText(drawFrame, showText, Point(20, 20),
		FONT_HERSHEY_SIMPLEX, 0.75, Scalar(255, 255, 255), 2);

	rectangle(drawFrame, det, param->color, 2);
	rectangle(drawFrame, truth, Scalar(255, 0, 0), 2);

	imshow("OTBTest", drawFrame);

	waitKey(1);
}

cv::Rect TestUtils::_initRect(cv::Rect orig, int rect_type){
    int x = frames[0].rows; int y = frames[0].cols;    

	switch(rect_type){
        case 0:
            return orig;
        case 1:
            return cv::Rect(orig.x, max(int(orig.y-orig.height*0.1), int(orig.height/2)), orig.width, orig.height);
        case 2:
            return cv::Rect(min(int(orig.x+orig.width*0.07), int(x-orig.width/2)), max(int(orig.y-orig.height*0.07),int(orig.height/2)), orig.width, orig.height);
        case 3:
            return cv::Rect(min(int(orig.x+orig.width*0.1), int(x-orig.width/2)), orig.y, orig.width, orig.height);
        case 4:
            return cv::Rect(min(int(orig.x+orig.width*0.07),int(x-orig.width/2)), min(int(orig.y+orig.height*0.07),int(y-orig.height/2)), orig.width, orig.height);
        case 5:
            return cv::Rect(orig.x, min(int(orig.y+orig.height*0.1), int(y-orig.height/2)), orig.width, orig.height);
        case 6:
            return cv::Rect(max(int(orig.x-orig.width*0.07), int(orig.width/2)), min(int(orig.y+orig.height*0.07),int(y-orig.height/2)), orig.width, orig.height);
        case 7:
            return cv::Rect(max(int(orig.x-orig.width*0.1),int(orig.width/2)), orig.y, orig.width, orig.height);
        case 8:
            return cv::Rect(max(int(orig.x-orig.width*0.07),int(orig.width/2)), max(int(orig.y-orig.height*0.07),int(orig.height/2)), orig.width, orig.height);
        case 9:
            return cv::Rect(orig.x, orig.y, orig.width*0.8, orig.height*0.8);
        case 10:
            return cv::Rect(orig.x, orig.y, orig.width*0.9, orig.height*0.9);
        case 11:
            return cv::Rect(min(orig.x,int(x-orig.width*1.1/2)), min(orig.y,int(y-orig.height*1.1/2)), orig.width*1.1, orig.height*1.1);
        default:
            return cv::Rect(min(orig.x,int(x-orig.width*1.2/2)), min(orig.y,int(y-orig.height*1.2/2)), orig.width*1.2, orig.height*1.2);
    }
    
    return orig;
}

//void OTBUtils::_calcEvaluation(Rect2d* rects, double* &dists, double* &oss) {
//
//	long len = truthRects.size();
//	dists = new double[len];
//	oss = new double[len];
//	
//	for (int i = 0; i < len; ++i) {
//		auto det = rects[i];
//		auto truth = truthRects[i];
//
//		dists[i] = __calcDistance(det, truth);
//		oss[i] = __calcOS(det, truth);
//
//		LOG(i << ". det:" << det.x << "," << det.y << ","
//			<< det.width << "," << det.height
//			<< " truth:" << truth.x << "," << truth.y << ","
//			<< truth.width << "," << truth.height <<
//			" Dist: " << dists[i] << " OS: " << oss[i]);
//	}
//}


void TestUtils::_loadRects(string filename) {
	truthRects.clear();

	ifstream file(filename, ios::in); // ���ı�ģʽ��in.txt����
	if (!file) { //��ʧ��
		LOG("Error opening source file."); return;
	}

	int x, y, w, h; char ch;
	while (file >> x) {
		file >> ch >> y; file >> ch >> w; file >> ch >> h;
		truthRects.push_back(cv::Rect(x, y, w, h));
	}
}

void TestUtils::_loadFrames(string path) {
	long len = truthRects.size();

	if (frames != NULL) {
		delete[] frames; frames = NULL;
	}

	frames = new Mat[len];
	for (int i = 0; i < len; ++i) {
		ImageProcess::progress = i * 1.0 / len;

		char name_[12];
		sprintf(name_, format.c_str(), i+1);
		string filename(name_);
		filename = path + filename;

		frames[i] = imread(filename);
	}
}

double TestUtils::__calcDistance(Rect2d dist, Rect2d truth) {
	double dx = dist.x + (dist.width / 2);
	double dy = dist.y + (dist.height / 2);
	double tx = truth.x + (truth.width / 2);
	double ty = truth.y + (truth.height / 2);
	return sqrt((tx - dx)*(tx - dx) + (ty - dy)*(ty - dy));
}

double TestUtils::__calcOS(Rect2d dist, Rect2d truth) {
	auto cross = __calcCrossSpace(dist, truth);
	return __calcCrossSpace(dist, truth) / __calcMergeSpace(dist, truth, cross);
}

double TestUtils::__calcCrossSpace(Rect2d dist, Rect2d truth) {
	double dx = dist.x, dy = dist.y,
		dw = dist.width, dh = dist.height;
	double tx = truth.x, ty = truth.y,
		tw = truth.width, th = truth.height;

	double minX = max(dx, tx), maxX = min(dx + dw, tx + tw);
	double minY = max(dy, ty), maxY = min(dy + dh, ty + th);

	double width = maxX - minX, height = maxY - minY;
	if (width <= 0 || height <= 0) return 0;
	return width * height;
}

double TestUtils::__calcMergeSpace(Rect2d dist, Rect2d truth, double cross) {
	return dist.area() + truth.area() - cross;
}
