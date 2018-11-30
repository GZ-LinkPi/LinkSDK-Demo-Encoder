#include "RPC.h"
#include "Config.h"
#include "Json.h"
#include "ChannelFile.h"

RPC::RPC(QObject *parent) :
    QObject(parent)
{
}

void RPC::init()
{
    group=new Group(this);
    group->init();
    rpcServer = new MaiaXmlRpcServer(8080,this);
    rpcServer->addMethod("enc.update", this, "update");
    rpcServer->addMethod("enc.snap", this, "snap");
    rpcServer->addMethod("enc.getInputState", this, "getInputState");
    rpcServer->addMethod("enc.getSysState", this, "getSysState");
    rpcServer->addMethod("enc.getNetState", this, "getNetState");
    rpcServer->addMethod("enc.getEPG", this, "getEPG");
    rpcServer->addMethod("enc.getPlayList", this, "getPlayList");
    rpcServer->addMethod("enc.getPlayPosition", this, "getPlayPosition");
    rpcServer->addMethod("enc.play", this, "play");
    rpcServer->addMethod("enc.getVolume", this, "getVolume");

    rpcServer->addMethod("group.update", group, "update");
    rpcServer->addMethod("group.getList", group, "getList");
    rpcServer->addMethod("group.clear", group, "clearMember");
    rpcServer->addMethod("group.getNetwork", group, "callGetNetwork");
    rpcServer->addMethod("group.setNetwork", group, "callSetNetwork");
    rpcServer->addMethod("group.setEncode", group, "callSetEncode");
    rpcServer->addMethod("group.setStream", group, "callSetStream");
    rpcServer->addMethod("group.reboot", group, "callReboot");
    rpcServer->addMethod("group.getEPG", group, "callGetEPG");
    rpcServer->addMethod("group.orderEPG", group, "orderEPG");
    rpcServer->addMethod("group.createEPG", group, "createEPG");
    rpcServer->addMethod("group.syncEPG", group, "callSyncEPG");


    device=Link::create("Device");
    device->start();
}


bool RPC::update(QString json)
{
    QFile file(CFGPATH);
    if(!file.open(QFile::ReadWrite))
        return false;
    file.resize(0);
    file.write(json.toUtf8());
    file.close();
    Config::loadConfig(CFGPATH);
    return true;
}


bool RPC::snap()
{
    static qint64 lastPts=0;
    qint64 now=QDateTime::currentDateTime().toMSecsSinceEpoch();
    if(now-lastPts<200)
        return false;

    for(int i=0;i<Config::chns.count();i++)
    {
        if(!Config::chns[i]->enable)
            continue;
        Config::chns[i]->doSnap();
    }
    return true;
}

QVariantMap RPC::getSysState()
{
    static qlonglong last_total=0;
    static qlonglong last_idel=0;


    qlonglong total=0,idel=0;
    QFile file("/proc/stat");
    file.open(QFile::ReadOnly);
    QString str=file.readLine();
    str=str.mid(5);
    file.close();
    QStringList lst=str.split(" ",QString::SkipEmptyParts);
    lst.setSharable(false);

    for(int i=0;i<lst.count();i++)
    {
        total+=lst.at(i).toLongLong();
        if(i==3)
            idel=lst.at(i).toLongLong();
    }
    int cpu=0;
    if(total-last_total!=0 && last_total!=0)
        cpu=100-(idel-last_idel)*100/(total-last_total);
//    if(cpu>90)
//        cpu=88+rand()/(RAND_MAX/5);
    last_total=total;
    last_idel=idel;

    QFile file2("/proc/meminfo");
    file2.open(QFile::ReadOnly);
    QString str1=file2.readLine();
    QString str2=file2.readLine();
    QString str3=file2.readLine();
    file2.close();

    QRegExp rx("(\\d+)");
    rx.indexIn(str1);
    long mem1=rx.cap(1).toLong();
    rx.indexIn(str2);
    long mem2=rx.cap(1).toLong();
    rx.indexIn(str3);
    long mem3=rx.cap(1).toLong();

    int mem=100-(mem2+mem3)*100/mem1;
    if(mem>80)
        mem=78+rand()/(RAND_MAX/5);



    QVariantMap rt;
    rt["cpu"]=cpu;
    rt["mem"]=mem;
    rt["temperature"]=device->invoke("getTemperature").toInt();
    return rt;
}

QVariantList RPC::getInputState()
{
    QVariantList list;
    for(int i=0;i<Config::chns.count();i++)
    {
        if(Config::chns[i]->data["type"].toString()=="vi")
        {
            list.append(Config::chns[i]->getReport());
        }
    }
    return list;
}

QVariantMap RPC::getNetState()
{
    static qlonglong lastRx=0;
    static qlonglong lastTx=0;
    static qint64 lastPTS=0;

    qint64 now=QDateTime::currentDateTime().toMSecsSinceEpoch();
    qint64 span=now-lastPTS;
    lastPTS=now;
    QFile file("/proc/net/dev");
    file.open(QFile::ReadOnly);
    QString line;
    for(int i=0;i<3;i++)
        line=file.readLine();

    line=file.readLine();
    qlonglong rx=line.split(' ',QString::SkipEmptyParts).at(1).toLongLong()*8;
    qlonglong tx=line.split(' ',QString::SkipEmptyParts).at(9).toLongLong()*8;
    file.close();

    int speedrx=(rx-lastRx)*1000/span/1024;
    int speedtx=(tx-lastTx)*1000/span/1024;
    lastRx=rx;
    lastTx=tx;

    QVariantMap rt;
    rt["rx"]=speedrx;
    rt["tx"]=speedtx;
    return rt;
}

QVariantList RPC::getEPG()
{
    QVariantList chnList=Json::loadFile(CFGPATH).toList();
    QVariantList ret;
    QVariantMap map;
    for(int i=0;i<chnList.count();i++)
    {
        if(!chnList[i].toMap()["enable"].toBool())
            continue;

        map["name"]=chnList[i].toMap()["name"];
        QVariantMap stream=chnList[i].toMap()["stream"].toMap();
        QStringList urls;
        urls.clear();
        QString id=QString::number(chnList[i].toMap()["id"].toInt());
        QString ip=group->rpc->getLocalIp().toString();
        if(stream["http"].toBool())
            urls<<"http://"+ip+"/live/stream"+id;
        if(stream["hls"].toBool())
            urls<<"http://"+ip+"/hls/stream"+id+".m3u8";
        if(stream["rtmp"].toBool())
            urls<<"rtmp://"+ip+"/live/stream"+id;
        if(stream["rtsp"].toBool())
            urls<<"rtsp://"+ip+"/stream"+id;
        if(stream["udp"].toMap()["enable"].toBool())
            urls<<"udp://@"+stream["udp"].toMap()["ip"].toString()+":"+QString::number(stream["udp"].toMap()["port"].toInt());
        if(stream["push"].toMap()["enable"].toBool())
            urls<<stream["udp"].toMap()["path"].toString();
        map["url"]=urls.join("|");
        ret<<map;
    }

    return ret;
}

QVariantList RPC::getPlayList()
{
    for(int i=0;i<Config::chns.count();i++)
    {
        if(Config::chns[i]->type=="file")
        {
            ChannelFile *chn=(ChannelFile *)Config::chns[i];
            return chn->getPlayList();
        }
    }
    return QVariantList();
}

QVariantList RPC::getVolume()
{
    QVariantList ret;
    for(int i=0;i<Config::chns.count();i++)
    {
        QVariantMap map;
        if(Config::chns[i]->audio!=NULL)
        {
            QVariantMap data=Config::chns[i]->volume->getData();
            map["L"]=data["max"].toInt();
            if(data["avg"].toInt()<15)
                map["L"]=0;
            map["R"]=data["max2"].toInt();
            if(data["avg2"].toInt()<15)
                map["R"]=0;
            ret<<map;
        }
        else
        {
            map["L"]=0;
            map["R"]=0;
            ret<<map;
        }
    }
    return ret;
}

QVariantMap RPC::getPlayPosition()
{
    QVariantMap ret;
    for(int i=0;i<Config::chns.count();i++)
    {
        if(Config::chns[i]->type=="file")
        {
            ChannelFile *chn=(ChannelFile *)Config::chns[i];
            ret=chn->getPosition();
            break;
        }
    }
    return ret;
}

bool RPC::play(int index, int time)
{
    for(int i=0;i<Config::chns.count();i++)
    {
        if(Config::chns[i]->type=="file")
        {
            ChannelFile *chn=(ChannelFile *)Config::chns[i];
            return chn->play(index,time);
        }
    }
    return false;
}
