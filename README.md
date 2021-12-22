# PASV
This is a prototype of PASV-based MyRocks, by which the MyRocks could be released form the double-logging problem.

MyRocks-PASV is implemented based on MyRocks.

The main difference is about eliminating the RocksDB's WAL and achieving the reliability by only relying on the binlog.

The special flush-flag will be generated during running. Based on flush-flags, the persistence status of the database can be monitered.

Some path may need be modified since all flush-flags will also be persisted by a on-disk log.

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++Original README+++++++++++++++++++++++++++++++++++++++++++
