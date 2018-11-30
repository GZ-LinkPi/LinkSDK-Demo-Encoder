#include "ChannelVI.h"
#include <math.h>
#include <QFile>
LinkObject *ChannelVI::audioMini=NULL;
LinkObject *ChannelVI::audioMiniOut=NULL;

ChannelVI::ChannelVI(QObject *parent) :
    Channel(parent)
{
    audio=Link::create("InputAi");
    video=Link::create("InputVi");
    encA=Link::create("EncodeA");
    encV=Link::create("EncodeV");
    isSrcLine=false;
    if(audioMini==NULL)
    {
        audioMini=Link::create("InputAi");
        QVariantMap data;
        data["interface"]="Mini-In";
        audioMini->start(data);

        audioMiniOut=Link::create("OutputAo");
        data["interface"]="Mini-Out";
        audioMiniOut->start(data);

        audioMini->linkA(audioMiniOut);

    }
}

void ChannelVI::init()
{
    audio->linkA(encA);
    overlay->linkV(encV);
    encVsub=Link::create("EncodeV");

    if(QFile::exists("/link/config/ncast"))
    {
        QVariantMap dataEnc;
        dataEnc["width"]=640;
        dataEnc["height"]=360;
        dataEnc["bitrate"]=1000;
        encVsub->start(dataEnc);
        overlay->linkV(encVsub);

        LinkObject *mux=Link::create("Mux");
        QVariantMap path;
        path["path"]="rtmp://127.0.0.1/live/sub" + QString::number(id);
        mux->start(path);

        encVsub->linkV(mux);
        encA->linkA(mux);
    }

    Channel::init();
}

void ChannelVI::updateConfig(QVariantMap cfg)
{
    if(cfg["enable"].toBool())
    {
        QVariantMap ad;
        ad["resamplerate"]=cfg["enca"].toMap()["samplerate"].toInt();
        ad["num"]=ad["resamplerate"].toInt()/50;
        ad["interface"]=cfg["interface"].toString();
        audio->start(ad);

        QVariantMap vd;
        vd["interface"]=cfg["interface"].toString();
        video->start(vd);

        encA->start(cfg["enca"].toMap());
        QVariantMap encD=cfg["encv"].toMap();        
        encV->start(encD);


        QVariantMap cfga=cfg["enca"].toMap();
        if(cfga.contains("audioSrc") && cfga["audioSrc"].toString()=="line" && !isSrcLine)
        {
            audio->unLinkA(encA);
            audioMini->linkA(encA);
            isSrcLine=true;
        }
        else if(cfga.contains("audioSrc") && cfga["audioSrc"].toString()=="hdmi" && isSrcLine)
        {
            isSrcLine=false;
            audioMini->unLinkA(encA);
            audio->linkA(encA);
        }
    }
    else
    {
        encA->stop();
        encV->stop();
    }
    Channel::updateConfig(cfg);
}
