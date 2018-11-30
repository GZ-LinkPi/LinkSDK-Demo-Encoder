#include "Group.h"
#include "Json.h"
#include "Config.h"
#include <QFile>
#include <QDomDocument>

Group::Group(QObject *parent) :
    QObject(parent),timer(this)
{
    rpc=new GroupRPC();
    connect(&timer,SIGNAL(timeout()),this,SLOT(onTimer()));
}

void Group::init()
{
    rpc->init();
    rpc->addMethod("hello",this,"hello");
    rpc->addMethod("getNetwork",this,"getNetwork");
    rpc->addMethod("setNetwork",this,"setNetwork");
    rpc->addMethod("setEncode",this,"setEncode");
    rpc->addMethod("setStream",this,"setStream");
    rpc->addMethod("reboot",this,"reboot");
    rpc->addMethod("getEPG",this,"getEPG");
    rpc->addMethod("syncEPG",this,"syncEPG");

    loadConfig();
    timer.start(3000);
    clearMember();
    onTimer();
}

void Group::loadConfig()
{
    QVariantMap cfg=Json::loadFile(GRPPATH).toMap();
    rpc->setGroupId(cfg["groupId"].toInt());
    type=cfg["type"].toString();
}

void Group::onTimer()
{
    QVariantList list=Json::loadFile(CFGPATH).toList();
    QVariantList nameList;
    for(int i=0;i<list.count();i++)
    {
        if(list[i].toMap()["enable"].toBool())
            nameList<<list[i].toMap()["name"].toString();
    }
    memberInfo[rpc->getLoaclMac()]["info"]=nameList;
    QVariantMap data;
    data["info"]=nameList;
    data["type"]=type;
    rpc->call("hello","any",data,true);
}

void Group::hello(QVariant data)
{
    QString mac=rpc->getFromMac();
    memberInfo[mac]["ip"]=rpc->getFromIp().toString();
    memberInfo[mac]["info"]=data.toMap()["info"];
    memberInfo[mac]["type"]=data.toMap()["type"];


}

bool Group::clearMember()
{
    memberInfo.clear();
    //    memberInfo[rpc->getLoaclMac()]["ip"]=rpc->getLocalIp().toString();
    return true;
}

bool Group::update(QVariantMap json)
{
    Json::saveFile(json,GRPPATH);
    loadConfig();
    return true;
}

QVariant Group::setNetwork(QVariant cfg)
{
    Json::saveFile(cfg,NETPATH);
    system("/link/shell/setNetwork.sh");
    rpc->updateNet();
    //    memberInfo[rpc->getLoaclMac()]["ip"]=rpc->getLocalIp().toString();
    return true;
}

QVariant Group::setEncode(QVariant cfg)
{
    QVariantList chnList=Json::loadFile(CFGPATH).toList();
    for(int i=0;i<chnList.count();i++)
    {
        QVariantMap map=chnList[i].toMap();
        map["enca"]=cfg.toMap()["enca"].toMap();
        map["encv"]=cfg.toMap()["encv"].toMap();
        chnList[i]=map;
    }
    Json::saveFile(chnList,CFGPATH);
    Config::loadConfig(CFGPATH);
    return true;
}

QVariant Group::setStream(QVariant cfg)
{
    QVariantMap config=cfg.toMap();
    qDebug()<<config;
    QVariantMap push=config["push"].toMap();
    QVariantMap udp=config["udp"].toMap();
    bool ipPlus = false;
    bool portPlus = false;
    QString ipBase1 = "";
    int ipBase2 = 0;
    int portBase = 0;
    QString udpip=cfg.toMap()["udp"].toMap()["ip"].toString();
    QString udpport=cfg.toMap()["udp"].toMap()["port"].toString();

    if ( udpip.endsWith( "+" )  ) {
        udpip = udpip.replace( "+", "" );
        ipPlus = true;
        int n = udpip.lastIndexOf( "." );
        ipBase1 = udpip.left(n);
        ipBase2 = udpip.mid(n+1).toInt();
    }
    if ( udpport.endsWith("+") ) {
        udpport = udpport.left(udpport.length()-1);
        portPlus = true;
        portBase = udpport.toInt();
    }

    QVariantList chnList=Json::loadFile(CFGPATH).toList();
    for(int i=0;i<chnList.count();i++)
    {
        QVariantMap map=chnList[i].toMap();
        push["path"]=map["stream"].toMap()["push"].toMap()["path"];
        config["push"]=push;

        if(ipPlus)
            udp["ip"]=ipBase1+QString::number(ipBase2+2);
        if(portPlus)
            udp["port"]=portBase+i;

        config["udp"]=udp;
        map["stream"]=config;
        chnList[i]=map;

    }
    Json::saveFile(chnList,CFGPATH);
    Config::loadConfig(CFGPATH);
    return true;
}

QVariant Group::getNetwork()
{
    return Json::loadFile(NETPATH);
}

QVariantList Group::getList()
{
    QVariantList list;
    QVariantMap info;
    foreach(QString mac,memberInfo.keys())
    {
        info=memberInfo[mac];
        info["mac"]=mac;
        list<<info;
    }
    return list;
}

QVariantList Group::callGetEPG()
{
    QVariantMap epgMap;
    QVariantList epgList;
    foreach(QString mac,memberInfo.keys())
    {
        QVariantMap map=rpc->call("getEPG",mac).toMap();
        foreach(QString id,map.keys())
        {
            QVariantMap chn=map[id].toMap();
            chn["id"]=id;
            epgMap[id]=chn;
        }
    }
    this->epgMap=epgMap;

    QVariantList order=Json::loadFile(GRPPATH).toMap()["order"].toList();
    for(int i=0;i<order.count();i++)
    {
        QString id=order[i].toString();
        if(!epgMap.contains(id))
            continue;
        epgList.append(epgMap.take(id));
    }

    foreach(QString id,epgMap.keys())
    {
        epgList.append(epgMap[id]);
    }

    return epgList;
}

QVariantList Group::orderEPG(QVariantList order)
{
    QVariantMap epgMap=this->epgMap;
    QVariantMap cfg=Json::loadFile(GRPPATH).toMap();
    cfg["order"]=order;
    Json::saveFile(cfg,GRPPATH);

    QVariantList epgList;

    for(int i=0;i<order.count();i++)
    {
        QString id=order[i].toString();
        if(!epgMap.contains(id))
            continue;
        epgList.append(epgMap.take(id));
    }

    foreach(QString id,epgMap.keys())
    {
        epgList.append(epgMap[id]);
    }

    return epgList;

}

QVariant Group::getEPG()
{
    QVariantList chnList=Json::loadFile(CFGPATH).toList();
    QVariantMap epgMap;
    QVariantMap map;
    for(int i=0;i<chnList.count();i++)
    {
        if(!chnList[i].toMap()["enable"].toBool())
            continue;
        if(chnList[i].toMap()["type"].toString()=="usb")
            continue;


        map["name"]=chnList[i].toMap()["name"];
        QVariantMap stream=chnList[i].toMap()["stream"].toMap();
        QStringList urls;
        urls.clear();
        QString id=QString::number(chnList[i].toMap()["id"].toInt());
        QString ip=rpc->getLocalIp().toString();
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
        epgMap[rpc->getLoaclMac()+"-"+QString::number(i)]=map;
    }
    return epgMap;

}

QVariant Group::syncEPG(QVariant epg)
{
    Json::saveFile(epg,"/link/config/epg.json");
    createEPGXML(epg.toList());

    return true;
}

bool Group::createEPG()
{
    QVariantMap epgMap=this->epgMap;
    QVariantList order=Json::loadFile(GRPPATH).toMap()["order"].toList();
    QVariantList epgList;

    for(int i=0;i<order.count();i++)
    {
        QString id=order[i].toString();
        if(!epgMap.contains(id))
            continue;
        epgList.append(epgMap.take(id));
    }

    foreach(QString id,epgMap.keys())
    {
        epgList.append(epgMap[id]);
    }

    createEPGXML(epgList);
    Json::saveFile(epgList,"/link/config/epg.json");
    return true;
}

bool Group::createEPGXML(QVariantList epgList)
{
    QDomDocument docXML;
    QDomElement root=docXML.createElement("root");
    for(int i=0;i<epgList.count();i++)
    {
        QDomElement channel = docXML.createElement("channel");
        QDomElement name = docXML.createElement("name");
        name.appendChild(docXML.createTextNode(epgList[i].toMap()["name"].toString()));
        channel.appendChild(name);
        QStringList urls=epgList[i].toMap()["url"].toString().split("|");
        for(int k=0;k<urls.count();k++)
        {
            QDomElement url = docXML.createElement("url");
            url.appendChild(docXML.createTextNode(urls[k]));
            channel.appendChild(url);
        }
        root.appendChild(channel);
    }
    docXML.appendChild(root);
    QFile fileXML("/link/config/epg.xml");
    fileXML.open(QFile::ReadWrite);
    fileXML.resize(0);
    QTextStream out(&fileXML);
    docXML.save(out,4,QDomNode::EncodingFromTextStream);
    fileXML.close();
    return true;
}

void Group::reboot()
{
    system("reboot");
}


bool Group::callSetNetwork(QString mac, QVariantMap json)
{
    return rpc->call("setNetwork",mac,json).toBool();
}

QVariantMap Group::callGetNetwork(QString mac)
{
    return rpc->call("getNetwork",mac).toMap();
}

bool Group::callSetEncode(QString mac, QVariantMap json)
{
    return rpc->call("setEncode",mac,json).toBool();
}

bool Group::callSetStream(QString mac, QVariantMap json)
{
    return rpc->call("setStream",mac,json).toBool();
}

bool Group::callReboot(QString mac)
{
    rpc->call("reboot",mac,QVariant(),true);
    return true;
}

bool Group::callSyncEPG(QString mac)
{
    QVariant epg=Json::loadFile("/link/config/epg.json");
    return rpc->call("syncEPG",mac,epg).toBool();
}
