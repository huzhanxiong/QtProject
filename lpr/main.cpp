#include "lpr.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/wali.ico"));
    
    lpr w;
    w.show();

    return a.exec();
}
