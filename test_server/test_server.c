//for memory leak test
#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif // _WIN32


#include "test_server.h"

struct global_info_stuct global_info;

const GOptionEntry arg_options[] ={
    {"--host",                  'h', 0, G_OPTION_ARG_STRING,    &global_info.host,        "listen address(default 0.0.0.0) .", NULL},
    {"--port",                  'P', 0, G_OPTION_ARG_INT,       &global_info.port,      "listen port(default 33060) .", NULL},
    {"--username",              'u', 0, G_OPTION_ARG_STRING,    &global_info.username,      "listen port(default 33060) .", NULL},
    {"--password",              'p', 0, G_OPTION_ARG_STRING,    &global_info.password,      "listen port(default 33060) .", NULL},
    {"--concurrnecy",           'c', 0, G_OPTION_ARG_INT,       &global_info.concurrency,      "listen port(default 33060) .", NULL},
    {"--time",                  't', 0, G_OPTION_ARG_INT,       &global_info.time,          "listen port(default 33060) .", NULL},
    {"--tablename",             'T', 0, G_OPTION_ARG_STRING,    &global_info.tablename,          "listen port(default 33060) .", NULL},
    {"--percent",               'e', 0, G_OPTION_ARG_INT,       &global_info.percent,          "listen port(default 33060) .", NULL},
    {"--max_pk_value",          'n', 0, G_OPTION_ARG_INT,       &global_info.max_pk_value,  "listen port(default 33060) .", NULL},
    {"--interval",              'i', 0, G_OPTION_ARG_INT,       &global_info.interval,  "listen port(default 33060) .", NULL},
    {"--wait_time",             'w', 0, G_OPTION_ARG_INT,       &global_info.wait_time,  "listen port(default 33060) .", NULL},
    {"--sql",                   's', 0, G_OPTION_ARG_STRING,    &global_info.sql,          "listen port(default 33060) .", NULL},
    {NULL,                      0,   0, 0,                      NULL,                   NULL, NULL},
}; 

gchar* help_str = 
"\ntest server startup options.\n\
-h, --host                  mysqld host(default localhost),\n\
-P, --port                  mysqld port(default 3306),\n\
-u, --username              username(default root)\n\
-p, --password              password(default '')\n\
-c, --concurrnecy           concurrnecy of test server(default 10)\n\
-t, --time                  test run time(seconds)(default 60s)\n\
-T, --tablename             test table name, must be set\n\
-n, --max_pk_value          test table max pk value(digit), must be set\n\
-e, --percent               percent of test pk value(1-100), default 100\n\
-i, --interval              collect reslut interval, default 10\n\
-w, --wait_time             wait time before measure, default 60\n\
-s, --sql                   test sql, like select * from t1 where id = ?, must be set\n\n";

int
test_server_global_init()
{ 
    if (!global_info.tablename || !global_info.sql || global_info.max_pk_value == 0)
    {
        fprintf(stderr, "%s tablename, sql, max_pk_value option must be set \n",
                G_STRLOC);
        return -1;
    }

    if (global_info.wait_time == 0)
        global_info.wait_time = 60;

    if (global_info.interval == 0)
        global_info.interval = 10;

    if (global_info.percent == 0)
        global_info.percent = 100;

    if (!global_info.host)
        global_info.host = g_strdup("localhost");

    if (global_info.port == 0)
        global_info.port = 3306;

    if (!global_info.username)
        global_info.username = g_strdup("root");

    if (!global_info.password)
        global_info.password = g_strdup("");

    if (global_info.concurrency == 0)
        global_info.concurrency = 10;

    if (global_info.time == 0)
        global_info.time = 60;

    return 0; 
}

void
test_server_global_deinit()
{
    if (global_info.sql != NULL)
        g_free(global_info.sql);

    if (global_info.tablename)
        g_free(global_info.tablename);

    if (global_info.host)
        g_free(global_info.host);

    if (global_info.username)
        g_free(global_info.username);

    if (global_info.password)
        g_free(global_info.password);
}


int 
test_server_parse_options(
    int*            argc_p, 
    char***         argv_p
) 
{
    int ret = 0;

    GError* gerr = NULL;

    GOptionContext * opt_ctx = g_option_context_new("- Test Server");

    g_option_context_add_main_entries(opt_ctx, arg_options, NULL);

    g_option_context_set_help_enabled(opt_ctx, FALSE);

    g_option_context_set_ignore_unknown_options(opt_ctx, FALSE);

    if (FALSE == g_option_context_parse(opt_ctx, argc_p, argv_p, &gerr))
    {
        if (gerr != NULL)
        {
            fprintf(stderr, "%s test_server_parse_options fail, reason %s \n",
                G_STRLOC, gerr->message);

            g_error_free(gerr);
        }

        return -1;
    }

    g_option_context_free(opt_ctx);

    return 0;
}


gint* g_runcnt_array = NULL;

int
test_server_init()
{
    const gchar*      check_str;

    if (!GLIB_CHECK_VERSION(2, 6, 0)) {
        fprintf(stderr, "the glib header are too old, need at least 2.6.0, got: %d.%d.%d", 
            GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION);

        return -1;
    }

    check_str = glib_check_version(GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION);

    if (check_str) {
        fprintf(stderr, "%s, got: lib=%d.%d.%d, headers=%d.%d.%d", 
            check_str,
            glib_major_version, glib_minor_version, glib_micro_version,
            GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION);

        return -1;
    }

    g_thread_init(NULL);

    if (test_server_global_init())
        return -1;

    g_runcnt_array = (gint*)g_malloc(sizeof(gint) * global_info.concurrency);
    memset(g_runcnt_array, 0, sizeof(gint) * global_info.concurrency);

    return 0;
}


void
test_server_deinit()
{

    test_server_global_deinit();

    g_free(g_runcnt_array);

}


gint g_count_on = 0;


void
mysql_print_error(
    MYSQL*      conn,
    char*       message
)
{
    if (message != NULL)
    {
        fprintf(stderr, "%s\n", message);
    }

    if (conn != NULL)
    {
        fprintf(stderr, "Error %u (%s), %s\n",
                    mysql_errno(conn), mysql_sqlstate(conn), mysql_error(conn));
    }
}

gpointer
test_server_worker_thread(
    void*       user_arg
)
{
    gint num_of_thread = *(gint*)user_arg;

    MYSQL*      conn;
    int         errnum = 0;
    MYSQL_BIND  param;
    MYSQL_STMT* stmt = NULL;
    int         rand_id = 0;

    conn = mysql_init(NULL);

    if (conn == NULL)
    {
        fprintf(stderr, "%s: mysql_init fail", G_STRLOC);
        errnum = -1;
        goto sqlerr;
    }

connect_again:
    if (mysql_real_connect(conn, global_info.host, global_info.username, global_info.password, NULL, global_info.port, NULL, 0) == NULL)
    {
        fprintf(stderr, "%s: mysql_real_connect fail\n", G_STRLOC);
        mysql_print_error(conn, NULL);
        goto connect_again;
    }

    stmt = mysql_stmt_init(conn);

    if (mysql_stmt_prepare(stmt, global_info.sql, strlen(global_info.sql)) )
    {
        mysql_print_error(conn, NULL);
        goto sqlerr;
    }

    /* 防止多个线程相同种子 */
    srand(time(NULL) + num_of_thread);
    while (1)
    {
        /* generate the random num  */
        rand_id = rand() % global_info.max_pk_value + 1;

        memset(&param, 0, sizeof(MYSQL_BIND)); /* initialize */
        param.buffer_type = MYSQL_TYPE_LONG;
        param.buffer = &rand_id;

        if( mysql_stmt_bind_param(stmt, &param) )
        {
            mysql_print_error(conn, NULL);
            goto sqlerr;
        }

        if( mysql_stmt_execute(stmt) )
        {
            mysql_print_error(conn, NULL);
            goto sqlerr;
        }

        if (global_info.is_select)
        {
            if( mysql_stmt_store_result(stmt) )
            {
                mysql_print_error(conn, NULL);
                goto sqlerr;
            }
            
            mysql_stmt_free_result(stmt);
        }

        if (g_count_on)
            g_runcnt_array[num_of_thread]++;
    }

sqlerr:
    fprintf(stderr, "%s: something error while executing %s, id = %d, thread num %d", G_STRLOC, global_info.sql, rand_id, num_of_thread);
    if (stmt)
        mysql_stmt_close(stmt);
    mysql_close(conn);

    return NULL;
}

int
test_server_warn_data()
{
    MYSQL*      conn;
    MYSQL_RES*  res_set;
    gchar       buff[1024];
    int         errnum = 0;

    conn = mysql_init(NULL);

    if (conn == NULL)
    {
        fprintf(stderr, "%s: mysql_init fail", G_STRLOC);
        errnum = -1;
        goto err;
    }

    if (mysql_real_connect(conn, global_info.host, global_info.username, global_info.password, NULL, global_info.port, NULL, 0) == NULL)
    {
        fprintf(stderr, "%s: mysql_real_connect fail", G_STRLOC);
        errnum = -1;
        goto err;
    }

    sprintf(buff, "select count(*) from %s", global_info.tablename);

    if (mysql_query(conn, buff))
    {
        fprintf(stderr, "%s: mysql_real_connect %s fail", G_STRLOC, buff);
        errnum = -1;
        goto err;
    }
    else
    {
        res_set = mysql_store_result(conn);

        if (res_set == NULL)
        {
            fprintf(stderr, "%s: mysql_store_result %s fail", G_STRLOC, buff);
            errnum = -1;
            goto err;
        }
    }

err:
    mysql_close(conn);

    return errnum;
}


int 
main(
     int         argc, 
     char        **argv
)
{
    GError*             gerr = NULL;
    GThread*            worker_thread;
    gint                i = 0;
    gint                j = 0;
    gint                prev_total_cnt = 0;
    gint                total_cnt = 0;
    gint*               arg_arr = NULL;

    /*** 读取配置信息 ***/
    if (argc == 2 &&
        (strcmp(argv[1], "--help") == 0 ||
        strcmp(argv[1], "-h") == 0))
    {
        printf("%s", help_str);
        exit(-1);
    }

    if (test_server_parse_options(&argc, &argv))
    {
        exit(-1);
    }

    if (test_server_init())
    {
        exit(-1);
    }

    printf("[host] : %s,\n\
[port] : %d,\n\
[username] : %s,\n\
[password] : %s,\n\
[concurrency] : %d,\n\
[time] : %d(s),\n\
[tablename] : %s,\n\
[max_pk_value] : %d,\n\
[percent] : %d,\n\
[interval] : %d,\n\
[wait_time] : %d,\n\
[sql] : %s\n\n", global_info.host, global_info.port, global_info.username, 
                global_info.password, global_info.concurrency, global_info.time, global_info.tablename,
                 global_info.max_pk_value, global_info.percent, global_info.interval, global_info.wait_time, global_info.sql);
    
    printf("Start warn data\n");
    if (test_server_warn_data())
        exit(-1);
    
    arg_arr = (gint*)g_malloc0(sizeof(gint) * global_info.concurrency);

    for (i = 0; i < global_info.concurrency; ++i)
    {
        arg_arr[i] = i;
        worker_thread = g_thread_create((GThreadFunc)test_server_worker_thread, (gpointer)&arg_arr[i], TRUE, &gerr); 
    }

    /* wait thread to start and create the connections */
    my_sleep(global_info.wait_time);
    
    printf("Start Measure\n");
    fflush(stdout);

    g_count_on = 1;

    for (i = 0; i < global_info.time / global_info.interval; ++i)
    {
        total_cnt = 0;
        my_sleep(global_info.interval);

        
        for (j = 0; j < global_info.concurrency; ++j)
        {
            total_cnt += g_runcnt_array[j];     
        }

        printf("%d\n", total_cnt - prev_total_cnt);
        fflush(stdout);

        prev_total_cnt = total_cnt;
    }

    g_count_on = 0;

    for (i = 0; i < global_info.concurrency; ++i)
    {
        total_cnt += g_runcnt_array[j];
    }

    printf("AVG QPS: %f\n", (float)total_cnt / global_info.time );
    fflush(stdout);

    g_free(arg_arr);

    test_server_deinit();

    return 0;
}
