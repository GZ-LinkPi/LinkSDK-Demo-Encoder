#ifndef CHANNELNET_H
#define CHANNELNET_H

#include "Channel.h"

class ChannelNet : public Channel
{
    Q_OBJECT
public:
    explicit ChannelNet(QObject *parent = 0);
    virtual void init();
    virtual void updateConfig(QVariantMap cfg);
private:
    LinkObject *net;
signals:

public slots:

};

#endif // CHANNELNET_H
