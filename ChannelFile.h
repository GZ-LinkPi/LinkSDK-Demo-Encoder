#ifndef CHANNELFILE_H
#define CHANNELFILE_H

#include "Channel.h"
#include <QList>
#include <QMap>

class ChannelFile : public Channel
{
    Q_OBJECT
public:
    explicit ChannelFile(QObject *parent = 0);
    virtual void init();
    virtual void updateConfig(QVariantMap cfg);
    QVariantList getPlayList();
    bool seek(int index,qint64 time);
    QVariantMap getPosition();
    bool play(int index,int time);
private:
    QList<QString> playList;
    QMap<QString,int> durationMap;
    QString file;
    int index;
private:
    void playNext();
    QString fullPath(QString path);
signals:

public slots:
    void onNewEvent(QString type, QVariant);
};

#endif // CHANNELFILE_H
