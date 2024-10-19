#include <QApplication>
#include "Logging.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    auto console = Logging::quickNewConsole();
    console->show();

    qDebug() << " debug string ";
    qWarning() << " warning string ";
    qInfo() << " info string ";
    qCritical() << " critical string ";

    return a.exec();
}
