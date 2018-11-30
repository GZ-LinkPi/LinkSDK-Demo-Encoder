#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <Channel.h>
#include <QList>

#define CFGPATH "/link/config/config.json"
#define GRPPATH "/link/config/group.json"
#define NETPATH "/link/config/net.json"

class Config : public QObject
{
    Q_OBJECT
public:
    explicit Config(QObject *parent = 0);
    static void loadConfig(QString path);
    static QList<Channel*> chns;
    static Channel* findChannelById(int id);
signals:

public slots:

};

#endif // CONFIG_H
