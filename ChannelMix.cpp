#include "ChannelMix.h"
#include "Config.h"

ChannelMix::ChannelMix(QObject *parent) : Channel(parent)
{
    audio=Link::create("MixA");
    video=Link::create("MixV");
    encA=Link::create("EncodeA");
    encV=Link::create("EncodeV");
}

void ChannelMix::init()
{
    audio->linkA(encA);
    overlay->linkV(encV);
    Channel::init();
}

void ChannelMix::updateConfig(QVariantMap cfg)
{
    QVariantList srcV=cfg["srcV"].toList();
    QVariantList srcA=cfg["srcA"].toList();
    QVariantList videoList;
    for(int i=0;i<srcV.count();i++)
    {

        if(srcV[i].toInt()!=-1)
        {
            LinkObject *v=Config::findChannelById(srcV[i].toInt())->video;

            videoList.append(v->name());
            v->linkV(video);
        }
        else
        {
            videoList.append("unknow");
        }
    }

    QVariantMap dataMixV;
    dataMixV["src"]=videoList;
    dataMixV["layout"]=cfg["layout"].toList();
    video->start(dataMixV);

    foreach(int id,curAList)
    {
        if(!srcA.contains(id))
        {
            LinkObject *a=Config::findChannelById(id)->audio;
            a->unLinkA(audio);
            curAList.removeAll(id);
        }
    }

    QVariantMap dataMixA;
    for(int i=0;i<srcA.count();i++)
    {
        if(srcA[i]==-1)
            continue;
        Channel *chn=Config::findChannelById(srcA[i].toInt());
        if(chn->type!="vi")
            continue;
        LinkObject *a=chn->audio;
        if(!dataMixA.contains("main"))
            dataMixA["main"]=a->name();
        a->linkA(audio);
        curAList.append(srcA[i].toInt());
    }
    audio->start(dataMixA);

    if(cfg["enable"].toBool())
    {
        encA->start(cfg["enca"].toMap());
        encV->start(cfg["encv"].toMap());
    }
    else
    {
        encA->stop();
        encV->stop();
    }
    Channel::updateConfig(cfg);
}

