#include "mywidget.h"
#include <QApplication>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <execinfo.h>
#include <unistd.h>
#include <signal.h>
#include <sys/reboot.h>

void dump(int signo)
{
    void *array[10];
    size_t size;
    char **strings;
    size_t i;
    ;
    size = backtrace (array, 10);
    strings = backtrace_symbols (array, size);
    printf ("Obtained %zd stack frames.\n", size);
    for(i = 0; i<=size; i++){
        printf ("%s\n", strings[i]);
    }
    free (strings);
    //system("reboot");
    reboot(0);
    exit(0);
}

int main(int argc, char *argv[])
{
    signal(SIGSEGV, &dump);
    QApplication a(argc, argv);
    //QApplication::setOverrideCursor(Qt::BlankCursor);
    MyWidget w;
    w.show();

    return a.exec();
}
