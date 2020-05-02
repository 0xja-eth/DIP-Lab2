#include "DIPLab2.h"
#include <QtWidgets/QApplication>

#define CV_MALLOC_ALIGN 16

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	DIPLab2 w;
	w.show();
	return a.exec();
}
