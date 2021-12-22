#ifndef SESSION_TRACKER_INCLUDED
#define SESSION_TRACKER_INCLUDED

/* Copyright (c) 2015, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "m_string.h"
#include "thr_lock.h"

#include "map"
#include <string>

/* forward declarations */
class THD;
class set_var;
class String;
typedef struct charset_info_st CHARSET_INFO;


enum enum_session_tracker
{
  SESSION_STATE_CHANGE_TRACKER,
  SESSION_GTIDS_TRACKER,                         /* Tracks GTIDs */
  SESSION_RESP_ATTR_TRACKER,
};

#define SESSION_TRACKER_END SESSION_RESP_ATTR_TRACKER

/**
  State_tracker
  -------------
  An abstract class that defines the interface for any of the server's
  'session state change tracker'. A tracker, however, is a sub- class of
  this class which takes care of tracking the change in value of a part-
  icular session state type and thus defines various methods listed in this
  interface. The change information is later serialized and transmitted to
  the client through protocol's OK packet.

  Tracker system variables :-
  A tracker is normally mapped to a system variable. So in order to enable,
  disable or modify the sub-entities of a tracker, the user needs to modify
  the respective system variable either through SET command or via command
  line option. As required in system variable handling, this interface also
  includes two functions to help in the verification of the supplied value
  (ON_CHECK) and the updation (ON_UPDATE) of the tracker system variable,
  namely - check() and update().
*/

class State_tracker
{
protected:
  /** Is tracking enabled for a particular session state type ? */
  bool m_enabled;

  /** Has the session state type changed ? */
  bool m_changed;

public:
  /** Constructor */
  State_tracker() : m_enabled(false), m_changed(false)
  {}

  /** Destructor */
  virtual ~State_tracker()
  {}

  /** Getters */
  bool is_enabled() const
  { return m_enabled; }

  virtual bool is_changed(THD* thd) const
  { return m_changed; }

  /** Called in the constructor of THD*/
  virtual bool enable(THD *thd)= 0;

  virtual bool force_enable()= 0;

  /** To be invoked when the tracker's system variable is checked (ON_CHECK). */
  virtual bool check(THD *thd, set_var *var)= 0;

  /** To be invoked when the tracker's system variable is updated (ON_UPDATE).*/
  virtual bool update(THD *thd)= 0;

  /** Store changed data into the given buffer. */
  virtual bool store(THD *thd, String &buf)= 0;

  /** Mark the entity as changed. */
  virtual void mark_as_changed(THD *thd, LEX_CSTRING *name,
                               LEX_CSTRING *value = NULL)= 0;

  virtual void claim_memory_ownership()
  {}

  virtual void audit_tracker(std::map<std::string, std::string>& audit_map)
  {}
};


/**
  Session_tracker
  ---------------
  This class holds an object each for all tracker classes and provides
  methods necessary for systematic detection and generation of session
  state change information.
*/

class Session_tracker
{
private:
  State_tracker *m_trackers[SESSION_TRACKER_END + 1];

  /* The following two functions are private to disable copying. */
  /** Copy constructor */
  Session_tracker(Session_tracker const &other)
  {
    DBUG_ASSERT(FALSE);
  }

  /** Copy assignment operator */
  Session_tracker& operator= (Session_tracker const &rhs)
  {
    DBUG_ASSERT(FALSE);
    return *this;
  }

  std::map<std::string, std::string> audit_attrs;

public:

  /** Constructor */
  Session_tracker()
  {}

  /** Destructor */
  ~Session_tracker()
  {
  }
  /**
    Initialize Session_tracker objects and enable them based on the
    tracker_xxx variables' value that the session inherit from global
    variables at the time of session initialization (see plugin_thdvar_init).
  */
  void init(const CHARSET_INFO *char_set);
  void enable(THD *thd);

  /** Returns the pointer to the tracker object for the specified tracker. */
  State_tracker *get_tracker(enum_session_tracker tracker) const;

  /** Checks if m_enabled flag is set for any of the tracker objects. */
  bool enabled_any();

  /** Checks if m_changed flag is set for any of the tracker objects. */
  bool changed_any(THD* thd);

  /**
    Stores the session state change information of all changes session state
    type entities into the specified buffer. Passes the audit map so each
    tracker can choose what to expose to the audit plugin
  */
  void store(THD *thd, String &main_buf);
  void deinit()
  {
    for (int i= 0; i <= SESSION_TRACKER_END; i ++)
      delete m_trackers[i];
  }

  void claim_memory_ownership();

  const std::map<std::string, std::string>& get_audit_attrs() const {
    return audit_attrs;
  }

  void reset_audit_attrs() {
    audit_attrs.clear();
  }
};

/*
  Session_state_change_tracker
  ----------------------------
  This is a boolean tracker class that will monitor any change that contributes
  to a session state change.
  Attributes that contribute to session state change include:
     - Successful change to System variables
     - User defined variables assignments
     - temporary tables created, altered or deleted
     - prepared statements added or removed
     - change in current database
*/

class Session_state_change_tracker : public State_tracker
{
private:

  void reset();

public:
  Session_state_change_tracker();
  bool enable(THD *thd) override;
  bool check(THD *thd, set_var *var) override
  { return false; }
  bool force_enable() override;
  bool update(THD *thd) override;
  bool store(THD *thd, String &buf) override;
  void mark_as_changed(THD *thd, LEX_CSTRING *tracked_item_name,
      LEX_CSTRING *tracked_item_value = NULL) override;
  bool is_state_changed(THD*);
  void ensure_enabled(THD *thd) {}
};

/*
  Session_resp_attr_tracker
  ----------------------
  This is a tracker class that will monotor response attributes
*/

class Session_resp_attr_tracker : public State_tracker
{
private:
  void reset();
  Session_resp_attr_tracker(const Session_resp_attr_tracker&) = delete;
  Session_resp_attr_tracker& operator=(const Session_resp_attr_tracker&) =
      delete;

  bool m_forced_enabled; // Override the variable to force this as enabled
  std::map<std::string, std::string> attrs_;

public:
  Session_resp_attr_tracker()
  { m_forced_enabled = false; m_changed = false; m_enabled = false; }

  bool enable(THD *thd) override;
  bool force_enable() override { m_forced_enabled = true; return false; }
  bool check(THD *thd, set_var *var) override { return false; }
  bool update(THD *thd) override { return enable(thd); }
  bool store(THD *thd, String &buf) override;
  void mark_as_changed(THD *thd, LEX_CSTRING *key, LEX_CSTRING *value) override;
  void audit_tracker(std::map<std::string, std::string>& audit_map) override;
};

#endif /* SESSION_TRACKER_INCLUDED */
