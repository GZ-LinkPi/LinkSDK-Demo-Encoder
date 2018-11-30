#include <QCoreApplication>
#include <QTextCodec>
#include "Link.h"
#include "Config.h"
#include "RPC.h"
#include "Version.h"
#include "Json.h"

RPC *GRPC;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);

    if(!Link::init())
        return 0;

    QString ver;
    ver=ver.sprintf("%s build %s_%d",VERSION_VER,VERSION_DATE,VERSION_BUILD);

    QVariantMap vjs=Json::loadFile("/link/config/version.json").toMap();
    if(vjs["app"].toString()!=ver)
    {
        vjs["app"]=ver;
        Json::saveFile(vjs,"/link/config/version.json");
    }

    Config::loadConfig(CFGPATH);
    GRPC=new RPC();
    GRPC->init();

    return a.exec();
}

