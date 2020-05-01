#include "OTBUtils.h"

const std::string OTBUtils::RectSpecFile = "groundtruth_rect.txt";

const std::string OTBUtils::ImagesDir = "img/";

const double OTBUtils::MaxDistanceTreshold = 50;

const double OTBUtils::MaxOSTreshold = 1;

const double OTBUtils::DeltaTreshold = 0.01;

bool OTBUtils::showImg = false;

std::string OTBUtils::format = "%04d.jpg";

std::string OTBUtils::path = "";

Mat* OTBUtils::frames = NULL;

vector<cv::Rect> OTBUtils::truthRects;

void OTBUtils::openDataset(string path, string format) {
	OTBUtils::path = path;
	OTBUtils::format = format;

	auto rectFile = path + "/" + RectSpecFile;
	auto imgDir = path + "/" + ImagesDir;

	_loadRects(rectFile); _loadFrames(imgDir);
}

<<<<<<< HEAD
void OTBUtils::run(ProcessParam *param_, ofstream &opt, int frames_num, int rect_type) {
=======
void OTBUtils::run(ProcessParam *param_) {
>>>>>>> 3ea4f2e53bfdea354dafead7beb5bc7565dfe4de
	auto param = (ObjTrackParam*)param_;
	ofstream opt(path + "/CustomOTBResult.csv");
	run(param, opt);
}

void OTBUtils::run(ObjTrackParam *param, ofstream &opt, int frames_num) {
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

void OTBUtils::_saveToFile(OutTable out, ofstream &opt) {
	OutTable::iterator oit = out.begin();
	for (; oit != out.end(); ++oit) {
		auto pair = *oit;
		opt << pair.first << "," << pair.second << endl;
	}
}

<<<<<<< HEAD
void OTBUtils::_runDetect(Rect2d* &rects, double* &dists, double* &oss, ObjTrackParam *param, ofstream &opt, long start_frame, int rect_type) {
	double start, end, run_time;

	bool newDet = true;
	Ptr<Tracker> tracker = NULL;
=======
void OTBUtils::_runDetect(Rect2d* &rects, double* &dists, double* &oss,
	ObjTrackParam *param, ofstream &opt, long start_frame) {
>>>>>>> 3ea4f2e53bfdea354dafead7beb5bc7565dfe4de

	long len = truthRects.size() - start_frame;
	rects = new Rect2d[len];
	dists = new double[len];
	oss = new double[len];

	if (param->algo == ObjTrackParam::STRUCK)
		__runSTRUCKDetect(rects, dists, oss, param, opt, start_frame);
	else
		__runStdDetect(rects, dists, oss, param, opt, start_frame);
}

void OTBUtils::__runStdDetect(Rect2d* &rects, double* &dists, double* &oss,
	ObjTrackParam *param, ofstream &opt, long start_frame) {
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
			param->setRect(rects[i] = _initRect(truth, rect_type)));
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
}

void OTBUtils::__runSTRUCKDetect(Rect2d* &rects, double* &dists, double* &oss, 
	ObjTrackParam *param, ofstream &opt, long start_frame) {
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

			resize(frame, scaledFrame, Size(conf.frameWidth, conf.frameHeight));
			FloatRect initBB = FloatRect(truth.x*scaleW, truth.y*scaleH,
				truth.width*scaleW, truth.height*scaleH);
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
}

Rect OTBUtils::_initRect(Rect orig, int rect_type){
    int x = frames[0].rows; int y = frames[0].cols;
    switch(rect_type){
        case 0:
            return orig;
        case 1:
            return Rect(orig.x, max(int(orig.y-orig.height*0.1), int(orig.height/2)), orig.width, orig.height);
        case 2:
            return Rect(min(int(orig.x+orig.width*0.07), int(x-orig.width/2)), max(int(orig.y-orig.height*0.07),int(orig.height/2)), orig.width, orig.height);
        case 3:
            return Rect(min(int(orig.x+orig.width*0.1), int(x-orig.width/2)), orig.y, orig.width, orig.height);
        case 4:
            return Rect(min(int(orig.x+orig.width*0.07),int(x-orig.width/2)), min(int(orig.y+orig.height*0.07),int(y-orig.height/2)), orig.width, orig.height);
        case 5:
            return Rect(orig.x, min(int(orig.y+orig.height*0.1), int(y-orig.height/2)), orig.width, orig.height);
        case 6:
            return Rect(max(int(orig.x-orig.width*0.07), int(orig.width/2)), min(int(orig.y+orig.height*0.07),int(y-orig.height/2)), orig.width, orig.height);
        case 7:
            return Rect(max(int(orig.x-orig.width*0.1),int(orig.width/2)), orig.y, orig.width, orig.height);
        case 8:
            return Rect(max(int(orig.x-orig.width*0.07),int(orig.width/2)), max(int(orig.y-orig.height*0.07),int(orig.height/2)), orig.width, orig.height);
        case 9:
            return Rect(orig.x, orig.y, orig.width*0.8, orig.height*0.8);
        case 10:
            return Rect(orig.x, orig.y, orig.width*0.9, orig.height*0.9);
        case 11:
            return Rect(min(orig.x,int(x-orig.width*1.1/2)), min(orig.y,int(y-orig.height*1.1/2)), orig.width*1.1, orig.height*1.1);
        default:
            return Rect(min(orig.x,int(x-orig.width*1.2/2)), min(orig.y,int(y-orig.height*1.2/2)), orig.width*1.2, orig.height*1.2);
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


void OTBUtils::_loadRects(string filename) {
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

void OTBUtils::_loadFrames(string path) {
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

double OTBUtils::__calcDistance(Rect2d dist, Rect2d truth) {
	double dx = dist.x + (dist.width / 2);
	double dy = dist.y + (dist.height / 2);
	double tx = truth.x + (truth.width / 2);
	double ty = truth.y + (truth.height / 2);
	return sqrt((tx - dx)*(tx - dx) + (ty - dy)*(ty - dy));
}

double OTBUtils::__calcOS(Rect2d dist, Rect2d truth) {
	auto cross = __calcCrossSpace(dist, truth);
	return __calcCrossSpace(dist, truth) / __calcMergeSpace(dist, truth, cross);
}

double OTBUtils::__calcCrossSpace(Rect2d dist, Rect2d truth) {
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

double OTBUtils::__calcMergeSpace(Rect2d dist, Rect2d truth, double cross) {
	return dist.area() + truth.area() - cross;
}
