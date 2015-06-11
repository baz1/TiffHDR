#include "gui/mainwindow.h"
#include "util/settings.h"
#include <QApplication>
#include <QStringList>
#include <QFile>
#include <stdio.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QStringList args = QApplication::arguments();
    args.removeFirst();
    bool debug = false;
    if ((!args.isEmpty()) && (args.at(0) == "-d"))
    {
        debug = true;
        args.removeFirst();
    }
    for (int i = args.length(); --i >= 0; )
    {
        if (!QFile::exists(args.at(i)))
        {
            fprintf(stderr, "Error: The file \"%s\" does not exist.\n", qPrintable(args.at(i)));
            fprintf(stderr, "Usage: TiffHDR [-d] [filename1] [filename2] [...]\n");
            return 0;
        }
    }
    MainWindow w(debug);
    Settings::init(&w);
    w.show();
    if (!args.isEmpty())
        w.startTIFFLoading(args);
    return a.exec();
}
