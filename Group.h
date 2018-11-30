#ifndef GROUP_H
#define GROUP_H

#include <QObject>
#include "GroupRPC.h"
#include <QTimer>
#include <QList>
#include <QVariantMap>

class Group : public QObject
{
    Q_OBJECT
public:
    explicit Group(QObject *parent = 0);
    void init();
    GroupRPC *rpc;
private:
    void loadConfig();
private:
    QString type;
    QTimer timer;
    QMap<QString,QVariantMap> memberInfo;
    QVariantMap epgMap;
signals:

public slots:

    void onTimer();
    void hello(QVariant data);
    bool clearMember();
    bool update(QVariantMap json);
    QVariant setNetwork(QVariant cfg);
    QVariant setEncode(QVariant cfg);
    QVariant setStream(QVariant cfg);
    QVariant getNetwork();
    QVariantList getList();
    QVariantList callGetEPG();
    QVariantList orderEPG(QVariantList order);
    QVariant getEPG();
    QVariant syncEPG(QVariant epg);
    bool createEPG();
    bool createEPGXML(QVariantList epgList);
    void reboot();
    bool callSetNetwork(QString mac, QVariantMap json);
    QVariantMap callGetNetwork(QString mac);
    bool callSetEncode(QString mac, QVariantMap json);
    bool callSetStream(QString mac, QVariantMap json);
    bool callReboot(QString mac);
    bool callSyncEPG(QString mac);
};
#endif // GROUP_H
