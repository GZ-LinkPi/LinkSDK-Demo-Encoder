#include <QCoreApplication>
#include <QTextCodec>
#include "Link.h"
#include "Config.h"
#include "RPC.h"

RPC *GRPC;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);

    if(!Link::init())
        return 0;

    Config::loadConfig(CFGPATH);
    GRPC=new RPC();
    GRPC->init();

    return a.exec();
}

