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
    {"--max_pk_value",          'n', 0, G_OPTION_ARG_STRING,    &global_info.max_pk_value_str,  "listen port(default 33060) .", NULL},
    {"--interval",              'i', 0, G_OPTION_ARG_INT,       &global_info.interval,  "listen port(default 33060) .", NULL},
    {"--wait_time",             'w', 0, G_OPTION_ARG_INT,       &global_info.wait_time,  "listen port(default 33060) .", NULL},
    {"--warn",                  'W', 0, G_OPTION_ARG_INT,       &global_info.is_warn,  "listen port(default 33060) .", NULL},
    {"--ratio",                 'r', 0, G_OPTION_ARG_STRING,    &global_info.ratio_str,          "listen port(default 33060) .", NULL},
    {"--sql",                   's', 0, G_OPTION_ARG_STRING,    &global_info.sql,          "listen port(default 33060) .", NULL},
    {"--max_table_id",          'm', 0, G_OPTION_ARG_INT,       &global_info.max_table_id, "max table id(default 0) .", NULL},
    {"--sleep",                 'S', 0, G_OPTION_ARG_INT,       &global_info.sleep, "each query sleep millisecond .", NULL},
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
-T, --tablename             test table name, must be set, maybe multi(t1;t2;t3)\n\
-n, --max_pk_value          test table max pk value(digit), must be set, maybe multi(n1;n2;n3)\n\
-e, --percent               percent of test pk value(1-100), default 100\n\
-i, --interval              collect reslut interval, default 10\n\
-W, --warn                  whether execute select count(*) from tbl to warn data, default 1\n\
-w, --wait_time             wait time before measure, default 60\n\
-m, --max_table_id          talbename must contain '%d', and tablename convert to tablename_0 ~ table_name_max_table_id\n\
-s, --sleep          		each query sleep millisecond\n\
-r, --ratio                 ratio of sql, like 5:1:1\n\
-s, --sql                   test sql, like select * from t1 where id = ?, must be set, maybe multi(like sql1;sql2;sql3)\n\n";

#ifndef _WIN32
#include <signal.h>
void dump(int signo)
{
	char buf[1024];
	char cmd[2024];

	FILE *fh;

	snprintf(buf, sizeof(buf), "/proc/%d/cmdline", getpid());
	if (!(fh = fopen(buf, "r")))
		exit(0);
	if (!fgets(buf, sizeof(buf), fh))
		exit(0);

	fclose(fh);

	if(buf[strlen(buf)-1] == '\n')
		buf[strlen(buf)-1] = '\0';

	snprintf(cmd, sizeof(cmd), "gdb %s %d", buf, getpid());
	system(cmd);

	exit(0);
}


#endif

gboolean
g_str_has_prefix_ignore_space_and_case(
    gchar*          str,
    gchar*          prefix
)
{
    gchar*      p = str;

    while (p)
    {
        if (!g_ascii_isspace(*p))
        {
            break;
        }

        p++;
    }

    if (!p)
        return 0;

    return !g_ascii_strncasecmp(p, prefix, strlen(prefix));
    
}

int
test_server_global_init()
{ 
    gchar**         table_arr;
    gchar**         sql_arr;
    gchar**         ratio_str_arr = NULL;
    gchar**         num_arr;
    gint            table_cnt;
    gint            sql_cnt, ratio_cnt, num_cnt;
    gint            i;
    gint            total_ratio = 0;

    if (!global_info.tablename || !global_info.sql || !global_info.max_pk_value_str)
    {
        fprintf(stderr, "%s tablename, sql, max_pk_value option must be set \n",
                G_STRLOC);
        return -1;
    }

    table_arr = g_strsplit(global_info.tablename, ";", 0);
    sql_arr = g_strsplit(global_info.sql, ";", 0);
    num_arr = g_strsplit(global_info.max_pk_value_str, ";", 0);

    table_cnt = g_strv_length(table_arr);
    sql_cnt = g_strv_length(sql_arr);
    num_cnt = g_strv_length(num_arr);

    if (global_info.ratio_str)
    {
        ratio_str_arr = g_strsplit(global_info.ratio_str, ":", 0);
        ratio_cnt = g_strv_length(ratio_str_arr);
    }
    else
    {
        ratio_cnt = table_cnt;
    }
    
    if (table_cnt != sql_cnt || table_cnt != num_cnt || table_cnt != ratio_cnt)
    {
        fprintf(stderr, "%s number of element of tablename, sql, max_pk_value must be same \n",
            G_STRLOC);
        return -1;
    }

    /* table_name 模糊匹配 */
    if (global_info.max_table_id)
    {
        if (table_cnt != 1 || !strstr(global_info.tablename, "%d") || !strstr(global_info.sql, "%d"))
        {
            fprintf(stderr, "max_table_id has set, table/sql num must be 1 and contains '%d'\n",
                G_STRLOC);
            return -1;
        }

        global_info.table_cnt = global_info.max_table_id + 1;

        global_info.table_arr = g_new0(gchar*, global_info.table_cnt + 1);
        global_info.sql_arr = g_new0(gchar*, global_info.table_cnt + 1);
        for (i = 0; i < global_info.table_cnt; ++i)
        {
            global_info.table_arr[i] = g_strdup_printf(global_info.tablename, i);
            global_info.sql_arr[i] = g_strdup_printf(global_info.sql, i);
        }
        
        g_strfreev(table_arr);
        g_strfreev(sql_arr);
    }
    else
    {
        global_info.table_cnt = table_cnt;
        global_info.table_arr = table_arr;
        global_info.sql_arr = sql_arr;
    }
    
    global_info.sql_type_arr = g_new0(int, global_info.table_cnt + 1);
    for (i = 0; i < global_info.table_cnt; ++i)
    {
        if (g_str_has_prefix_ignore_space_and_case(global_info.sql_arr[i], "SELECT"))
        {
            global_info.sql_type_arr[i] = SQL_TYPE_SELECT;
        }
        else if (g_str_has_prefix_ignore_space_and_case(global_info.sql_arr[i], "INSERT"))
        {
            global_info.sql_type_arr[i] = SQL_TYPE_INSERT;
        }
        else if (g_str_has_prefix_ignore_space_and_case(global_info.sql_arr[i], "UPDATE"))
        {
            global_info.sql_type_arr[i] = SQL_TYPE_UPDATE;
        } 
        else if (g_str_has_prefix_ignore_space_and_case(global_info.sql_arr[i], "DELETE"))
        {
            global_info.sql_type_arr[i] = SQL_TYPE_DELETE;
        }
        else
        {
            global_info.sql_type_arr[i] = SQL_TYPE_UNKNOW;

            fprintf(stderr, "%s unknow sql type %s \n",
                G_STRLOC, global_info.sql_arr[i]);
            return -1;
        }
    }

    global_info.max_pk_value_arr = g_new0(int, global_info.table_cnt + 1);
    for (i = 0; i < global_info.table_cnt; ++i)
    {
        if (global_info.max_table_id)
        {
            global_info.max_pk_value_arr[i] = atoi(num_arr[0]);
        }
        else
            global_info.max_pk_value_arr[i] = atoi(num_arr[i]);

        if (global_info.max_pk_value_arr[i] <= 0)
        {
            fprintf(stderr, "%s invalid max_pk_value\n",
                G_STRLOC);
            return -1;
        }
    }

    g_strfreev(num_arr);

    global_info.ratio_arr = g_new0(int, global_info.table_cnt + 1);
    
    for (i = 0; i < global_info.table_cnt; ++i)
    {
        
        if (ratio_str_arr)
        {
            gint r = atoi(ratio_str_arr[i]);

            if (r <= 0)
            {
                fprintf(stderr, "%s invalid ratio\n",
                    G_STRLOC);
                return -1;
            }

            total_ratio += r;
        }
        else
        {
            /* 如果没设定该参数，所有比例是1:1 */
            total_ratio += 1;
        }
        
        global_info.ratio_arr[i] = total_ratio;
    }

    g_strfreev(ratio_str_arr);

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
    g_free(global_info.sql);

    g_strfreev(global_info.sql_arr);

    if (global_info.ratio_str != NULL)
        g_free(global_info.ratio_str);

    g_free(global_info.ratio_arr);

    if (global_info.tablename)
        g_free(global_info.tablename);

    g_strfreev(global_info.table_arr);

    g_free(global_info.max_pk_value_str);
    g_free(global_info.max_pk_value_arr);

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
    MYSQL_STMT**stmt_arr = NULL;
    MYSQL_STMT* stmt = NULL;
    int         rand_id = 0;
    int         rand_tbl_id = 0;
    int         i;
    int         conn_cnt = 0;

    conn = mysql_init(NULL);

    if (conn == NULL)
    {
        fprintf(stderr, "%s: mysql_init fail", G_STRLOC);
        errnum = -1;
        goto sqlerr;
    }

    /* 直到连接成功为止 */
connect_again:
    if (mysql_real_connect(conn, global_info.host, global_info.username, global_info.password, NULL, global_info.port, NULL, 0) == NULL)
    {
        fprintf(stderr, "%s: mysql_real_connect fail\n", G_STRLOC);
        mysql_print_error(conn, NULL);
        if (conn_cnt++ < 100)
        	goto connect_again;
		else
			goto sqlerr;
    }

    stmt_arr = g_new0(MYSQL_STMT*, global_info.table_cnt + 1);

    for (i = 0; i < global_info.table_cnt; ++i)
    {
        stmt = mysql_stmt_init(conn);

        if (mysql_stmt_prepare(stmt, global_info.sql_arr[i], strlen(global_info.sql_arr[i])) )
        {
            mysql_print_error(conn, NULL);
            goto sqlerr;
        }

        stmt_arr[i] = stmt;
    }
    
    /* 防止多个线程相同种子 */
    srand(time(NULL) + num_of_thread * 10000);
    while (1)
    {
        int r_id = rand() % global_info.ratio_arr[global_info.table_cnt - 1];

        for (i =0; i < global_info.table_cnt; ++i)
        {
            if (r_id < global_info.ratio_arr[i])
            {
                rand_tbl_id = i;
                break;
            }
        }

        stmt = stmt_arr[rand_tbl_id];

        /* generate the random num  */
        rand_id = rand() % global_info.max_pk_value_arr[rand_tbl_id] + 1;

        memset(&param, 0, sizeof(MYSQL_BIND)); /* initialize */
        param.buffer_type = MYSQL_TYPE_LONG;
        param.buffer = &rand_id;

        //if (mysql_stmt_prepare(stmt, global_info.sql_arr[rand_tbl_id], strlen(global_info.sql_arr[rand_tbl_id])) )
        //{
        //    mysql_print_error(conn, NULL);
        //    goto sqlerr;
        //}

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

        if (global_info.sql_type_arr[rand_tbl_id] == SQL_TYPE_SELECT)
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

        if (global_info.sleep)
        {
            my_msleep(global_info.sleep * 1000);
        }
    }

sqlerr:
    fprintf(stderr, "%s: something error while executing %s, id = %d, thread num %d, conn_cnt %d\n", 
        G_STRLOC, global_info.sql_arr[rand_tbl_id], rand_id, num_of_thread, conn_cnt);

    for (i = 0; i < global_info.table_cnt; ++i)
    {
        stmt = stmt_arr[i];
        if (stmt)
            mysql_stmt_close(stmt);

    }

    g_free(stmt_arr);
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
    int         i = 0;

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

    for (i = 0; i < global_info.table_cnt; ++i)
    {
        sprintf(buff, "select count(*) from %s", global_info.table_arr[i]);

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
	
#ifndef _WIN32
    signal(SIGSEGV, &dump);
    signal(SIGFPE, &dump);
#endif
    /*** 读取配置信息 ***/
    if (argc == 2 &&
        (strcmp(argv[1], "--help") == 0 ||
        strcmp(argv[1], "-h") == 0))
    {
        printf("%s", help_str);
        exit(-1);
    }

    global_info.is_warn = 1;

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
[max_pk_value] : %s,\n\
[percent] : %d,\n\
[interval] : %d,\n\
[wait_time] : %d,\n\
[max_table_id] : %d,\n\
[sleep] : %d,\n\
[warn] : %d,\n\
[ratio] : %s,\n\
[sql] : %s\n\n", global_info.host, global_info.port, global_info.username, 
                global_info.password, global_info.concurrency, global_info.time, global_info.tablename,
                 global_info.max_pk_value_str, global_info.percent, global_info.interval, global_info.wait_time, global_info.max_table_id, global_info.sleep, 
                 global_info.is_warn, global_info.ratio_str, global_info.sql);
    
    if (global_info.is_warn)
    {
        time_t      t;

        printf("Start warn data");

        t = time(NULL);
        if (test_server_warn_data())
            exit(-1);

        printf(", execute time :%d\n", time(NULL) - t);
    }
    else
    {
        printf("don't warn data\n");        
    }
    
    arg_arr = (gint*)g_malloc0(sizeof(gint) * global_info.concurrency);

    for (i = 0; i < global_info.concurrency; ++i)
    {
        arg_arr[i] = i;
        worker_thread = g_thread_create((GThreadFunc)test_server_worker_thread, (gpointer)&arg_arr[i], TRUE, &gerr); 
#ifndef _WIN32 
		usleep(10);
#endif
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

    printf("AVG QPS: %f\n", (float)total_cnt / global_info.time );
    fflush(stdout);

    g_free(arg_arr);

    test_server_deinit();

    return 0;
}
