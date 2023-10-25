//#include <QCoreApplication>
#include "include.h"

int main(int argc, char *argv[])
{
//  QCoreApplication a(argc, argv);
    pthread_t analog_tid;

    if(pthread_create(&analog_tid, NULL,Analog_proc, NULL) != 0)
    {
        printf("create pth_spi1_proc failed\n");
    }

    while (1)
    {
       sleep(1);
    }
    return 0;//a.exec();
}
