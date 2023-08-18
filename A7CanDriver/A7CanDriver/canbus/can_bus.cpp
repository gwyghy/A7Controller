#include "can_bus.h"
#include <stdio.h>
#include "timers.h"
#include <sstream>

#define prop_printf printf

using namespace std;


CanFrame prodata_to_lawdata(const CANDATAFORM & f)
{
    CanFrame cansend;
    memset(&cansend, 0 , sizeof(CanFrame));

    cansend.can_id = (f.IDE<<31)|(f.RTR<<30)|(f.ExtId);
    cansend.can_dlc = f.DLC;
    memcpy(cansend.data, f.Data, cansend.can_dlc);
    return cansend;

}

CANDATAFORM lawdata_to_prodata(const CanFrame & f)
{
    CANDATAFORM rxcan;

    memset(&rxcan, 0x00, sizeof(CANDATAFORM));
    rxcan.IDE = (f.can_id>>31)&1;
    rxcan.RTR = (f.can_id>>30)&1;
    if(rxcan.IDE)
    {
        rxcan.ExtId = f.can_id&0x1FFFFFFF;
    }
    else
    {
       rxcan.StdId = f.can_id&0x7FF;
    }

    if(rxcan.RTR&1)
    {
       rxcan.DLC = 0;
    }
    else
    {
       rxcan.DLC = f.can_dlc;
       memcpy(rxcan.Data, f.data, rxcan.DLC);
    }
//    zprintf3("ID = 0x%x, DLC = %d data[0]:%d,data[1]:%d\n",rxcan.ExtId,rxcan.DLC,rxcan.Data[0],rxcan.Data[1]);
    return rxcan;
}

CANDATAFORM lawdata_to_prodata(CanFrame * f)
{
    CANDATAFORM rxcan;

    memset(&rxcan, 0x00, sizeof(CANDATAFORM));
    rxcan.IDE = (f->can_id>>31)&1;
    rxcan.RTR = (f->can_id>>30)&1;
    if(rxcan.IDE)
    {
        rxcan.ExtId = f->can_id&0x1FFFFFFF;
    }
    else
    {
       rxcan.StdId = f->can_id&0x7FF;
    }

    if(rxcan.RTR&1)
    {
       rxcan.DLC = 0;
    }
    else
    {
       rxcan.DLC = f->can_dlc;
       memcpy(rxcan.Data, f->data, rxcan.DLC);
    }
    return rxcan;
}


int  call_write_back(CanDriver * pro, CANDATAFORM data)
{

        pro->writeframe(data);
        return 0;
}

 int CanDriver::can_bus_init(int registerdev, int brate)
 {
     int ret;
     struct sockaddr_can addr;
     struct ifreq ifr;
     char canname[6];
     ostringstream canset;

     canIndex = registerdev;

        prop_printf("Socket PF_CAN start!\n");
     /* socketCAN连接 */
     CanFileP = socket(PF_CAN, SOCK_RAW, CAN_RAW);
     if(CanFileP < 0)
     {
         prop_printf("Socket PF_CAN failed!\n");
         return -1;
     }


     sprintf(canname, "vcan%d", registerdev-1);
     strcpy(ifr.ifr_name, canname);


    printf("can %s\n",canname);

     ret = ioctl(CanFileP, SIOCGIFINDEX, &ifr);
     if(ret<0)
     {
         prop_printf("Ioctl failed!\n");
         return -2;
     }

     addr.can_family = PF_CAN;
     addr.can_ifindex = ifr.ifr_ifindex;
     ret = bind(CanFileP, (struct sockaddr *)&addr, sizeof(addr));
     if(ret<0)
     {
         prop_printf("Bind failed!\n");
         return -3;
     }

     // 设置CAN滤波器
     struct can_filter rfilter[1];
     rfilter[0].can_id= 0x00;
     rfilter[0].can_mask = 0x00;

     ret = setsockopt(CanFileP, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
     if(ret < 0)
     {
         prop_printf("Set sockopt failed!\n");
         return 4;
     }

     e_poll_add(CanFileP);
     canwrite.z_pthread_init(call_write_back, this);
//     start();
    prop_printf("init finish!\n");
     return 0;
 }

void CanDriver::run()
{
    printf("can driver start\n");
    struct epoll_event events[get_epoll_size()];
    char buf[sizeof(CanFrame)];

    for (;  ; )
    {
        memset(&events, 0, sizeof(events));
        if(wait_fd_change(5000) != -1)
        {
//            printf("receive can frame\n");
           while(read(CanFileP, buf,sizeof(CanFrame)) == sizeof(CanFrame))
           {
               canread.buf_write_data((CanFrame*)buf);
           }
        }
        else
        {
            printf("wati over!\n");
        }
    }
}

int CanDriver::writeframe(const CanFrame& f)
{
    int nbytes;
    int retry_times = 10;

    int total_write = sizeof(CanFrame);


    while(retry_times)
    {
        nbytes = ::write(CanFileP, &f, total_write);
        if(nbytes==total_write)
        {
//           timeprf("send successful %d!\n", f.can_dlc);
           break;
        }

        if(nbytes>0)
        {
            retry_times--;
             continue;
        }
        if(nbytes<0)
        {
//            printf("can bus write error %d!\n",errno);
//            printf("%s\n",strerror(errno));
            linuxDly(1);
            retry_times--;
            continue;
        }
    }
    if(retry_times==0)
    {
           printf("can bus may be full!");
    }
    return nbytes;
}

int CanDriver::writeframe(const CANDATAFORM& f)
{
    return writeframe(prodata_to_lawdata(f));
}

int CanDriver::write_send_data(CANDATAFORM  & Msg)
{
    canwrite.buf_write_data(Msg);
    return 1;
}




