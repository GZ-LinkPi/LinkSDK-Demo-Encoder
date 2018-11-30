#include "Channel.h"
#include "unistd.h"

LinkObject* Channel::httpServer=NULL;
LinkObject* Channel::rtspServer=NULL;
Channel::Channel(QObject *parent) :
    QObject(parent)
{
    overlay=Link::create("Overlay");
    volume=Link::create("Volume");
    snap=Link::create("EncodeV");
    QVariantMap sd;
    sd["width"]=640;
    sd["height"]=360;
    sd["codec"]="jpeg";
    snap->start(sd);
    enable=false;
    audio=NULL;
    video=NULL;
    encA=NULL;
    encV=NULL;
    id=-1;
    if(httpServer==NULL)
    {
        httpServer=Link::create("TSHttp");
        httpServer->start();
    }
    if(rtspServer==NULL)
    {
        rtspServer=Link::create("Rtsp");
        rtspServer->start();
    }
}

void Channel::init()
{
    if(video!=NULL)
    {
        video->linkV(overlay)->linkV(snap);
    }

    if(audio!=NULL)
    {
        volume->start();
        audio->linkA(volume);
    }

    QVariantMap path;
    if(encA==NULL)
        path["mute"]=true;

    muxMap["rtmp"]=Link::create("Mux");
    path["path"]="rtmp://127.0.0.1/live/stream" + QString::number(id);
    muxMap["rtmp"]->setData(path);

    muxMap["hls"]=Link::create("Mux");
    path["path"]="/tmp/hls/stream" + QString::number(id)+".m3u8";
    muxMap["hls"]->setData(path);

    muxMap["ts"]=Link::create("Mux");
    path["format"]="mpegts";
    path["path"]="mem://stream" + QString::number(id);
    muxMap["ts"]->setData(path);

    udp=Link::create("TSUdp");
    muxMap["ts"]->linkV(udp);
    muxMap["push"]=Link::create("Mux");


    foreach(QString key,muxMap.keys())
    {
        if(encA!=NULL)
            encA->linkA(muxMap[key]);
        encV->linkV(muxMap[key]);
    }

}

void Channel::updateConfig(QVariantMap cfg)
{
    data=cfg;
    enable=data["enable"].toBool();
    QVariantMap lays;
    lays["lays"]=cfg["overlay"].toList();
    overlay->start(lays);

    QVariantMap stream;
    stream=cfg["stream"].toMap();


    if(stream["rtmp"].toBool())
        muxMap["rtmp"]->start();
    else
        muxMap["rtmp"]->stop();

    if(stream["hls"].toBool())
        muxMap["hls"]->start();
    else
        muxMap["hls"]->stop();


    if(stream["http"].toBool() || stream["rtsp"].toBool() || stream["udp"].toMap()["enable"].toBool())
        muxMap["ts"]->start();
    else
        muxMap["ts"]->stop();

    if(stream["http"].toBool())
        muxMap["ts"]->linkV(httpServer);
    else
        muxMap["ts"]->unLinkV(httpServer);

    if(stream["rtsp"].toBool())
        muxMap["ts"]->linkV(rtspServer);
    else
        muxMap["ts"]->unLinkV(rtspServer);



    if(stream["udp"].toMap()["enable"].toBool())
    {
        udp->start(stream["udp"].toMap());
    }
    else
        udp->stop();

    if(stream["push"].toMap()["enable"].toBool())
        muxMap["push"]->start(stream["push"].toMap());
    else
        muxMap["push"]->stop();

}

void Channel::doSnap()
{
    QString path="/tmp/snap/snap"+QString::number(id)+".jpg";
    snap->invoke("snap",path);
}

QVariantMap Channel::getReport()
{
    return video->invoke("getReport").toMap();
}
