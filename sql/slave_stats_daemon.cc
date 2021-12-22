#include "mysql.h"
#include "sql_base.h"
#include "sql_show.h"
#include "sql_string.h"
#include "global_threads.h"
#include "srv_session.h"
#include "my_time.h"
#include "rpl_slave.h"
#include "rpl_mi.h"
#include "slave_stats_daemon.h"

/*
 * The Slave stats daemon thread is responsible for
 * continuously sending lag statistics from slaves to masters
 */

ulong slave_stats_daemon_interval;

pthread_t slave_stats_daemon_thread;
mysql_mutex_t LOCK_slave_stats_daemon;
mysql_cond_t COND_slave_stats_daemon;

#ifdef HAVE_REPLICATION
static bool abort_slave_stats_daemon;

static bool connected_to_master = false;

/**
  Create and initialize the mysql object, and connect to the
  master.

  @retval true if connection successful
  @retval false otherwise.
*/
static int safe_connect_slave_stats_thread_to_master(MYSQL * &mysql)
{
  if (mysql != NULL) {
    mysql_close(mysql);
  }
  mysql = mysql_init(NULL);
  if (!mysql) {
    return false;
  }
  configure_master_connection_options(mysql, active_mi);

  char pass[MAX_PASSWORD_LENGTH + 1];
  int password_size = sizeof(pass);
  if (active_mi->get_password(pass, &password_size)) {
    return false;
  }

  if (!mysql_real_connect(mysql, active_mi->host, active_mi->get_user(), pass,
                          0, active_mi->port, 0, 0)) {
    return false;
  }
  return true;
}

pthread_handler_t handle_slave_stats_daemon(void *arg MY_ATTRIBUTE((unused)))
{
  THD *thd= NULL;
  int error = 0;
  struct timespec abstime;

  DBUG_ENTER("handle_slave_stats_daemon");

  pthread_detach_this_thread();

  slave_stats_daemon_thread = pthread_self();
  slave_stats_daemon_thread_in_use = 1;
  MYSQL *mysql = nullptr;
  while(true) {
    mysql_mutex_lock(&LOCK_slave_stats_daemon);
    set_timespec(abstime, slave_stats_daemon_interval);
    while ((!error || error == EINTR) && !abort_slave_stats_daemon) {
      /*
       slave_stats_daemon_interval is set to 0. Do not send stats to master.
       Wait until a signal is received either for aborting the thread or for
       updating slave_stats_daemon_interval.
      */
      if (slave_stats_daemon_interval == 0) {
        error = mysql_cond_wait(&COND_slave_stats_daemon, &LOCK_slave_stats_daemon);
      } else {
        /*
        wait for slave_stats_daemon_interval seconds before sending next set
        of slave lag statistics
        */
        error = mysql_cond_timedwait(&COND_slave_stats_daemon, &LOCK_slave_stats_daemon, &abstime);
      }
    }

    mysql_mutex_unlock(&LOCK_slave_stats_daemon);

    if (abort_slave_stats_daemon)
      break;

    if (error == ETIMEDOUT) {
      // Initialize connection thd, if not already done.
      if (thd == NULL) {
        my_thread_init();
        thd= new THD;
        THD_CHECK_SENTRY(thd);
        thd->thread_stack= (char*) &thd;
        my_net_init(thd->get_net(), 0);
        thd->store_globals();
      }

      // If not connected to current master, try connection. If not
      // successful, try again in next cycle
      if (!connected_to_master) {
        connected_to_master = safe_connect_slave_stats_thread_to_master(mysql);
        if (connected_to_master) {
          sql_print_information(
              "Slave Stats Daemon: connected to master '%s@%s:%d'",
              active_mi->get_user(), active_mi->host, active_mi->port);
        } else {
          sql_print_information(
              "Slave Stats Daemon: Couldn't connect to master '%s@%s:%d', "
              "will try again during next cycle, (Error: %s)",
              active_mi->get_user(), active_mi->host, active_mi->port, mysql_error(mysql));
        }
      }
      if (connected_to_master &&
          active_mi->slave_running == MYSQL_SLAVE_RUN_CONNECT) {
        if(send_replica_statistics_to_master(mysql, active_mi)) {
          sql_print_warning("Slave Stats Daemon: Failed to send lag "
                            "statistics, resetting connection, (Error: %s)",
                            mysql_error(mysql));
          connected_to_master = false;
        }
      }
      error = 0;
    }
  }
  mysql_close(mysql);
  mysql = nullptr;
  connected_to_master = false;
  if (thd != NULL) {
    net_end(thd->get_net());
    thd->release_resources();
    delete (thd);
  }
  // DBUG_LEAVE; // Can't use DBUG_RETURN after my_thread_end
  my_thread_end();
  ERR_remove_state(0);
  slave_stats_daemon_thread_in_use = 0;
  pthread_exit(0);
  return (NULL);
}

/* Start handle Slave Stats Daemon thread */
void start_handle_slave_stats_daemon() {
  DBUG_ENTER("start_handle_slave_stats_daemon");
  abort_slave_stats_daemon = false;
  pthread_t hThread;
  int error;
  if ((error = mysql_thread_create(key_thread_handle_slave_stats_daemon,
                                   &hThread, &connection_attrib,
                                   handle_slave_stats_daemon, 0)))
    sql_print_warning(
        "Can't create handle_slave_stats_daemon thread (errno= %d)", error);
  DBUG_VOID_RETURN;
}

/* Initiate shutdown of handle Slave Stats Daemon thread */
void stop_handle_slave_stats_daemon() {
  DBUG_ENTER("stop_handle_slave_stats_daemon");
  abort_slave_stats_daemon = true;
  mysql_mutex_lock(&LOCK_slave_stats_daemon);
  if (slave_stats_daemon_thread_in_use) {
    DBUG_PRINT("quit",
               ("initiate shutdown of handle Slave Stats Daemon thread: 0x%lx",
                (ulong)slave_stats_daemon_thread));
    mysql_cond_signal(&COND_slave_stats_daemon);
  }
  mysql_mutex_unlock(&LOCK_slave_stats_daemon);
  while(slave_stats_daemon_thread_in_use) {
    // wait for the thread to finish, sleep for 10ms
    my_sleep(10000);
  }
  DBUG_VOID_RETURN;
}

#endif
