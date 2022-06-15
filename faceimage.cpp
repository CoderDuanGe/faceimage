#include "faceimage.h"
#include "ui_faceimage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QBuffer>
#include <QJsonArray>
#include <QThread>
#include <QPainter>


FaceImage::FaceImage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FaceImage)
{
    ui->setupUi(this);
    cameraInfoList=QCameraInfo::availableCameras();
    for(const QCameraInfo &tmpCam:cameraInfoList){
        qDebug()<<tmpCam.deviceName()<<"|||"<<tmpCam.description();
        ui->comboBox->addItem(tmpCam.description());
    }

    connect(ui->comboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&FaceImage::pickCamera);


    //设置摄像头功能
    camera=new QCamera();
    finder=new QCameraViewfinder();

    finder->setFixedSize(300,500);

    imageCapture=new QCameraImageCapture(camera);
    camera->setViewfinder(finder);
    camera->setCaptureMode(QCamera::CaptureStillImage);
    imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);

    connect(imageCapture,&QCameraImageCapture::imageCaptured,this,&FaceImage::showCamera);
    connect(ui->pushButton,&QPushButton::clicked,this,&FaceImage::prePostData);
    camera->start();

    //进行布局

    this->resize(1200,700);
    QVBoxLayout *vboxl=new QVBoxLayout;
    vboxl->addWidget(ui->label);
    vboxl->addWidget(ui->pushButton);

    QVBoxLayout *vboxr=new QVBoxLayout;
    vboxr->addWidget(ui->comboBox);
    vboxr->addWidget(finder);
    vboxr->addWidget(ui->textBrowser);

    QHBoxLayout *hbox=new QHBoxLayout(this);
    hbox->addLayout(vboxl);
    hbox->addLayout(vboxr);

    this->setLayout(hbox);

    //定时器来不断的刷新拍照界面
    refreshTimer=new QTimer(this);
    connect(refreshTimer,&QTimer::timeout,this,&FaceImage::takePicture);
    refreshTimer->start(20);

    //利用定时器来不断进行人脸识别的请求
    netTimer=new QTimer(this);
    connect(netTimer,&QTimer::timeout,this,&FaceImage::prePostData);

    tokenManger=new QNetworkAccessManager(this);
    connect(tokenManger,&QNetworkAccessManager::finished,this,&FaceImage::tokenReply);

    imgManger=new QNetworkAccessManager(this);
    connect(imgManger,&QNetworkAccessManager::finished,this,&FaceImage::imgReply);

    qDebug()<<tokenManger->supportedSchemes();


    //拼接url
    QUrl url("https://aip.baidubce.com/oauth/2.0/token");

    QUrlQuery query;
    query.addQueryItem("grant_type","client_credentials");
    query.addQueryItem("client_id","SwOpae0NeKQBrAZ9b6Bqg5DN");
    query.addQueryItem("client_secret","sk72obbhU9qoLdbGGEcHhZRjykLapTAI");

    url.setQuery(query);
    qDebug()<<url;

    //ssl支持
    if(QSslSocket::supportsSsl()){
        qDebug()<<"支持ssl";

    }else{
        qDebug()<<"不支持ssl";
    }

    //ssl配置
    sslConfig=QSslConfiguration::defaultConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::QueryPeer);
    sslConfig.setProtocol(QSsl::TlsV1_2);

    //组装请求
    QNetworkRequest req;
    req.setUrl(url);
    req.setSslConfiguration(sslConfig);

    //发送get请求
    tokenManger->get(req);

}


void FaceImage::showCamera(int id,QImage img)
{
    Q_UNUSED(id);
    this->img=img;

    //绘制人脸框
    QPainter p(&img);
    p.setPen(Qt::red);
    p.drawRect(faceLeft,faceTop,faceWidth,faceHeight);

    QFont font=p.font();
    font.setPixelSize(30);
    p.setFont(font);

    p.drawText(faceLeft+faceWidth+5,faceTop,QString("年龄:").append(QString::number(age)));

    p.drawText(faceLeft+faceWidth+5,faceTop+40,QString("性别:").append(gender));

    p.drawText(faceLeft+faceWidth+5,faceTop+80,QString("戴口罩:").append(mask==0?"没戴口罩":"戴了口罩"));

    p.drawText(faceLeft+faceWidth+5,faceTop+120,QString("颜值:").append(QString::number(beauty)));

    ui->label->setPixmap(QPixmap::fromImage(img));
}

void FaceImage::takePicture()
{
    imageCapture->capture();
}

void FaceImage::tokenReply(QNetworkReply *reply){
    //错误处理
    if(reply->error()!=QNetworkReply::NoError){
        qDebug()<<reply->errorString();
        return;
    }

    //正常应答
    const QByteArray reply_data=reply->readAll();

    //json解析
    QJsonParseError jsonErr;
    QJsonDocument doc=QJsonDocument::fromJson(reply_data,&jsonErr);

    //解析成功
    if(jsonErr.error==QJsonParseError::NoError){
        QJsonObject obj=doc.object();
        if(obj.contains("access_token")){
            accessToken=obj.take("access_token").toString();
        }
        ui->textBrowser->setText(accessToken);

    }else{
        qDebug()<<"json err:"<<jsonErr.errorString();
    }

    reply->deleteLater();


    //prePostData();
}

void FaceImage::prePostData()
{
    //创建子线程
    //创建工人
    //把工人送进子线程
    //绑定信号和槽的关系
    //启动子线程
    //给工人发通知干活

    childThread=new QThread(this);
    Work *worker=new Work;
    worker->moveToThread(childThread);

    connect(this,&FaceImage::beginWork,worker,&Work::doWork);
    connect(worker,&Work::resultReady,this,&FaceImage::beginFaceDetect);
    connect(childThread,&QThread::finished,worker,&QObject::deleteLater);

    childThread->start();

    emit beginWork(img,childThread);
}

void FaceImage::pickCamera(int index)
{
    qDebug()<<cameraInfoList.at(index).description();

    refreshTimer->stop();
    camera->stop();

    camera=new QCamera(cameraInfoList.at(index));
    imageCapture=new QCameraImageCapture(camera);
    connect(imageCapture,&QCameraImageCapture::imageCaptured,this,&FaceImage::showCamera);
    imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
    camera->setCaptureMode(QCamera::CaptureStillImage);
    camera->setViewfinder(finder);

    camera->start();
    refreshTimer->start(10);

}


void FaceImage::beginFaceDetect(QByteArray postData,QThread *overThread)
{ 
    //关闭子线程
    //组装图像识别请求
    //用post发送数据给百度的API

    overThread->exit();
    overThread->wait();

    //组装图像识别请求
    QUrl url("https://aip.baidubce.com/rest/2.0/face/v3/detect");

    QUrlQuery query;
    query.addQueryItem("access_token",accessToken);
    url.setQuery(query);

    //组装请求
    QNetworkRequest req;
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));
    req.setUrl(url);
    req.setSslConfiguration(sslConfig);
    imgManger->post(req,postData);

}

void FaceImage::imgReply(QNetworkReply *reply)
{
    if(reply->error()!=QNetworkReply::NoError){
        qDebug()<<reply->errorString();
        return ;
    }

    const QByteArray replyData=reply->readAll();
    qDebug()<<replyData;

    QJsonParseError jsonErr;
    QJsonDocument doc=QJsonDocument::fromJson(replyData,&jsonErr);

    if(jsonErr.error==QJsonParseError::NoError){

        QString faceInfo;

        //取出最外层的json
        QJsonObject  obj=doc.object();
        if(obj.contains("timestamp")){
            int tmpTime=obj.take("timestamp").toInt();
            if(tmpTime<latestTime){
                return;
            }else{
                latestTime=tmpTime;
            }
        }


        if(obj.contains("result")){
            QJsonObject resultObj=obj.take("result").toObject();

            //取出人脸的列表
            if(resultObj.contains("face_list")){
                QJsonArray faceList=resultObj.take("face_list").toArray();

                //取出第一张人脸
                QJsonObject faceObj=faceList.at(0).toObject();


                //取出人脸定位信息
                if(faceObj.contains("location")){
                    QJsonObject locObj=faceObj.take("location").toObject();
                    if(locObj.contains("left")){
                        faceLeft=locObj.take("left").toDouble();
                    }
                    if(locObj.contains("top")){
                        faceTop=locObj.take("top").toDouble();
                    }
                    if(locObj.contains("width")){
                        faceWidth=locObj.take("width").toDouble();
                    }
                    if(locObj.contains("height")){
                        faceHeight=locObj.take("height").toDouble();
                    }

                }

                //取出年龄
                if(faceObj.contains("age")){
                    age=faceObj.take("age").toDouble();
                    faceInfo.append("年龄").append(QString::number(age)).append("\r\n");
                }


                //取出性别
                if(faceObj.contains("gender")){
                    QJsonObject genderObj=faceObj.take("gender").toObject();
                    if(genderObj.contains("type")){
                        gender=genderObj.take("type").toString();
                        faceInfo.append("性别").append(gender).append("\r\n");
                    }
                }

                //取出表情
                if(faceObj.contains("emotion")){
                    QJsonObject emotionObj=faceObj.take("emotion").toObject();
                    if(emotionObj.contains("type")){
                        emotion=emotionObj.take("type").toString();
                        faceInfo.append("表情").append(emotion).append("\r\n");
                    }
                }

                //是否戴口罩
                if(faceObj.contains("mask")){
                    QJsonObject maskObj=faceObj.take("mask").toObject();
                    if(maskObj.contains("type")){
                        mask=maskObj.take("type").toInt();
                        faceInfo.append("口罩").append(mask==0?"没戴口罩":"戴了口罩").append("\r\n");
                    }
                }

                //取出颜值
                if(faceObj.contains("beauty")){
                    beauty=faceObj.take("beauty").toDouble();
                    faceInfo.append("颜值").append(QString::number(beauty)).append("\r\n");
                }


            }
            ui->textBrowser->setText(faceInfo);

        }else{
            qDebug()<<"json err:"<<jsonErr.errorString();
        }
    }

    reply->deleteLater();
    netTimer->start(1500);

    //prePostData();
}




FaceImage::~FaceImage()
{
    delete ui;
}
