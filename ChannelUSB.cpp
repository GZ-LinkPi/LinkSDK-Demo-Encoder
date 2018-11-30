#include "ChannelUSB.h"

ChannelUSB::ChannelUSB(QObject *parent) : Channel(parent)
{
    audio=NULL;
    video=Link::create("DecodeV");
    encA=NULL;
    encV=Link::create("EncodeV");
    usb=Link::create("InputV4l2");
}

void ChannelUSB::init()
{
    video->start();
    usb->linkV(video);
    overlay->linkV(encV);
    Channel::init();
}

void ChannelUSB::updateConfig(QVariantMap cfg)
{
    if(cfg["enable"].toBool())
    {
        usb->start();
        encV->start(cfg["encv"].toMap());
    }
    else
    {
        usb->stop();
        encV->stop();
    }
    Channel::updateConfig(cfg);
}

