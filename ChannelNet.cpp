#include "ChannelNet.h"
#include <QFile>
#include <unistd.h>

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
    if(QFile::exists("/link/config/tucodec"))
    {
        encA=NULL;
        encV=Link::create("EncodeV");
        video->linkV(encV);
    }
    net->linkV(video);
    Channel::init();
}

void ChannelNet::updateConfig(QVariantMap cfg)
{
    if(cfg["enable"].toBool())
    {
        video->start();
    }
    else
        video->stop();

    if(cfg["enable"].toBool())
    {
        QVariantMap nd;
        nd["path"]=cfg["net"].toMap()["path"].toString();
        if(cfg["net"].toMap()["tcp"].toBool())
            nd["protocol"]="tcp";
        else
            nd["protocol"]="udp";

        if(encV!=net)
        {
            if(nd["path"].toString()!=net->getData()["path"].toString())
            {
                encV->stop();
                net->stop();
                video->stop();
            }

            QVariantMap encD=cfg["encv"].toMap();
            encV->start(encD);
            video->start();
        }
        net->start(nd);
    }
    else
    {
        if(encV!=net)
            encV->stop();
        net->stop();
    }
    Channel::updateConfig(cfg);
}
