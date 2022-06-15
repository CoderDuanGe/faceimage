#include "work.h"
#include <QBuffer>
#include <QJsonObject>
#include <QJsonDocument>

Work::Work(QObject *parent)
    : QObject{parent}
{

}

void Work::doWork(QImage img,QThread *childThread)
{
    //转成base64编码
    QByteArray ba;
    QBuffer buff(&ba);
    img.save(&buff,"png");
    QString b64str=ba.toBase64();
    //qDebug()<<b64str;

    //请求体body体参数设置
    QJsonObject postJson;
    QJsonDocument doc;
    postJson.insert("image",b64str);
    postJson.insert("image_type","BASE64");
    postJson.insert("face_field","age,expression,face_shape,gender,glasses,quality,eye_status,emotion,face_type,mask,beauty");

    doc.setObject(postJson);
    QByteArray postData=doc.toJson(QJsonDocument::Compact);

    emit resultReady(postData,childThread);
}
