/* Copyright (c) 2016, Facebook. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */


#include "sql_acl.h"
#include "sql_priv.h"
#include "sql_show.h"
#include "sql_multi_tenancy.h"
#include "global_threads.h"
#include "handler.h"
#include "m_string.h"

#ifndef EMBEDDED_LIBRARY


/*
  @param sql_command command the thread is currently executing

  @return true Skips the current query in admission control
          false Admission control checks are applied for this query
*/
static bool filter_command(enum_sql_command sql_command)
{
  switch (sql_command) {
    case SQLCOM_ALTER_TABLE:
    case SQLCOM_ALTER_DB:
    case SQLCOM_ALTER_PROCEDURE:
    case SQLCOM_ALTER_FUNCTION:
    case SQLCOM_ALTER_TABLESPACE:
    case SQLCOM_ALTER_SERVER:
    case SQLCOM_ALTER_EVENT:
    case SQLCOM_ALTER_DB_UPGRADE:
    case SQLCOM_ALTER_USER:
    case SQLCOM_RENAME_TABLE:
    case SQLCOM_RENAME_USER:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_ALTER);

    case SQLCOM_BEGIN:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_BEGIN);

    case SQLCOM_COMMIT:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_COMMIT);

    case SQLCOM_CREATE_TABLE:
    case SQLCOM_CREATE_INDEX:
    case SQLCOM_CREATE_DB:
    case SQLCOM_CREATE_FUNCTION:
    case SQLCOM_CREATE_NPROCEDURE:
    case SQLCOM_CREATE_USER:
    case SQLCOM_CREATE_PROCEDURE:
    case SQLCOM_CREATE_SPFUNCTION:
    case SQLCOM_CREATE_VIEW:
    case SQLCOM_CREATE_TRIGGER:
    case SQLCOM_CREATE_SERVER:
    case SQLCOM_CREATE_EVENT:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_CREATE);

    case SQLCOM_DELETE:
    case SQLCOM_DELETE_MULTI:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_DELETE);

    case SQLCOM_DROP_TABLE:
    case SQLCOM_DROP_INDEX:
    case SQLCOM_DROP_DB:
    case SQLCOM_DROP_FUNCTION:
    case SQLCOM_DROP_NPROCEDURE:
    case SQLCOM_DROP_USER:
    case SQLCOM_DROP_PROCEDURE:
    case SQLCOM_DROP_VIEW:
    case SQLCOM_DROP_TRIGGER:
    case SQLCOM_DROP_SERVER:
    case SQLCOM_DROP_EVENT:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_DROP);

    case SQLCOM_INSERT:
    case SQLCOM_INSERT_SELECT:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_INSERT);

    case SQLCOM_LOAD:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_LOAD);

    case SQLCOM_SELECT:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_SELECT);

    case SQLCOM_SET_OPTION:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_SET);

    case SQLCOM_REPLACE:
    case SQLCOM_REPLACE_SELECT:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_REPLACE);

    case SQLCOM_ROLLBACK:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_ROLLBACK);

    case SQLCOM_TRUNCATE:
      return IS_BIT_SET(admission_control_filter,  ADMISSION_CONTROL_TRUNCATE);

    case SQLCOM_UPDATE:
    case SQLCOM_UPDATE_MULTI:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_UPDATE);

    case SQLCOM_SHOW_DATABASES:
    case SQLCOM_SHOW_TABLES:
    case SQLCOM_SHOW_FIELDS:
    case SQLCOM_SHOW_KEYS:
    case SQLCOM_SHOW_VARIABLES:
    case SQLCOM_SHOW_STATUS:
    case SQLCOM_SHOW_ENGINE_LOGS:
    case SQLCOM_SHOW_ENGINE_STATUS:
    case SQLCOM_SHOW_ENGINE_MUTEX:
    case SQLCOM_SHOW_PROCESSLIST:
    case SQLCOM_SHOW_TRANSACTION_LIST:
    case SQLCOM_SHOW_MASTER_STAT:
    case SQLCOM_SHOW_SLAVE_STAT:
    case SQLCOM_SHOW_GRANTS:
    case SQLCOM_SHOW_CREATE:
    case SQLCOM_SHOW_CHARSETS:
    case SQLCOM_SHOW_COLLATIONS:
    case SQLCOM_SHOW_CREATE_DB:
    case SQLCOM_SHOW_TABLE_STATUS:
    case SQLCOM_SHOW_TRIGGERS:
    case SQLCOM_SHOW_BINLOGS:
    case SQLCOM_SHOW_OPEN_TABLES:
    case SQLCOM_SHOW_SLAVE_HOSTS:
    case SQLCOM_SHOW_BINLOG_EVENTS:
    case SQLCOM_SHOW_BINLOG_CACHE:
    case SQLCOM_SHOW_WARNS:
    case SQLCOM_SHOW_ERRORS:
    case SQLCOM_SHOW_STORAGE_ENGINES:
    case SQLCOM_SHOW_PRIVILEGES:
    case SQLCOM_SHOW_CREATE_PROC:
    case SQLCOM_SHOW_CREATE_FUNC:
    case SQLCOM_SHOW_STATUS_PROC:
    case SQLCOM_SHOW_STATUS_FUNC:
    case SQLCOM_SHOW_PROC_CODE:
    case SQLCOM_SHOW_FUNC_CODE:
    case SQLCOM_SHOW_PLUGINS:
    case SQLCOM_SHOW_CREATE_EVENT:
    case SQLCOM_SHOW_EVENTS:
    case SQLCOM_SHOW_CREATE_TRIGGER:
    case SQLCOM_SHOW_PROFILE:
    case SQLCOM_SHOW_PROFILES:
    case SQLCOM_SHOW_RELAYLOG_EVENTS:
    case SQLCOM_SHOW_ENGINE_TRX:
    case SQLCOM_SHOW_MEMORY_STATUS:
    case SQLCOM_SHOW_CONNECTION_ATTRIBUTES:
    case SQLCOM_SHOW_RESOURCE_COUNTERS:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_SHOW);

    case SQLCOM_CHANGE_DB:
      return IS_BIT_SET(admission_control_filter, ADMISSION_CONTROL_USE);

    default:
      return false;
  }
}


/*
 * Add a connection in multi-tenancy plugin
 *
 * @param thd THD structure
 * @param attrs Resource attributes
 *
 * @return 0 if the connection is added, 1 if rejected
 */
int multi_tenancy_add_connection(THD *thd, const MT_RESOURCE_ATTRS *attrs)
{
  int ret = 0;
  st_mysql_multi_tenancy *data= NULL;

  if (thd->variables.multi_tenancy_plugin)
  {
    mysql_mutex_lock(&thd->LOCK_thd_data);
    data = plugin_data(thd->variables.multi_tenancy_plugin,
                       struct st_mysql_multi_tenancy *);
    ret = (int) data->request_resource(
        thd,
        MT_RESOURCE_TYPE::MULTI_TENANCY_RESOURCE_CONNECTION,
        attrs);
    mysql_mutex_unlock(&thd->LOCK_thd_data);

    // plugin is turned off
    if (ret == (int) MT_RETURN_TYPE::MULTI_TENANCY_RET_FALLBACK)
      ret = 0;
  }

  return ret;
}


/*
 * Release a connection in multi-tenancy plugin
 *
 * @param thd THD structure
 * @param attrs Resource attributes
 *
 * @return 0 if successful, 1 if otherwise
 */
int multi_tenancy_close_connection(THD *thd, const MT_RESOURCE_ATTRS *attrs)
{
  st_mysql_multi_tenancy *data= NULL;
  if (thd->variables.multi_tenancy_plugin)
  {
    mysql_mutex_lock(&thd->LOCK_thd_data);
    data = plugin_data(thd->variables.multi_tenancy_plugin,
                       struct st_mysql_multi_tenancy *);
    data->release_resource(
        thd,
        MT_RESOURCE_TYPE::MULTI_TENANCY_RESOURCE_CONNECTION,
        attrs);
    mysql_mutex_unlock(&thd->LOCK_thd_data);
  }

  return 0;
}

/*
 * Admit a query based on database entity
 *
 * @param thd THD structure
 * @param attrs Resource attributes
 *
 * @return 0 if the query is admitted, 1 otherwise.
 */
int multi_tenancy_admit_query(THD *thd, const MT_RESOURCE_ATTRS *attrs)
{
  bool admission_check= false;

  /*
   * Admission control check will be enforced if ALL of the following
   * conditions are satisfied
   *  1. The query is run by regular (non-super) user
   *  2. The THD is not a replication thread
   *  3. The query is not part of a transaction (controlled by global var
   *     opt_admission_control_by_trx), nor the THD is already in an admission
   *     control (e.g. part of a multi query packet)
   *  4. Session database is set for THD
   *  5. sys var max_running_queries > 0
   *  6. The command is not filtered by admission_control_filter
   */
  if (!(thd->security_ctx->master_access & SUPER_ACL) && /* 1 */
      !thd->rli_slave && /* 2 */
      ((!opt_admission_control_by_trx || thd->is_real_trans) &&
       !thd->is_in_ac) && /* 3 */
      attrs->database && /* 4 */
      db_ac->get_max_running_queries() && /* 5 */
      !filter_command(thd->lex->sql_command) /* 6 */
     )
  {
    admission_check= true;
  }

  if (admission_check) {
    Ac_result res = db_ac->admission_control_enter(thd, attrs);
    if (res == Ac_result::AC_ABORTED) {
      my_error(ER_DB_ADMISSION_CONTROL, MYF(0),
               db_ac->get_max_waiting_queries(),
               thd->db ? thd->db : "unknown database");
      return 1;
    } else if (res == Ac_result::AC_TIMEOUT) {
      my_error(ER_DB_ADMISSION_CONTROL_TIMEOUT, MYF(0),
               thd->db ? thd->db : "unknown database");
      return 1;
    } else if (res == Ac_result::AC_KILLED) {
      return 1;
    }

    thd->is_in_ac = true;
  }

  return 0;
}


/*
 * Exit a query based on database entity
 *
 * @param thd THD structure
 * @param attrs Resource attributes
 *
 * @return 0 if successful, 1 if otherwise
 */
int multi_tenancy_exit_query(THD *thd, const MT_RESOURCE_ATTRS *attrs)
{
  if (!attrs->database)
    return 0; // no limiting

  db_ac->admission_control_exit(thd, attrs);
  return 0;
}


/*
 * Get the resource entity (e.g. db or user) from multi-tenancy plugin
 *
 * @param thd THD structure
 * @param type Resource type
 * @param attrs Resource attributes
 *
 * @return resource entity name. Emtpy string if no plugin is installed.
 */
std::string multi_tenancy_get_entity(
    THD *thd, MT_RESOURCE_TYPE type, const MT_RESOURCE_ATTRS *attrs)
{
  std::string entity;
  st_mysql_multi_tenancy *data= NULL;
  if (thd->variables.multi_tenancy_plugin)
  {
    mysql_mutex_lock(&thd->LOCK_thd_data);
    data = plugin_data(thd->variables.multi_tenancy_plugin,
                       struct st_mysql_multi_tenancy *);
    entity = data->get_entity_name(thd, type, attrs);
    mysql_mutex_unlock(&thd->LOCK_thd_data);
  }

  return entity;
}


/*
 * Get the resource counter of database or user from multi-tenancy plugin
 *
 * @param thd THD structure
 * @param type Resource type
 * @param attrs Resource attributes
 * @param entity_name Resource entity name if not null
 * @param limit Resource limit of the entity (output)
 * @param count Resource current count of the entity (output)
 *
 * @return current count. 0 if no plugin is installed.
 */
std::string multi_tenancy_get_entity_counter(
    THD *thd, MT_RESOURCE_TYPE type, const MT_RESOURCE_ATTRS *attrs,
    const char *entity_name, int *limit, int *count)
{
  *limit = 0;
  *count = -1;
  std::string entity;
  st_mysql_multi_tenancy *data= NULL;
  if (thd->variables.multi_tenancy_plugin)
  {
    mysql_mutex_lock(&thd->LOCK_thd_data);

    data = plugin_data(thd->variables.multi_tenancy_plugin,
                       struct st_mysql_multi_tenancy *);

    if (entity_name)
    {
      entity = entity_name;
    }
    else
    {
      entity = data->get_entity_name(thd, type, attrs);
    }

    if (!entity.empty())
    {
      *count = data->get_resource_counter(thd, type, entity.c_str(), limit);
    }

    mysql_mutex_unlock(&thd->LOCK_thd_data);
  }

  return entity;
}


int initialize_multi_tenancy_plugin(st_plugin_int *plugin_int)
{
  if (plugin_int->plugin->init && plugin_int->plugin->init(plugin_int))
  {
    sql_print_error("Plugin '%s' init function returned error.",
                    plugin_int->name.str);
    return 1;
  }

  /* Make the interface info more easily accessible */
  plugin_int->data= plugin_int->plugin->info;

  /* release the ref count of the previous multi-tenancy plugin */
  if (global_system_variables.multi_tenancy_plugin)
    plugin_unlock(NULL, global_system_variables.multi_tenancy_plugin);

  /* Add the global var referencing the plugin */
  plugin_ref plugin_r= plugin_int_to_ref(plugin_int);
  global_system_variables.multi_tenancy_plugin=
    my_plugin_lock(NULL, &plugin_r);

  return 0;
}


int finalize_multi_tenancy_plugin(st_plugin_int *plugin_int)
{
  /* release the ref count of the previous multi-tenancy plugin */
  if (global_system_variables.multi_tenancy_plugin)
  {
    plugin_unlock(NULL, global_system_variables.multi_tenancy_plugin);
    global_system_variables.multi_tenancy_plugin= NULL;
  }

  if (plugin_int->plugin->deinit && plugin_int->plugin->deinit(NULL))
  {
    DBUG_PRINT("warning", ("Plugin '%s' deinit function returned error.",
                            plugin_int->name.str));
    DBUG_EXECUTE("finalize_audit_plugin", return 1; );
  }

  plugin_int->data= NULL;
  return 0;
}


#else /* EMBEDDED_LIBRARY */


int multi_tenancy_add_connection(THD *thd, const MT_RESOURCE_ATTRS *attrs)
{
  return 1;
}

int multi_tenancy_close_connection(THD *thd, const MT_RESOURCE_ATTRS *attrs)
{
  return 1;
}

int multi_tenancy_admit_query(THD *thd, const MT_RESOURCE_ATTRS *attrs)
{
  return 1;
}

int multi_tenancy_exit_query(THD *thd, const MT_RESOURCE_ATTRS *attrs)
{
  return 1;
}


int initialize_multi_tenancy_plugin(st_plugin_int *plugin)
{
  return 1;
}


int finalize_multi_tenancy_plugin(st_plugin_int *plugin)
{
  return 1;
}


std::string multi_tenancy_get_entity(
    THD *thd, MT_RESOURCE_TYPE type, const MT_RESOURCE_ATTRS *attrs)
{
  return std::string("");
}


std::string multi_tenancy_get_entity_counter(
    THD *thd, MT_RESOURCE_TYPE type, const MT_RESOURCE_ATTRS *attrs,
    const char *entity, int *limit, int *count)
{
  *limit = 0;
  *count = -1;
  return std::string("");
}


#endif /* EMBEDDED_LIBRARY */


/*
 * Helper function: write a counter row to output
 */
static void store_counter(Protocol *protocol,
                          THD *thd,
                          MT_RESOURCE_TYPE type,
                          const char * type_str,
                          const MT_RESOURCE_ATTRS *attrs,
                          const char *entity_str)
{
  int limit;
  int count;
  std::string entity = multi_tenancy_get_entity_counter(
        thd, type, attrs, entity_str, &limit, &count);
  if (!entity.empty() && count >= 0)
  {
    protocol->prepare_for_resend();
    protocol->store(entity.c_str(), system_charset_info);
    protocol->store(type_str, system_charset_info);
    protocol->store_long((longlong) limit);
    protocol->store_long((longlong) count);
    protocol->update_checksum();
    protocol->write();
  }
}

/*
 * Show resource counters if there is any.
 * This is called for the command "SHOW RESOURCE COUNTERS".
 *
 * @param thd THD structure
 * @param attrs Resource attributes to get entity
 * @param entity Resource entity name if provided
 *
 * @return counter per type, or empty set if no plugin is installed
 */
void multi_tenancy_show_resource_counters(
    THD *thd, const MT_RESOURCE_ATTRS *attrs, const char *entity)
{
  List<Item> field_list;
  field_list.push_back(new Item_empty_string("Entity Name",NAME_LEN));
  field_list.push_back(new Item_empty_string("Resource Type",NAME_LEN));
  field_list.push_back(new Item_return_int("Resource Limit",
        MY_INT32_NUM_DECIMAL_DIGITS, MYSQL_TYPE_LONG));
  field_list.push_back(new Item_return_int("Current Count",
        MY_INT32_NUM_DECIMAL_DIGITS, MYSQL_TYPE_LONG));

  Protocol *protocol= thd->protocol;
  if (protocol->send_result_set_metadata(&field_list,
        Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF))
    return;

  // connection counter
  store_counter(protocol,
                thd,
                MT_RESOURCE_TYPE::MULTI_TENANCY_RESOURCE_CONNECTION,
                "connection",
                attrs,
                entity);

  // query counter
  store_counter(protocol,
                thd,
                MT_RESOURCE_TYPE::MULTI_TENANCY_RESOURCE_QUERY,
                "query",
                attrs,
                entity);

  my_eof(thd);
}


AC *db_ac; // admission control object

/*
 * A wrapper around std::stoul that catches any exceptions to return an error
 * code instead. On success, val is populated with the output.
 *
 * @return 0 on success, -1 on error
 */
static int stoul_noexcept(const std::string str, ulong *val) {
  try {
    size_t pos = 0;
    *val = std::stoul(str, &pos);
    // Check if at least 1 charater was read and that the whole string was
    // parsed completely.
    if (pos > 0 && pos == str.size()) {
      return 0;
    }
  } catch (const std::exception& e) {
    return -1;
  }

  return -1;
}

static ulong get_queue(THD * thd) {
  const std::string attr_name = "@@admission_control_queue";

  // To get queue name, we look at query attribute, connection attribute, and
  // then session variable in that order.
  for (const auto& map : { &thd->query_attrs_map, &thd->connection_attrs_map }) {
    auto it = map->find(attr_name);
    if (it != map->end()) {
      ulong value = 0;
      if (!stoul_noexcept(it->second.c_str(), &value) &&
          value < MAX_AC_QUEUES) {
        return value;
      }
    }
  }

  return thd->variables.admission_control_queue;
}

/**
 * @param thd THD structure.
 * @param attrs session resource attributes
 *
 * Applies admission control checks for the entity. Outline of
 * the steps in this function:
 * 1. Error out if we crossed the max waiting limit.
 * 2. Put the thd in a queue.
 * 3. If we crossed the max running limit then wait for signal from threads
 *    that completed their query execution.
 *
 * Note current implementation assumes the admission control entity is
 * database. We will lift the assumption and implement the entity logic in
 * multitenancy plugin.
 *
 * @return AC_ADMITTED - Admitted
 *         AC_ABORTED  - Rejected because queue size too large
 *         AC_TIMEOUT  - Rejected because waiting on queue for too long
 *         AC_KILLED   - Killed while waiting for admission
 */
Ac_result AC::admission_control_enter(THD *thd,
                                      const MT_RESOURCE_ATTRS *attrs) {
  Ac_result res = Ac_result::AC_ADMITTED;
  std::string entity = attrs->database;
  const char* prev_proc_info = thd->proc_info;
  THD_STAGE_INFO(thd, stage_admission_control_enter);
  // Unlock this before waiting.
  mysql_rwlock_rdlock(&LOCK_ac);
  if (max_running_queries) {
    auto it = ac_map.find(entity);
    if (it == ac_map.end()) {
      // New DB.
      mysql_rwlock_unlock(&LOCK_ac);
      insert(entity);
      mysql_rwlock_rdlock(&LOCK_ac);
      it = ac_map.find(entity);
    }

    if (!thd->ac_node) {
      // Both THD and the admission control queue will share the object
      // created here.
      thd->ac_node = std::make_shared<st_ac_node>(thd);
    }
    auto ac_info = it->second;
    MT_RETURN_TYPE ret = MT_RETURN_TYPE::MULTI_TENANCY_RET_FALLBACK;
    st_mysql_multi_tenancy *data= NULL;
    mysql_mutex_lock(&ac_info->lock);
    // If a multi_tenancy plugin exists, check plugin for per-entity limit
    if (thd->variables.multi_tenancy_plugin)
    {
      data = plugin_data(thd->variables.multi_tenancy_plugin,
                         struct st_mysql_multi_tenancy *);
      ret = data->request_resource(
          thd,
          MT_RESOURCE_TYPE::MULTI_TENANCY_RESOURCE_QUERY,
          attrs);
    }
    // if plugin is disabled, fallback to check global query limit
    if (ret == MT_RETURN_TYPE::MULTI_TENANCY_RET_FALLBACK)
    {
      if (ac_info->running_queries < max_running_queries)
        ret = MT_RETURN_TYPE::MULTI_TENANCY_RET_ACCEPT;
      else
      {
        if (max_waiting_queries && ac_info->waiting_queries >= max_waiting_queries)
          ret = MT_RETURN_TYPE::MULTI_TENANCY_RET_REJECT;
        else
          ret = MT_RETURN_TYPE::MULTI_TENANCY_RET_WAIT;
      }
    }

    bool timeout;
    thd->ac_node->queue = get_queue(thd);
    switch (ret)
    {
      case MT_RETURN_TYPE::MULTI_TENANCY_RET_REJECT:
        ++ac_info->queues[thd->ac_node->queue].aborted_queries;
        ++total_aborted_queries;
        // We reached max waiting limit. Error out
        mysql_mutex_unlock(&ac_info->lock);
        res = Ac_result::AC_ABORTED;
        break;

      case MT_RETURN_TYPE::MULTI_TENANCY_RET_WAIT:
        enqueue(thd, ac_info);
        /**
          Inserting or deleting in std::map will not invalidate existing
          iterators except of course if the current iterator is erased. If the
          db corresponding to this iterator is getting dropped, these waiting
          queries are given signal to abort before the iterator
          is erased. See AC::remove().
          So, we don't need LOCK_ac here. The motivation to unlock the read lock
          is that waiting queries here shouldn't block other operations
          modifying ac_map or max_running_queries/max_waiting_queries.
        */
        while (true) {
          mysql_rwlock_unlock(&LOCK_ac);
          timeout = wait_for_signal(thd, thd->ac_node, ac_info);
          // Retake locks in correct lock order.
          mysql_rwlock_rdlock(&LOCK_ac);
          mysql_mutex_lock(&ac_info->lock);
          if (timeout || thd->killed || thd->ac_node->running)
            break;
        }

        // If a query has been successfully admitted, then it has already
        // been dequeued by a different thread, with updated counters.
        //
        // The only reason it might be still queued is during error conditions.
        if (thd->ac_node->queued) {
          DBUG_ASSERT(timeout || thd->killed);
          dequeue(thd, ac_info);
        }

        if (timeout || thd->killed) {
          // It's possible that we've gotten an error after this thread was
          // passed admission control. If so, reset the running counters.
          if (thd->ac_node->running) {
            DBUG_ASSERT(ac_info->running_queries > 0);
            --ac_info->running_queries;
            DBUG_ASSERT(ac_info->queues[thd->ac_node->queue].running_queries > 0);
            --ac_info->queues[thd->ac_node->queue].running_queries;
            thd->ac_node->running = false;
          }

          mysql_mutex_unlock(&ac_info->lock);
          if (timeout) {
            ++total_timeout_queries;
            ++ac_info->queues[thd->ac_node->queue].timeout_queries;
            res = Ac_result::AC_TIMEOUT;
          } else {
            res = Ac_result::AC_KILLED;
          }
          break;
        }
        mysql_mutex_unlock(&ac_info->lock);
        break;
      case MT_RETURN_TYPE::MULTI_TENANCY_RET_ACCEPT:
        // We are below the max running limit.
        ++ac_info->running_queries;
        ++ac_info->queues[thd->ac_node->queue].running_queries;
        thd->ac_node->running = true;
        mysql_mutex_unlock(&ac_info->lock);
        break;

      default:
        // unreachable branch
        DBUG_ASSERT(0);
    }
  }
  mysql_rwlock_unlock(&LOCK_ac);
  thd->proc_info = prev_proc_info;
  return res;
}

/**
  Wait for admission control slots to free up, or until timeout.

  @return False No timeout
          True  Timeout occurred

*/
bool AC::wait_for_signal(THD *thd, std::shared_ptr<st_ac_node> &ac_node,
                         std::shared_ptr<Ac_info> ac_info) {
  PSI_stage_info old_stage;
  int res = 0;
  mysql_mutex_lock(&ac_node->lock);
  /**
    The locking order followed during admission_control_enter() is
    lock ac_info
    lock ac_node
    unlock ac_info
    unlock ac_node
    The locks are interleaved to avoid possible races which makes
    this waiting thread miss the signal from admission_control_exit().
  */
  mysql_mutex_unlock(&ac_info->lock);
  thd->ENTER_COND(&ac_node->cond, &ac_node->lock,
                  &stage_waiting_for_admission,
                  &old_stage);
  if (thd->variables.admission_control_queue_timeout == 0) {
    // Don't bother waiting if timeout is 0.
    res = ETIMEDOUT;
  } else if (thd->variables.admission_control_queue_timeout < 0) {
    // Spurious wake-ups are checked by callers of wait_for_signal.
    mysql_cond_wait(&ac_node->cond, &ac_node->lock);
  } else {
    struct timespec wait_timeout;
    set_timespec_nsec(wait_timeout,
                      thd->variables.admission_control_queue_timeout * 1000000);

    res = mysql_cond_timedwait(&ac_node->cond, &ac_node->lock, &wait_timeout);
    DBUG_ASSERT(res == 0 || res == ETIMEDOUT);
  }
  thd->EXIT_COND(&old_stage);

  return res == ETIMEDOUT;
}

/**
  @param thd THD structure
  @param attrs session resource attributes

  Signals one waiting thread. Pops out the first THD in the queue.
*/
void AC::admission_control_exit(THD* thd, const MT_RESOURCE_ATTRS *attrs) {
  std::string entity = attrs->database;
  const char* prev_proc_info = thd->proc_info;
  THD_STAGE_INFO(thd, stage_admission_control_exit);
  mysql_rwlock_rdlock(&LOCK_ac);
  auto it = ac_map.find(entity);
  if (it != ac_map.end()) {
    auto ac_info = it->second;
    st_mysql_multi_tenancy *data= NULL;
    mysql_mutex_lock(&ac_info->lock);
    // If a multi_tenancy plugin exists, check plugin for per-entity limit
    if (thd->variables.multi_tenancy_plugin)
    {
      data = plugin_data(thd->variables.multi_tenancy_plugin,
                         struct st_mysql_multi_tenancy *);
      data->release_resource(
          thd,
          MT_RESOURCE_TYPE::MULTI_TENANCY_RESOURCE_QUERY,
          attrs);
    }

    if (thd->ac_node->running) {
      DBUG_ASSERT(ac_info->running_queries > 0);
      --ac_info->running_queries;
      DBUG_ASSERT(ac_info->queues[thd->ac_node->queue].running_queries > 0);
      --ac_info->queues[thd->ac_node->queue].running_queries;
      thd->ac_node->running = false;
    }

    // We determine here which queue to pick from. For every queue, we
    // calculate a score based on the number of running queries, and its
    // weight. Inituitively, the weight determines how much of the running
    // pool a queue is allowed to occupy. For example, if queue A has weight 3
    // and queue B has weight 7, the we expect 30% of the pool to have queries
    // running from A.
    //
    // We calculate a score for all queues that have waiting queries, and pick
    // the queue with the minimum score. In case of ties, we arbitrarily pick
    // the first encountered queue.
    if (max_running_queries && ac_info->waiting_queries > 0) {
      double min_score = std::numeric_limits<double>::max();
      ulong min_queue = 0;
#ifndef DBUG_OFF
      ulong running_queries_sum = 0;
      ulong waiting_queries_sum = 0;
#endif

      for (ulong i = 0; i < MAX_AC_QUEUES; i++) {
        const auto& queue = ac_info->queues[i];
#ifndef DBUG_OFF
        running_queries_sum += queue.running_queries;
        waiting_queries_sum += queue.waiting_queries();
#endif
        // Skip queues that don't have waiting queries.
        if (queue.waiting_queries() == 0) continue;

        double score = queue.running_queries / (weights[i] ? weights[i] : 1);

        if (score < min_score) {
          min_queue = i;
          min_score = score;
        }
      }

      DBUG_ASSERT(ac_info->waiting_queries == waiting_queries_sum);
      DBUG_ASSERT(ac_info->running_queries == running_queries_sum);

      auto& candidate = ac_info->queues[min_queue].queue.front();
      dequeue_and_run(candidate->thd, ac_info);
    }

    mysql_mutex_unlock(&ac_info->lock);
  }
  mysql_rwlock_unlock(&LOCK_ac);
  thd->proc_info = prev_proc_info;
}

/*
 * @param thd THD
 * @param ac_info AC info
 *
 * Enqueues thd onto its queue. The queue is taken from thd->ac_node->queue.
 */
void AC::enqueue(THD *thd, std::shared_ptr<Ac_info> ac_info) {
  mysql_mutex_assert_owner(&ac_info->lock);
  auto& ac_node = thd->ac_node;

  ulong queue = ac_node->queue;
  ac_info->queues[queue].queue.push_back(ac_node);
  ac_node->pos = --ac_info->queues[queue].queue.end();
  ac_node->queued = true;
  ++ac_info->waiting_queries;
}

/*
 * @param thd THD
 * @param ac_info AC info
 *
 * Dequeues thd from its queue. The queue is taken from thd->ac_node->queue.
 */
void AC::dequeue(THD *thd, std::shared_ptr<Ac_info> ac_info) {
  mysql_mutex_assert_owner(&ac_info->lock);
  auto& ac_node = thd->ac_node;

  DBUG_ASSERT(ac_node->queued);
  ac_info->queues[ac_node->queue].queue.erase(ac_node->pos);
  ac_node->queued = false;
  --ac_info->waiting_queries;
}

/*
 * @param thd THD
 * @param ac_info AC info
 *
 * Dequeues thd from its queue. Sets its state to running, and signals
 * that thread to start running.
 */
void AC::dequeue_and_run(THD *thd, std::shared_ptr<Ac_info> ac_info) {
  mysql_mutex_assert_owner(&ac_info->lock);

  dequeue(thd, ac_info);

  ++ac_info->running_queries;
  ++ac_info->queues[thd->ac_node->queue].running_queries;
  thd->ac_node->running = true;

  signal(thd->ac_node);
}

/*
 * @param s comma delimited string containing the weights
 *
 * Updates AC::weights based on the comma delimited string passed in.
 *
 * @returns -1 on failure
 *          0 on success.
 */
int AC::update_queue_weights(char *s) {
  auto v = split_into_vector(s ? s : "", ',');
  auto tmp = weights;

  if (v.size() > MAX_AC_QUEUES) {
    return -1;
  }

  for (ulong i = 0; i < std::min(MAX_AC_QUEUES, v.size()); i++) {
    ulong value = 0;
    if (!stoul_noexcept(v[i], &value) && value > 0 && value < LONG_MAX) {
      tmp[i] = (ulong)value;
    } else {
      return -1;
    }
  }
  mysql_rwlock_wrlock(&LOCK_ac);
  weights = tmp;
  mysql_rwlock_unlock(&LOCK_ac);

  return 0;
}

/*
 * @param entity The entity being dropped
 *
 * Removes a dropped entity info from the global map.
 */
void AC::remove(const char* entity) {
  std::string str(entity);
  // First take a read lock to unblock any waiting queries.
  mysql_rwlock_rdlock(&LOCK_ac);
  auto it = ac_map.find(str);
  if (it != ac_map.end()) {
    auto ac_info  = it->second;
    mysql_mutex_lock(&ac_info->lock);
    for (auto &q : ac_info->queues) {
      while (q.waiting_queries() > 0) {
        dequeue_and_run(q.queue.front()->thd, ac_info);
      }
    }
    mysql_mutex_unlock(&ac_info->lock);
  }
  mysql_rwlock_unlock(&LOCK_ac);
  mysql_rwlock_wrlock(&LOCK_ac);
  it = ac_map.find(std::string(str));
  if (it != ac_map.end()) {
    ac_map.erase(it);
  }
  mysql_rwlock_unlock(&LOCK_ac);
}

ST_FIELD_INFO admission_control_queue_fields_info[]=
{
  {"SCHEMA_NAME", NAME_LEN, MYSQL_TYPE_STRING, 0, 0, 0, SKIP_OPEN_TABLE},
  {"QUEUE_ID", 21, MYSQL_TYPE_LONGLONG, 0, MY_I_S_UNSIGNED, 0, SKIP_OPEN_TABLE},
  {"WAITING_QUERIES", 21, MYSQL_TYPE_LONGLONG, 0, MY_I_S_UNSIGNED, 0, SKIP_OPEN_TABLE},
  {"RUNNING_QUERIES", 21, MYSQL_TYPE_LONGLONG, 0, MY_I_S_UNSIGNED, 0, SKIP_OPEN_TABLE},
  {"ABORTED_QUERIES", 21, MYSQL_TYPE_LONGLONG, 0, MY_I_S_UNSIGNED, 0, SKIP_OPEN_TABLE},
  {"TIMEOUT_QUERIES", 21, MYSQL_TYPE_LONGLONG, 0, MY_I_S_UNSIGNED, 0, SKIP_OPEN_TABLE},
  {0, 0, MYSQL_TYPE_STRING, 0, 0, 0, SKIP_OPEN_TABLE}
};

/*
 * @param thd THD
 * @param tables contains the TABLE struct to populate
 *
 * Removes a dropped entity info from the global map.
 *
 * @return 0 on success
 *         1 on failure
 */
int fill_ac_queue(THD *thd, TABLE_LIST *tables, Item *) {
  DBUG_ENTER("fill_ac_queue");
  TABLE* table= tables->table;

  mysql_rwlock_rdlock(&db_ac->LOCK_ac);
  for (const auto& pair : db_ac->ac_map) {
    const std::string& db = pair.first;
    const auto& ac_info = pair.second;
    mysql_mutex_lock(&ac_info->lock);
    for (ulong i = 0; i < MAX_AC_QUEUES; i++) {
      const auto& q = ac_info->queues[i];

      auto waiting = q.waiting_queries();
      auto running = q.running_queries;
      auto timeout = q.timeout_queries;
      auto aborted = q.aborted_queries;
      // Skip queues with no waiting/running queries.
      if (waiting == 0 && running == 0 && timeout == 0 && aborted == 0)
        continue;

      int f = 0;

      // SCHEMA_NAME
      table->field[f++]->store(db.c_str(), db.size(), system_charset_info);

      // QUEUE_ID
      table->field[f++]->store((ulonglong) i, TRUE);

      // WAITING_QUERIES
      table->field[f++]->store((ulonglong) waiting, TRUE);

      // RUNNING_QUERIES
      table->field[f++]->store((ulonglong) running, TRUE);

      // ABORTED_QUERIES
      table->field[f++]->store((ulonglong) aborted, TRUE);

      // TIMEOUT_QUERIES
      table->field[f++]->store((ulonglong) timeout, TRUE);

      if (schema_table_store_record(thd, table)) {
        mysql_mutex_unlock(&ac_info->lock);
        mysql_rwlock_unlock(&db_ac->LOCK_ac);

        DBUG_RETURN(1);
      }
    }
    mysql_mutex_unlock(&ac_info->lock);
  }
  mysql_rwlock_unlock(&db_ac->LOCK_ac);

  DBUG_RETURN(0);
}
