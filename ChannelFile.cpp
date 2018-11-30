#include "ChannelFile.h"
#include <QDateTime>

ChannelFile::ChannelFile(QObject *parent) : Channel(parent)
{
    video=NULL;//Link::create("InputFile");
    audio=NULL;
    encV=Link::create("InputFile");
    encA=encV;
    index=-1;
    connect(encV,SIGNAL(newEvent(QString,QVariant)),this,SLOT(onNewEvent(QString,QVariant)));
}

void ChannelFile::init()
{
    Channel::init();
}

void ChannelFile::updateConfig(QVariantMap cfg)
{
    if(cfg["enable"].toBool())
    {
        QVariantList list=cfg["file"].toList();
        playList.clear();
        for(int i=0;i<list.count();i++)
        {
           playList.append(list[i].toString());
        }
        if(list.count()>0 && index==-1)
            playNext();

    }
    else
    {
        index=-1;
        encV->stop();
    }
    Channel::updateConfig(cfg);
}

QVariantList ChannelFile::getPlayList()
{
    QVariantList ret;
    for(int i=0;i<playList.count();i++)
    {
        QVariantMap map;
        QString name=playList[i];
        map["name"]=name;
        if(!durationMap.contains(name))
        {
            durationMap[name]=encV->invoke("getDuration",fullPath(name)).toLongLong();
        }
        map["duration"]=durationMap[playList[i]];
        ret<<map;
    }
    return ret;
}

bool ChannelFile::seek(int index, qint64 time)
{
    this->index=index;
    QVariantMap data;
    data["path"]=playList[index];
    encV->start(data);
    encV->invoke("seek",time);
    return true;
}

QVariantMap ChannelFile::getPosition()
{
    QVariantMap ret;
    ret["file"]=file;
    ret["position"]=encV->invoke("getPosition").toInt();
    return ret;
}

bool ChannelFile::play(int index, int time)
{
    this->index =index-1;
    playNext();
    encV->invoke("seek",time);
    return true;
}

void ChannelFile::playNext()
{
    index=(index+1)%playList.count();
    QVariantMap data;
    file=playList[index];
    data["path"]=fullPath(playList[index]);
    qDebug()<<"Play"<< data["path"].toString();
    encV->start(data);

}

QString ChannelFile::fullPath(QString path)
{
    return "/root/usb/"+path;
}

void ChannelFile::onNewEvent(QString type, QVariant )
{
    qDebug()<<type;
    if(type!="EOF")
        return;

    playNext();
}

