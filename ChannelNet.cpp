#include "ChannelNet.h"

ChannelNet::ChannelNet(QObject *parent) :
    Channel(parent)
{
    video=Link::create("DecodeV");    
    net=Link::create("InputNet");
    encV=net;
    encA=net;
}

void ChannelNet::init()
{
    Channel::init();
}

void ChannelNet::updateConfig(QVariantMap cfg)
{
    if(cfg["net"].toMap()["decode"].toBool())
    {
        net->linkV(video);
        video->start();
    }
    else
        video->stop();

    if(cfg["enable"].toBool())
    {
        QVariantMap nd;
        nd["path"]=cfg["net"].toMap()["path"].toString();
        net->start(nd);
    }
    else
    {
        net->stop();
    }
    Channel::updateConfig(cfg);
}
