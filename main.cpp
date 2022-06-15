#include "faceimage.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FaceImage w;
    w.show();
    return a.exec();
}
