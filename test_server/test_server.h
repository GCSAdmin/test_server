#ifndef TEST_SERVER_H
#define TEST_SERVER_H


#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/***********  network   ***********/
#include <glib.h>
#include <errno.h>

#define NETWORK_SUCCESS         0

#define NETWORK_ERROR           -1
#define NETWORK_ADDR_ERROR      -2
#define NETWORK_SOCKET_ERROR    -3 

#ifndef _WIN32
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>     /** struct sockaddr_in */
#endif
#include <netinet/tcp.h>
#include <netdb.h>

#define closesocket(x) close(x)

#ifdef HAVE_SYS_UN_H
#include <sys/un.h>         /** struct sockaddr_un */
#endif
#include <sys/socket.h>     /** struct sockaddr (freebsd and hp/ux need it) */
#else
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <errno.h>
#ifndef SHUT_RDWR
#define SHUT_RDWR 0x02
#endif

#endif

/*  mysql  */
#include <mysql.h>
#include <my_global.h>
#include <my_sys.h>

#define MAX_STRLEN 128

#define SQL_TYPE_UNKNOW 0
#define SQL_TYPE_SELECT 1
#define SQL_TYPE_INSERT 2
#define SQL_TYPE_UPDATE 3
#define SQL_TYPE_DELETE 4

struct global_info_stuct 
{
    gchar*      host;       
    gint        port;      
    gchar*      username;   
    gchar*      password;   
    gint        concurrency;
    gint        time;       
    gint        table_cnt;
    gchar*      tablename;
    gchar**     table_arr;
    gchar*      sql;
    gchar**     sql_arr;
    gint*       sql_type_arr;
    gchar*      ratio_str;
    gint*       ratio_arr;      /* 对应sql的比例，ratio_arr[table_cnt - 1]表示比例总数 */
    gint        is_select;
    gint        percent;
    gchar*      max_pk_value_str;
    gint*       max_pk_value_arr;
    gint        interval;
    gint        wait_time;
    gint        is_warn;
};


#ifdef _WIN32
#define my_sleep(cnt) Sleep(cnt * 1000)
#else
#define my_sleep(cnt) sleep(cnt)
#endif // _WIN32


#endif