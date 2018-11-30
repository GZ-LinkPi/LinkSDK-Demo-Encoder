#ifndef CHANNEL_H
#define CHANNEL_H

#include <QObject>
#include "Link.h"
#include <QMap>

class Channel : public QObject
{
    Q_OBJECT
public:
    explicit Channel(QObject *parent = 0);
    virtual void init();
    virtual void updateConfig(QVariantMap cfg);
    void doSnap();
    QString type;
    int id;
    QVariantMap getReport();
    QVariantMap data;
    LinkObject *audio;
    LinkObject *video;
    LinkObject *volume;
    bool enable;
protected:
    static LinkObject *httpServer;
    static LinkObject *rtspServer;
    LinkObject *udp;    
    LinkObject *encA;
    LinkObject *encV;
    LinkObject *encVsub;
    LinkObject *overlay;
    LinkObject *snap;

private:

    QMap<QString,LinkObject*> muxMap;
signals:

public slots:

};

#endif // CHANNEL_H
