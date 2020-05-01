
#pragma execution_character_set("utf-8")

#include "LabRun.hpp"

string LabRun::srcPath;

//ofstream LabRun::opt1;

bool LabRun::labIn(string src_path) {
	srcPath = src_path;

	FilesProcessUtils::createDir(srcPath + "/OTB_OPE");
	FilesProcessUtils::createDir(srcPath + "/OTB_TRE");
	//opt1.open(srcPath + "/otb_lab.csv");
	// opt1 << "编号" << "," << "数据名" << "," << "KCF(用时/秒)" << endl;

	return true;
}

bool LabRun::otb_lab(string otb_path) {
	double start, end, run_time; // 记录用时

	// 获取目录下全部文件夹名称
	LOG("[Get Directory Info]");
	vector<string> files = FilesProcessUtils::getFiles(otb_path);

	// 循环处理各个数据
	LOG("[Start Processing]");
	string filepath;
	for (int i = 0; i < files.size(); i++) {
		LOG(to_string(i) + ". " + files[i]);
		filepath = otb_path + "/OTB_LAB/" + files[i] + "/";
		FilesProcessUtils::createDir(filepath);

		// 读取数据
		OTBUtils::openDataset(otb_path + "/" + files[i]);
		LOG("a) Read data");

		// KCF
		ofstream optKCF;
		optKCF.open(filepath + "KCF.csv");
		optKCF << "Overall" << endl;
		optKCF << "Frame" << "," << "Distance" << "," << "OverlapSpace" << "," << "Times(s)" << endl;

		start = static_cast<double>(getTickCount());
		ObjTrackParam trackparamKCF(ObjTrackParam::KCF);
		OTBUtils::run(&trackparamKCF, optKCF);
		end = static_cast<double>(getTickCount());
		run_time = (end - start) / getTickFrequency();
		LOG("b) KCF, Cost Time" + to_string(run_time));

		optKCF.close();

		//BOOSTING
        ofstream optBOOSTING;
        optBOOSTING.open(filepath+"/BOOSTING.csv");
        optBOOSTING<<"Overall"<<endl;
        optBOOSTING<<"Frame"<<","<<"Distance"<<","<<"OverlapSpace"<<","<<"Times(s)"<<endl;
        
        start = static_cast<double>(getTickCount());
        ObjTrackParam trackparamBOOSTING(ObjTrackParam::BOOSTING);
        OTBUtils::run(&trackparamBOOSTING, optBOOSTING);
        end = static_cast<double>(getTickCount());
        run_time = (end - start) / getTickFrequency();
        LOG("c) BOOSTING, 用时"+to_string(run_time));
        
        optBOOSTING.close();
        
        //TLD
        ofstream optTLD;
        optTLD.open(filepath+"/TLD.csv");
        optTLD<<"Overall"<<endl;
        optTLD<<"Frame"<<","<<"Distance"<<","<<"OverlapSpace"<<","<<"Times(s)"<<endl;
        
        start = static_cast<double>(getTickCount());
        ObjTrackParam trackparamTLD(ObjTrackParam::TLD);
        OTBUtils::run(&trackparamTLD, optTLD);
        end = static_cast<double>(getTickCount());
        run_time = (end - start) / getTickFrequency();
        LOG("d) TLD, 用时"+to_string(run_time));
        
        optTLD.close();
        
        //MEDIANFLOW
        ofstream optMEDIANFLOW;
        optMEDIANFLOW.open(filepath+"/MEDIANFLOW.csv");
        optMEDIANFLOW<<"Overall"<<endl;
        optMEDIANFLOW<<"Frame"<<","<<"Distance"<<","<<"OverlapSpace"<<","<<"Times(s)"<<endl;
        
        start = static_cast<double>(getTickCount());
        ObjTrackParam trackparamMEDIANFLOW(ObjTrackParam::MEDIANFLOW);
        OTBUtils::run(&trackparamMEDIANFLOW, optMEDIANFLOW);
        end = static_cast<double>(getTickCount());
        run_time = (end - start) / getTickFrequency();
        LOG("e) MEDIANFLOW, 用时"+to_string(run_time));
        
        optMEDIANFLOW.close();
        
        //GOTURN
        /*ofstream optGOTURN;
        optGOTURN.open(filepath+"/GOTURN.csv");
        optGOTURN<<"Overall"<<endl;
        optGOTURN<<"Frame"<<","<<"Distance"<<","<<"OverlapSpace"<<","<<"Times(s)"<<endl;
        
        start = static_cast<double>(getTickCount());
        ObjTrackParam trackparamGOTURN(ObjTrackParam::GOTURN);
        OTBUtils::run(&trackparamGOTURN, optGOTURN);
        end = static_cast<double>(getTickCount());
        run_time = (end - start) / getTickFrequency();
        LOG("f) GOTURN, 用时"+to_string(run_time));
        
        optGOTURN.close();*/
	}

	return true;
}

bool LabRun::otb_lab_tre(string otb_path) {
	double start, end, run_time; //记录用时

	//获取目录下全部文件夹名称
	LOG("[Get Directory Info]");
	vector<string> files = FilesProcessUtils::getFiles(otb_path);

	//循环处理各个数据
	LOG("[Start Processing]");
	string filepath;
	for (int i = 0; i < files.size(); i++) {
		LOG(to_string(i) + ". " + files[i]);
		filepath = srcPath + "/OTB_TRE/" + files[i];
		FilesProcessUtils::createDir(filepath);

		//读取数据
		OTBUtils::openDataset(otb_path + "/" + files[i]);
		LOG("a) Reading Data");

		//KCF
		ofstream optKCF;
		optKCF.open(filepath + "/KCF.csv");
		LOG("b) KCF");
		for (int s = 0; s < 20; s++) {
			//optKCF<<"Overall"<<endl;
			//optKCF<<"Frame"<<","<<"Distance"<<","<<"OverlapSpace"<<","<<"Times(s)"<<endl;
			optKCF << "#" << endl;

			start = static_cast<double>(getTickCount());
			ObjTrackParam trackparamKCF(ObjTrackParam::KCF);
			OTBUtils::run(&trackparamKCF, optKCF, s);

			end = static_cast<double>(getTickCount());
			run_time = (end - start) / getTickFrequency();
			LOG("Slide: " << s << "  Time: " << (run_time));
		}

		optKCF << "##" << endl;

		optKCF.close();
		LOG("After Process");
		otb_after_tre(filepath + "/KCF.csv", filepath + "/KCF-after.csv");

		//BOOSTING
        ofstream optBOOSTING;
        optBOOSTING.open(filepath+"/BOOSTING.csv");
        LOG("c) BOOSTING");
        for(int s=0; s<20; s++){
            optBOOSTING<<"#"<<endl;
                
            start = static_cast<double>(getTickCount());
            ObjTrackParam trackparamBOOSTING(ObjTrackParam::BOOSTING);
            OTBUtils::run(&trackparamBOOSTING, optBOOSTING, s);
        
            end = static_cast<double>(getTickCount());
            run_time = (end - start) / getTickFrequency();
            LOG("时间段："+to_string(s)+" | 用时："+to_string(run_time));
        }
            
        optBOOSTING<<"##"<<endl;
            
        optBOOSTING.close();
        LOG("- 数据后处理");
        otb_after_tre(filepath+"/BOOSTING.csv", filepath+"/BOOSTING-after.csv");
        
        //TLD
        ofstream optTLD;
        optTLD.open(filepath+"/TLD.csv");
        LOG("d) TLD");
        for(int s=0; s<20; s++){
            optTLD<<"#"<<endl;
                
            start = static_cast<double>(getTickCount());
            ObjTrackParam trackparamTLD(ObjTrackParam::TLD);
            OTBUtils::run(&trackparamTLD, optTLD, s);
        
            end = static_cast<double>(getTickCount());
            run_time = (end - start) / getTickFrequency();
            LOG("时间段："+to_string(s)+" | 用时："+to_string(run_time));
        }
            
        optTLD<<"##"<<endl;
            
        optTLD.close();
        LOG("- 数据后处理");
        otb_after_tre(filepath+"/TLD.csv", filepath+"/TLD-after.csv");
        
        //MEDIANFLOW
        ofstream optMEDIANFLOW;
        optMEDIANFLOW.open(filepath+"/MEDIANFLOW.csv");
        LOG("e) MEDIANFLOW");
        for(int s=0; s<20; s++){
            optMEDIANFLOW<<"#"<<endl;
                
            start = static_cast<double>(getTickCount());
            ObjTrackParam trackparamMEDIANFLOW(ObjTrackParam::MEDIANFLOW);
            OTBUtils::run(&trackparamMEDIANFLOW, optMEDIANFLOW, s);
        
            end = static_cast<double>(getTickCount());
            run_time = (end - start) / getTickFrequency();
            LOG("时间段："+to_string(s)+" | 用时："+to_string(run_time));
        }
            
        optMEDIANFLOW<<"##"<<endl;
            
        optMEDIANFLOW.close();
        LOG("- 数据后处理");
        otb_after_tre(filepath+"/MEDIANFLOW.csv", filepath+"/MEDIANFLOW-after.csv");
        
        //GOTURN
        /*ofstream optGOTURN;
        optGOTURN.open(filepath+"/GOTURN.csv");
        LOG("f) GOTURN");
        for(int s=0; s<20; s++){
            optGOTURN<<"#"<<endl;
                
            start = static_cast<double>(getTickCount());
            ObjTrackParam trackparamGOTURN(ObjTrackParam::GOTURN);
            OTBUtils::run(&trackparamGOTURN, optGOTURN, s);
        
            end = static_cast<double>(getTickCount());
            run_time = (end - start) / getTickFrequency();
            LOG("时间段："+to_string(s)+" | 用时："+to_string(run_time));
        }
            
        optGOTURN<<"##"<<endl;
            
        optGOTURN.close();
        LOG("- 数据后处理");
        otb_after_tre(filepath+"/GOTURN.csv", filepath+"/GOTURN-after.csv");*/
	}



	return true;
}

bool LabRun::otb_after_tre(string inpath, string outpath) {
	ifstream in(inpath);

	string str = ".";

	int frame;
	double dis, os, time;
	vector<double> diss, oss, times;

	double d_threshold, d_ratio;
	vector<double> d_thresholds;
	vector<vector<double>> d_ratioss;

	double os_threshold, os_ratio;
	vector<double> os_thresholds;
	vector<vector<double>> os_ratioss;

	int num = 0;
	while (in.good()) {
		num++;
		//cout<<"num: "<<num<<endl;

		vector<double> d_ratios, os_ratios;

		while (str != "#") in >> str;

		if (num == 1) {
			while (in >> str, str != "#") {
				/*stringstream strstream(str);
				string a;

				getline(strstream, a, ',');
				frame = atoi(a.c_str());

				getline(strstream, a, ',');
				dis = atof(a.c_str());

				getline(strstream, a, ',');
				os = atof(a.c_str());

				getline(strstream, a, ',');
				time = atof(a.c_str());

				diss.push_back(dis);
				oss.push_back(os);
				times.push_back(time);*/
			}

			while (in >> str, str != "#") {
				stringstream strstream(str);
				string a;

				getline(strstream, a, ',');
				d_threshold = atof(a.c_str());

				getline(strstream, a, ',');
				d_ratio = atof(a.c_str());

				d_thresholds.push_back(d_threshold);
				d_ratios.push_back(d_ratio);
			}

			d_ratioss.push_back(d_ratios);

			while (in >> str, str != "#") {
				stringstream strstream(str);
				string a;

				getline(strstream, a, ',');
				os_threshold = atof(a.c_str());

				getline(strstream, a, ',');
				os_ratio = atof(a.c_str());

				os_thresholds.push_back(os_threshold);
				os_ratios.push_back(os_ratio);
			}

			os_ratioss.push_back(os_ratios);
		} else {
			while (in >> str, str != "#");

			while (in >> str, str != "#") {
				stringstream strstream(str);
				string a;

				getline(strstream, a, ',');
				d_threshold = atof(a.c_str());

				getline(strstream, a, ',');
				d_ratio = atof(a.c_str());

				d_ratios.push_back(d_ratio);
			}

			d_ratioss.push_back(d_ratios);

			while (in >> str, str != "#") {
				if (str == "##") break;
				stringstream strstream(str);
				string a;

				getline(strstream, a, ',');
				os_threshold = atof(a.c_str());

				getline(strstream, a, ',');
				os_ratio = atof(a.c_str());

				os_ratios.push_back(os_ratio);
			}

			os_ratioss.push_back(os_ratios);
			if (str == "##") break;
		}
	}
	in.close();

	//cout<<"Distance"<<endl;

	ofstream opt;
	opt.open(outpath);

	opt << "Distance" << endl;
	opt << "Threshold" << ",";
	for (int i = 0; i < num; i++) opt << "Ratio(" << i << "th)" << ",";
	opt << "Average" << endl;

	for (int i = 0; i < d_thresholds.size(); i++) {
		d_ratio = 0;

		opt << d_thresholds[i] << ",";
		for (int j = 0; j < num; j++) {
			opt << d_ratioss[j][i] << ",";
			d_ratio += d_ratioss[j][i];
		}
		opt << d_ratio / num << endl;
	}

	//cout<<"OverlapSpace"<<endl;
	opt << endl << "OverlapSpace" << endl;
	opt << "Threshold" << ",";
	for (int i = 0; i < num; i++) opt << "Ratio(" << i << "th)" << ",";
	opt << "Average" << endl;

	for (int i = 0; i < os_thresholds.size(); i++) {
		os_ratio = 0;

		opt << os_thresholds[i] << ",";
		for (int j = 0; j < num; j++) {
			opt << os_ratioss[j][i] << ",";
			os_ratio += d_ratioss[j][i];
		}
		opt << os_ratio / num << endl;
	}

	return true;
}

bool LabRun::otb_lab_sre(string otb_path){
    double start, end, run_time; //记录用时
    
    //获取目录下全部文件夹名称
    LOG("「获取目录信息」");
    vector<string> files= FilesProcessUtils::getFiles(otb_path);
    
    //循环处理各个数据
    LOG("「开始处理」");
    string filepath;
    for(int i=0; i<files.size(); i++){
        LOG(to_string(i)+". "+files[i]);
        filepath = srcPath+"/OTB_SRE/"+files[i];
        FilesProcessUtils::createDir(filepath);
        
        //读取数据
        OTBUtils::openDataset(otb_path+"/"+files[i]);
        LOG("a) 读取数据");
        
        //KCF
        ofstream optKCF;
        optKCF.open(filepath+"/KCF.csv");
        LOG("b) KCF");
        for(int s=0; s<13; s++){
            //optKCF<<"Overall"<<endl;
            //optKCF<<"Frame"<<","<<"Distance"<<","<<"OverlapSpace"<<","<<"Times(s)"<<endl;
            optKCF<<"#"<<endl;
            
            start = static_cast<double>(getTickCount());
            ObjTrackParam trackparamKCF(ObjTrackParam::KCF);
            OTBUtils::run(&trackparamKCF, optKCF, 0, s);
    
            end = static_cast<double>(getTickCount());
            run_time = (end - start) / getTickFrequency();
            LOG("空间变化："+to_string(s)+" | 用时："+to_string(run_time));
        }
        
        optKCF<<"##"<<endl;
        
        optKCF.close();
        LOG("- 数据后处理");
        otb_after_sre(filepath+"/KCF.csv", filepath+"/KCF-after.csv");
        
        //BOOSTING
        ofstream optBOOSTING;
        optBOOSTING.open(filepath+"/BOOSTING.csv");
        LOG("c) BOOSTING");
        for(int s=0; s<13; s++){
            optBOOSTING<<"#"<<endl;
                
            start = static_cast<double>(getTickCount());
            ObjTrackParam trackparamBOOSTING(ObjTrackParam::BOOSTING);
            OTBUtils::run(&trackparamBOOSTING, optBOOSTING, 0, s);
        
            end = static_cast<double>(getTickCount());
            run_time = (end - start) / getTickFrequency();
            LOG("空间变化："+to_string(s)+" | 用时："+to_string(run_time));
        }
            
        optBOOSTING<<"##"<<endl;
            
        optBOOSTING.close();
        LOG("- 数据后处理");
        otb_after_sre(filepath+"/BOOSTING.csv", filepath+"/BOOSTING-after.csv");
        
        //MEDIANFLOW
        ofstream optMEDIANFLOW;
        optMEDIANFLOW.open(filepath+"/MEDIANFLOW.csv");
        LOG("d) MEDIANFLOW");
        for(int s=0; s<13; s++){
            optMEDIANFLOW<<"#"<<endl;
                
            start = static_cast<double>(getTickCount());
            ObjTrackParam trackparamMEDIANFLOW(ObjTrackParam::MEDIANFLOW);
            OTBUtils::run(&trackparamMEDIANFLOW, optMEDIANFLOW, 0, s);
        
            end = static_cast<double>(getTickCount());
            run_time = (end - start) / getTickFrequency();
            LOG("空间变化："+to_string(s)+" | 用时："+to_string(run_time));
        }
            
        optMEDIANFLOW<<"##"<<endl;
            
        optMEDIANFLOW.close();
        LOG("- 数据后处理");
        otb_after_sre(filepath+"/MEDIANFLOW.csv", filepath+"/MEDIANFLOW-after.csv");
        
        //GOTURN
        /*ofstream optGOTURN;
        optGOTURN.open(filepath+"/GOTURN.csv");
        LOG("d) GOTURN");
        for(int s=0; s<13; s++){
            optGOTURN<<"#"<<endl;
                
            start = static_cast<double>(getTickCount());
            ObjTrackParam trackparamGOTURN(ObjTrackParam::GOTURN);
            OTBUtils::run(&trackparamGOTURN, optGOTURN, 0, s);
        
            end = static_cast<double>(getTickCount());
            run_time = (end - start) / getTickFrequency();
            LOG("空间变化："+to_string(s)+" | 用时："+to_string(run_time));
        }
            
        optGOTURN<<"##"<<endl;
            
        optGOTURN.close();
        LOG("- 数据后处理");
        otb_after_sre(filepath+"/GOTURN.csv", filepath+"/GOTURN-after.csv");*/
    }
    
    return true;
}

bool LabRun::otb_after_sre(string inpath, string outpath){
    ifstream in(inpath);
    
    string str=".";
    
    int frame;
    double dis, os, time;
    vector<double> diss, oss, times;
    
    double d_threshold, d_ratio;
    vector<double> d_thresholds;
    vector<vector<double>> d_ratioss;
    
    double os_threshold, os_ratio;
    vector<double> os_thresholds;
    vector<vector<double>> os_ratioss;
    
    int num=0;
    while(in.good()){
        num++;
        //cout<<"num: "<<num<<endl;
        
        vector<double> d_ratios, os_ratios;
        
        while(str!="#") in>>str;
        
        if(num==1){
            while(in>>str, str!="#"){
                stringstream strstream(str);
                string a;
            
                getline(strstream, a, ',');
                frame = atoi(a.c_str());
            
                getline(strstream, a, ',');
                dis = atof(a.c_str());
            
                getline(strstream, a, ',');
                os = atof(a.c_str());
            
                getline(strstream, a, ',');
                time = atof(a.c_str());

                diss.push_back(dis);
                oss.push_back(os);
                times.push_back(time);
            }
    
            while(in>>str, str!="#"){
                stringstream strstream(str);
                string a;
        
                getline(strstream, a, ',');
                d_threshold = atof(a.c_str());
        
                getline(strstream, a, ',');
                d_ratio = atof(a.c_str());
        
                d_thresholds.push_back(d_threshold);
                d_ratios.push_back(d_ratio);
            }
            
            d_ratioss.push_back(d_ratios);
    
            while(in>>str, str!="#"){
                stringstream strstream(str);
                string a;
            
                getline(strstream, a, ',');
                os_threshold = atof(a.c_str());
            
                getline(strstream, a, ',');
                os_ratio = atof(a.c_str());
            
                os_thresholds.push_back(os_threshold);
                os_ratios.push_back(os_ratio);
            }
            
            os_ratioss.push_back(os_ratios);
        }else{
            while(in>>str, str!="#"){
                stringstream strstream(str);
                string a;
                
                getline(strstream, a, ',');
                frame = atoi(a.c_str());
                
                getline(strstream, a, ',');
                dis = atof(a.c_str());
                
                getline(strstream, a, ',');
                os = atof(a.c_str());
                
                getline(strstream, a, ',');
                time = atof(a.c_str());

                diss.push_back(dis);
                oss.push_back(os);
                times.push_back(time);
            };
            
            while(in>>str, str!="#"){
                stringstream strstream(str);
                string a;
                
                getline(strstream, a, ',');
                d_threshold = atof(a.c_str());
                
                getline(strstream, a, ',');
                d_ratio = atof(a.c_str());
                
                d_ratios.push_back(d_ratio);
            }
                    
            d_ratioss.push_back(d_ratios);
            
            while(in>>str, str!="#"){
                if(str=="##") break;
                stringstream strstream(str);
                string a;
                    
                getline(strstream, a, ',');
                os_threshold = atof(a.c_str());
                    
                getline(strstream, a, ',');
                os_ratio = atof(a.c_str());
                    
                os_ratios.push_back(os_ratio);
            }
                    
            os_ratioss.push_back(os_ratios);
            if(str=="##") break;
        }
    }
    in.close();
    
    //cout<<"Distance"<<endl;
    
    ofstream opt;
    opt.open(outpath);
    
    opt<<"Distance"<<endl;
    opt<<"Threshold"<<","<<"Ratio(原)"<<",";
    opt<<"Ratio(上)"<<",""Ratio(右上)"<<","<<"Ratio(右)"<<","<<"Ratio(右下)"<<",";
    opt<<"Ratio(下)"<<",""Ratio(左下)"<<","<<"Ratio(左)"<<","<<"Ratio(左上)"<<",";
    opt<<"Ratio(80%)"<<",""Ratio(90%)"<<","<<"Ratio(110%)"<<","<<"Ratio(120%)"<<",";
    opt<<"平均"<<endl;
    
    for(int i=0; i<d_thresholds.size(); i++){
        d_ratio = 0;
        
        opt<<d_thresholds[i]<<",";
        for(int j=0; j<num; j++){
            opt<<d_ratioss[j][i]<<",";
            d_ratio += d_ratioss[j][i];
        }
        opt<<d_ratio/num<<endl;
    }
    
    //cout<<"OverlapSpace"<<endl;
    opt<<endl<<"OverlapSpace"<<endl;
    opt<<"Threshold"<<","<<"Ratio(原)"<<",";
    opt<<"Ratio(上)"<<",""Ratio(右上)"<<","<<"Ratio(右)"<<","<<"Ratio(右下)"<<",";
    opt<<"Ratio(下)"<<",""Ratio(左下)"<<","<<"Ratio(左)"<<","<<"Ratio(左上)"<<",";
    opt<<"Ratio(80%)"<<",""Ratio(90%)"<<","<<"Ratio(110%)"<<","<<"Ratio(120%)"<<",";
    opt<<"平均"<<endl;
    
    for(int i=0; i<os_thresholds.size(); i++){
        os_ratio = 0;
        
        opt<<os_thresholds[i]<<",";
        for(int j=0; j<num; j++){
            opt<<os_ratioss[j][i]<<",";
            os_ratio += d_ratioss[j][i];
        }
        opt<<os_ratio/num<<endl;
    }
    
    return true;
}
/*vector<string> LabRun::getFiles(string path) {
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
}*/
