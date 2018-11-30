#include "GroupRPC.h"
#include <QNetworkInterface>
#include <sys/socket.h>
#include <QEventLoop>
#include <QTimer>
#include <QMetaMethod>

#define GRP_REQUEST "request"
#define GRP_RESPOND "respond"

uint GroupRequest::nextSeq=0;
GroupRequest::GroupRequest(QObject *parent) : QObject(parent)
{
    seq=nextSeq++;
}


GroupRPC::GroupRPC(QObject *parent) : QObject(parent),socket(this)
{
    timeout=200;
    connect(&socket,SIGNAL(readyRead()),this,SLOT(onRead()));
    groupId=0;
    port=5432;
}

bool GroupRPC::init(int p)
{
    port=p;
    updateNet();
    if(!socket.bind(port))
        return false;
    socket.setSocketOption(QUdpSocket::SendBufferSizeSocketOption,1024*1024);
    socket.setSocketOption(QUdpSocket::ReceiveBufferSizeSocketOption,1024*1024);

    return true;
}

void GroupRPC::updateNet()
{
    QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();
    for(int i = 0; i < nets.count(); i ++)
    {
        if(!nets[i].flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            localMac = nets[i].hardwareAddress();
            QList<QNetworkAddressEntry> addrs = nets[i].addressEntries();
            for(int k=0;k<addrs.count();k++)
            {
                if(addrs[k].ip()!=QHostAddress::LocalHost && addrs[k].ip().protocol()==QAbstractSocket::IPv4Protocol)
                {
                    localIp = addrs[k].ip();
                }
            }
            break;
        }
    }
}

void GroupRPC::addMethod(QString name, QObject *obj, const char *method)
{
    objectMap[name]=obj;
    methodMap[name]=method;
}

QVariant GroupRPC::call(QString method, QString to, QVariant data, bool noRespond)
{
    GroupRequest *request=new GroupRequest();
    requestList.append(request);
    QVariantMap msg;
    msg["from"]=localMac;
    msg["to"]=to;
    msg["groupId"]=groupId;
    msg["seq"]=request->seq;
    msg["method"]=method;
    msg["type"]=GRP_REQUEST;
    msg["data"]=data;
    if(to!=localMac)
    {
        send(msg);
        if(to=="any")
        {
            fromIp=localIp;
            callbak(msg);
        }
    }
    else
    {
        return callbak(msg);
    }

    if(!noRespond)
    {
        QEventLoop loopLoad;
        QTimer timer;
        QObject::connect(request,SIGNAL(respond()), &loopLoad, SLOT(quit()));
        QObject::connect(request,SIGNAL(respond()), &timer, SLOT(stop()));
        QObject::connect(&timer, SIGNAL(timeout()), &loopLoad, SLOT(quit()));
        timer.start(timeout);
        loopLoad.exec();
        if(timer.isActive())
        {
            timer.stop();
            qDebug()<<"timeout";
        }
    }
    requestList.removeAll(request);
    request->deleteLater();

    return request->ret;
}

QString GroupRPC::getLoaclMac()
{
    return localMac;
}

void GroupRPC::setGroupId(int gid)
{
    groupId=gid;
}

QHostAddress GroupRPC::getFromIp()
{
    return fromIp;
}

QHostAddress GroupRPC::getLocalIp()
{
    return localIp;
}

QString GroupRPC::getFromMac()
{
    return fromMac;
}


QVariant GroupRPC::callbak(QVariantMap msg)
{
    fromMac=msg["from"].toString();
    QString method=msg["method"].toString();
    msg["to"]=msg["from"];
    msg["from"]=localMac;
    msg["type"]=GRP_RESPOND;
    QVariant ret;
    QVariant data=msg["data"];
    int retType=0;

    for(int i=0;i<objectMap[method]->metaObject()->methodCount();i++)
    {
        if(objectMap[method]->metaObject()->method(i).name()==method)
        {
            retType=objectMap[method]->metaObject()->method(i).returnType();
        }
    }
    if(retType!=43)
    {
        if(!data.isNull())
            QMetaObject::invokeMethod(objectMap[method],methodMap[method],Qt::DirectConnection,Q_RETURN_ARG(QVariant,ret),Q_ARG(QVariant,data));
        else
            QMetaObject::invokeMethod(objectMap[method],methodMap[method],Qt::DirectConnection,Q_RETURN_ARG(QVariant,ret));
        msg["data"]=ret;
        if(msg["to"]!=localMac)
            send(msg);
        else
            return ret;
    }
    else
    {
        if(!data.isNull())
            QMetaObject::invokeMethod(objectMap[method],methodMap[method],Qt::DirectConnection,Q_ARG(QVariant,data));
        else
            QMetaObject::invokeMethod(objectMap[method],methodMap[method],Qt::DirectConnection);
    }

    return ret;
}

void GroupRPC::send(QVariantMap &msg)
{
    QByteArray json=QJsonDocument::fromVariant(msg).toJson();
    socket.writeDatagram(json,QHostAddress::Broadcast,port);
}


void GroupRPC::onRead()
{
    while(socket.hasPendingDatagrams())
    {
        QByteArray ba;
        ba.resize(socket.pendingDatagramSize());

        socket.readDatagram(ba.data(),ba.size(),&fromIp);

        QVariantMap msg=QJsonDocument::fromJson(ba).toVariant().toMap();
        if(!msg.contains("to"))
            continue;

        if(msg["from"].toString()==localMac || (msg["to"].toString()!=localMac && msg["to"].toString()!="any") || msg["groupId"].toInt()!=groupId)
            continue;

        QString type=msg["type"].toString();
        if(type==GRP_REQUEST)
        {
            callbak(msg);
        }
        else if(type==GRP_RESPOND)
        {
            foreach(GroupRequest *req,requestList)
            {
                if(req->seq==msg["seq"].toUInt())
                {
                    req->ret=msg["data"];
                    emit req->respond();
                }
            }

        }


    }
}

