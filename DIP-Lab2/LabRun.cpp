
#pragma execution_character_set("utf-8")

#include "LabRun.hpp"

string LabRun::srcPath;

ofstream LabRun::opt1;

bool LabRun::labIn(string src_path) {
	srcPath = src_path;
	opt1.open(srcPath + "/otb_lab.csv");
	// opt1 << "编号" << "," << "数据名" << "," << "KCF(用时/秒)" << endl;

	return true;
}

bool LabRun::otb_lab(string otb_path) {
	double start, end, run_time; // 记录用时

	// 获取目录下全部文件夹名称
	LOG("[获取目录信息]");
	vector<string> files = getFiles(otb_path);

	// 循环处理各个数据
	LOG("[开始处理]");
	for (int i = 0; i < files.size(); i++) {
		LOG(to_string(i) + ". " + files[i]);
		opt1 << i << "," << files[i] << ",";

		//读取数据
		OTBUtils::openDataset(otb_path + "/" + files[i]);
		LOG("a) 读取数据");

		start = static_cast<double>(getTickCount());
		ObjTrackParam trackparamKCF(ObjTrackParam::KCF);
		OTBUtils::run(&trackparamKCF);
		end = static_cast<double>(getTickCount());
		run_time = (end - start) / getTickFrequency();
		LOG("b) KCF, 用时" + to_string(run_time));
		opt1 << run_time << ",";

		opt1 << endl;
	}

	opt1.close();

	return true;
}

vector<string> LabRun::getFiles(string path) {
	vector<string> files;//存放文件名

#ifdef WIN32
	path = path + "\\*";
	_finddata_t fileinfo;
	long long lf;
	//输入文件夹路径
	if ((lf = _findfirst(path.c_str(), &fileinfo)) == -1) {
		LOG("找不到此目录");
	} else {
		while (_findnext(lf, &fileinfo) == 0) {
			if ((fileinfo.attrib & _A_SUBDIR)) {
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					files.push_back(fileinfo.name);
			}
		}
		_findclose(lf);
	}
#endif


#ifdef __APPLE__
	struct dirent *dirp;
	DIR* dir = opendir(path.data());

	while ((dirp = readdir(dir)) != nullptr) {
		if (dirp->d_type == DT_DIR) {
			files.push_back(dirp->d_name);
		}
	}

	closedir(dir);
#endif

	sort(files.begin(), files.end());
	return files;
}
