#ifndef FACEIMAGE_H
#define FACEIMAGE_H

#include <QWidget>
#include <QCamera>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QUrlQuery>
#include <QSslConfiguration>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include "work.h"
#include <QCameraInfo>
#include <QComboBox>

QT_BEGIN_NAMESPACE
namespace Ui { class FaceImage; }
QT_END_NAMESPACE

class FaceImage : public QWidget
{
    Q_OBJECT
signals:
    void beginWork(QImage img,QThread *childThread);

public:
    FaceImage(QWidget *parent = nullptr);
    ~FaceImage();

public slots:
    void showCamera(int id,QImage preview);
    void takePicture();
    void tokenReply(QNetworkReply *reply);
    void beginFaceDetect(QByteArray postData,QThread *overThread);
    void imgReply(QNetworkReply *reply);
    void prePostData();
    void pickCamera(int index);

private:
    Ui::FaceImage *ui;
    QCamera *camera;
    QCameraViewfinder *finder;
    QCameraImageCapture *imageCapture;
    QTimer *refreshTimer;
    QTimer *netTimer;
    QNetworkAccessManager *tokenManger;
    QNetworkAccessManager *imgManger;
    QSslConfiguration sslConfig;
    QString accessToken;
    QImage img;

    QThread *childThread;

    double faceTop;
    double faceWidth;
    double faceLeft;
    double faceHeight;

    double age;
    QString gender;
    QString emotion;
    int mask;
    double beauty;

    int latestTime;


    QList<QCameraInfo> cameraInfoList;
};
#endif // FACEIMAGE_H
