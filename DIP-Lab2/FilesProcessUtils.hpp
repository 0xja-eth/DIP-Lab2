#ifdef WIN32
#include <direct.h>
#include <io.h>
#endif

#ifdef __APPLE__
#include <dirent.h>
#include <stdarg.h>
#include <sys/stat.h>
#endif

#include <fstream>
#include <vector>
using namespace std;

static class FilesProcessUtils{
public:
    static vector<string> getFiles(string path); //获取目录下文件名
    static void createDir(string path); //创建文件夹
};
