# Copyright (c) 2003, 2012, Oracle and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
# USA
#
# This grammar contains ddl for partitions as well as partition extentions to dml.
# It creates partitioned and not partitioned tables and also dml without partition
# extension. The focus is on partitions and dml with partition extention.
# The grammar doesn't rely on the predefined tables, but uses instead tables defined in
# query_init.
#
# Experiences
#
# --rpl_mode=row:
# perl runall.pl  --engine=MyISAM --debug --rpl_mode=row --duration=12000 --queries=40000 
# --reporter=Deadlock,Backtrace,ErrorLog --threads=10 --basedir=/home/horst/bzrysql-trunk 
# --mysqld=--lock-wait-timeout=1 --mysqld=--general-log=ON --mysqld=--default-storage-engine=
# MyISAM --mysqld=--log-output=table --mysqld=--loose-innodb-lock-wait-timeout=1 
# --grammar=./conpartitioning/partitions.yy --vardir=/dev/shm/var1 --mask-level=0 
# --mask=0 --seed=1 >./storage/1.l 2>&1a
#
# Failed sometimes with reclication-error 103, but also succedded (needed about 5-10 Minutes). 
# Produced a log file of about 10MB and a var directory with 6,6MB.
# 
# --rpl_mode=mixed:
# Failed sometimes with reclication-error 103, but also succedded (needed about 5-10 Minutes). 
# Produced a log file of about 36KB  and a var directory with 5MB.
# 
# perl runall.pl  --engine=MyISAM --debug --duration=12000 --queries=40000 
# --reporter=Deadlock,Backtrace,ErrorLog --threads=10 
# --basedir=/home/horst/bzr/mysql-trunk--mysqld=--lock-wait-timeout=1 --mysqld=--general-log=ON 
# --mysqld=--default-storage-engine=MyISAM --mysqld=--log-output=table 
# --mysqld=--loose-innodb-lock-wait-timeout=1 --grammar=./conf/partitioning/partitions.yy 
# --vardir=/dev/shm/var1 --mask-level=0 --mask=0 --seed=1 >./storage/1.log 2>&1

# Log: 21MB, var: 148Mb, time about 8Min.  Semantic_error: 12234, OK:27980

# perl runall.pl  --engine=MyISAM --debug --duration=24000 --queries=100000 
# --reporter=Deadlock,Backtrace,ErrorLog --threads=16 
# --basedir=/home/horst/bzr/mysql-trunk--mysqld=--lock-wait-timeout=1 --mysqld=--general-log=ON 
# --mysqld=--default-storage-engine=MyISAM --mysqld=--log-output=table 
# --mysqld=--loose-innodb-lock-wait-timeout=1 --grammar=./conf/partitioning/partitions.yy 
# --vardir=/dev/shm/var1 --mask-level=0 --mask=0 --seed=1 >./storage/1.log 2>&1

# Log: 48MB, var: 320Mb, time about 20Min. 


##########################################################################
# Initialization of tables with focus on partitioned tables.

query_init:
	init_db ;

init_db:
	create_tables ; insert_tables ;  cache_index ; load_index ;

create_tables:
	create_10 ; create_10 ; create_10 ; create_10 ; create_10 ; create_10 ; create_nop_4 ;

create_10:
	create ; create ; create ; create ; create ; create ; create ; create ; create ; create ;

create_nop_4:
	create_nop ; create_nop ; create_nop ; create_nop ;

insert_tables:
	insert_part_tables ; insert_part_tables ; insert_part_tables ; insert_part_tables ; insert_nop_tables ;

insert_part_tables:
	insert_part_6 ; insert_part_6 ; insert_part_6 ; insert_part_6 ; insert_part_6 ;

insert_nop_tables:
	insert_nop_6 ; insert_nop_6 ; insert_nop_6 ; insert_nop_6 ; insert_nop_6 ;

insert_part_6:
	insert_part ; insert_part ; insert_part ; insert_part ; insert_part ; insert_part ;

insert_nop_6:
	insert_nop ; insert_nop ; insert_nop ; insert_nop ; insert_nop ; insert_nop ;

create:
        CREATE TABLE if_not_exists table_name_part (
                `col_int_nokey` INTEGER,
                `col_int_key` INTEGER NOT NULL,
                KEY (`col_int_key`)
	) ENGINE = engine /*!50100 partition */ ;

create_nop:
        CREATE TABLE if_not_exists table_name_nopart (
                `col_int_nokey` INTEGER,
                `col_int_key` INTEGER NOT NULL,
                KEY (`col_int_key`)
	) ENGINE = engine ;

insert_part:
        INSERT INTO table_name_part   ( `col_int_nokey`, `col_int_key` ) VALUES ( value , value ) , ( value , value ) , ( value , value ) , ( value , value ) ;

insert_nop:
        INSERT INTO table_name_nopart ( `col_int_nokey`, `col_int_key` ) VALUES ( value , value ) , ( value , value ) , ( value , value ) , ( value , value ) ;

##########################################################################
# Randomly executed SQL

query:
	exec_sql ;

exec_sql:
	select_explain |
	select | select | select | select | select | select                   |
	select | select | select | select | select | select                   |
	select | select | select | select | select | select                   |
	insert | update | delete | insert | update                            |
	insert | update | delete | insert | update                            |
	alter | alter | alter | alter | alter | alter                         |
	alter | alter | alter | alter | alter | alter                         |
	cache_index | load_index                                              |
	create_sel | create_sel | create_sel | create_sel | create_sel | drop |
	set_key_buffer_size | set_key_cache_block_size                        ;

cache_index:
	CACHE INDEX table_name_letter IN cache_name                                               |
	CACHE INDEX table_name_letter /*!50400 PARTITION ( ALL ) */ IN cache_name                 |
	CACHE INDEX table_name_letter /*!50400 PARTITION ( partition_name_list ) */ IN cache_name ;

load_index:
	LOAD INDEX INTO CACHE table_name_letter ignore_leaves                                               |
	LOAD INDEX INTO CACHE table_name_letter /*!50400 PARTITION ( ALL ) */ ignore_leaves                 |
	LOAD INDEX INTO CACHE table_name_letter /*!50400 PARTITION ( partition_name_list ) */ ignore_leaves ;

ignore_leaves:
	| IGNORE LEAVES ;

set_key_buffer_size:
	/*!50400 SET GLOBAL cache_name.key_buffer_size = _tinyint_unsigned   */ |
	/*!50400 SET GLOBAL cache_name.key_buffer_size = _smallint_unsigned  */ |
	/*!50400 SET GLOBAL cache_name.key_buffer_size = _mediumint_unsigned */ ;

set_key_cache_block_size:
	/*!50400 SET GLOBAL cache_name.key_cache_block_size = key_cache_block_size_enum */ ;

key_cache_block_size_enum:
	512 | 1024 | 2048 | 4096 | 8192 | 16384 ;

cache_name:
	c1 | c2 | c3 | c4;

select_explain:
	EXPLAIN /*!50100 PARTITIONS */ SELECT _field FROM table_name_letter where ;

create_select:
	SELECT `col_int_nokey` % 10 AS `col_int_nokey` , `col_int_key` % 10 AS `col_int_key` FROM table_name_letter where ;

select:
	SELECT `col_int_nokey` % 10 AS `col_int_nokey` , `col_int_key` % 10 AS `col_int_key` FROM dml_table_name    where ;

# WHERE clauses suitable for partition pruning
where:
	|                                      |
	WHERE _field comparison_operator value |
	WHERE _field BETWEEN value AND value   ;

comparison_operator:
        > | < | = | <> | != | >= | <= ;

insert:
        insert_replace INTO dml_table_name ( `col_int_nokey`, `col_int_key` ) VALUES ( value , value ) , ( value , value )                     |
        insert_replace INTO dml_table_name ( `col_int_nokey`, `col_int_key` ) select ORDER BY `col_int_key` , `col_int_nokey` LIMIT limit_rows ;

insert_replace:
        INSERT | REPLACE ;

update:
        UPDATE dml_table_name SET _field = value WHERE _field = value ;

delete:
        DELETE FROM dml_table_name WHERE _field = value ORDER BY `col_int_key` , `col_int_nokey` LIMIT limit_rows ;

dml_table_name:
	table_name_part_ext | table_name_part_ext | table_name_part_ext | table_name_part_ext |
	table_name_part_ext | table_name_part_ext | table_name_part_ext | table_name_part_ext |
	table_name_nopart                                                                     ;

table_name_part_ext:
	table_name_part PARTITION (partition_name_list) ;

table_name_nopart:
	a | b ;

table_name_part:
	c | d | e | f | g | h | i | j | k | l | m | n | o | p | q | r | s | t | u | v | w | x | y | z ;

value:
        _digit ;

_field:
        `col_int_nokey` | `col_int_nokey` ;

create_sel:
        create_part | create_part | create_part | create_nopart | create_nopart ;

create_part:
	CREATE TABLE if_not_exists table_name_part (
		`col_int_nokey` INTEGER,
		`col_int_key` INTEGER NOT NULL,
		KEY (`col_int_key`)
	) ENGINE = engine /*!50100 partition */ create_select ;

create_nopart:
        CREATE TABLE if_not_exists table_name_nopart (
                `col_int_nokey` INTEGER,
                `col_int_key` INTEGER NOT NULL,
                KEY (`col_int_key`)
        ) ENGINE = engine create_select ;

table_name_letter:
	table_name_part   |
	table_name_nopart ;

drop:
	DROP TABLE if_exists table_name_letter ;

alter:
	/*!50400 ALTER TABLE table_name_letter alter_operation */;

alter_operation:
	partition                                                           |
	enable_disable KEYS                                                 |
	ADD PARTITION (PARTITION partition_name VALUES LESS THAN MAXVALUE)  |
	ADD PARTITION (PARTITION p25 VALUES LESS THAN MAXVALUE)             |
	DROP PARTITION partition_name                                       |
	COALESCE PARTITION one_two                                          |
	ANALYZE PARTITION partition_name_list                               |
	CHECK PARTITION partition_name_list                                 |
	REBUILD PARTITION partition_name_list                               |
	REPAIR PARTITION partition_name_list                                |
	REMOVE PARTITIONING                                                 |
	OPTIMIZE PARTITION partition_name_list                              |
	ENGINE = engine                                                     |
	ORDER BY _field                                                     |
	TRUNCATE PARTITION partition_name_list		# can not be used in comparison tests against 5.0
;
#	REORGANIZE PARTITION partition_name_list                            |
#       EXCHANGE PARTITION partition_name WITH TABLE table_name_nopart      |

one_two:
	1 | 2;

some_partition_names:
	partition_name                      |
	partition_name                      |
	partition_name                      |
	partition_name                      |
	partition_name                      |
	partition_name, some_partition_names;

partition_name_list:
	partition_name_comb                 |
	partition_name_comb                 |
	partition_name_comb                 |
        some_partition_names                ;

partition_name_comb:
	p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17,p18,p19,p20,p21,p22,p23,p24,
p25,p26,p27,p28,p29,p30,p31,p32,p33,p34,p35,p36,p37,p38,p39,p40,p41,p42,p43,p44,p45,p46,
p47,p48,p49,p50,p51,p52,p53,p54,p55,p56,p57,p58,p59,p60,p61,p62,p63,p64,p65,p66,p67,p68,
p69,p70,p71,p72,p73,p74,p75,p76,p77,p78,p79,p80,p81,p82,p83,p84,p85,p86,p87,p88,p89,p90,
p91,p92,p93,p94,p95,p96,p97,p98,p99                                                       |
	p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17,p18,p19,p20,p21,p22,p23,p24,
p25,p26,p27,p28,p29,p30,p31,p32,p33,p34,p35,p36,p37,p38,p39,p40,p41,p42,p43,p44,p45,p46,
p47,p48,p49,p50,p51,p52,p53,p54,p55,p56,p57,p58,p59,p60,p61,p62,p63,p64,p65,p66,p67,p68,
p69,p70,p71,p72,p73,p74,p75,p76,p77,p78,p79,p80,p81,p82,p83,p84,p85,p86,p87,p88,p89,p90,
p91,p92,p93,p94,p95,p96,p97,p98                                                           |
	p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17,p18,p19,p20,p21,p22,p23,p24,
p25,p26,p27,p28,p29,p30,p31,p32,p33,p34,p35,p36,p37,p38,p39,p40,p41,p42,p43,p44,p45,p46,
p47,p48,p49,p50,p51,p52,p53,p54,p55,p56,p57,p58,p59,p60,p61,p62,p63,p64,p65,p66,p67,p68,
p69,p70,p71,p72,p73,p74,p75,p76,p77,p78,p79,p80,p81,p82,p83,p84,p85,p86,p87,p88,p89,p90,
p91,p92,p93,p94,p95,p96,p97                                                               |
	p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17,p18,p19,p20,p21,p22,p23,p24,
p25,p26,p27,p28,p29,p30,p31,p32,p33,p34,p35,p36,p37,p38,p39,p40,p41,p42,p43,p44,p45,p46,
p47,p48,p49,p50,p51,p52,p53,p54,p55,p56,p57,p58,p59,p60,p61,p62,p63,p64,p65,p66,p67,p68,
p69,p70,p71,p72,p73,p74,p75,p76,p77,p78,p79,p80,p81,p82,p83,p84,p85,p86,p87,p88,p89,p90,
p91,p92,p93,p94,p95,p96
  ;

partition_name:
	p0 | p1 | p2 | p3 | p4 | p5 | p6 | p7 | p8 | p9 | p10 | p11 | p12 | p13 | p14 | p15 | p16 | p17 | p18 | p19 | p20 | p21 | p22 | p23 | p24 | p25 | p26 | p27 | p28 | p29 | p30 | p31 | p32 | p33 | p34 | p35 | p36 | p37 | p38 | p39 | p40 | p41 | p42 | p43 | p44 | p45 | p46 | p47 | p48 | p49 | p50 | p51 | p52 | p53 | p54 | p55 | p56 | p57 | p58 | p59 | p60 | p61 | p62 | p63 | p64 | p65 | p66 | p67 | p68 | p69 | p70 | p71 | p72 | p73 | p74 | p75 | p76 | p77 | p78 | p79 | p80 | p81 | p82 | p83 | p84 | p85 | p86 | p87 | p88 | p89 | p90 | p91 | p92 | p93 | p94 | p95 | p96 | p97 | p98 | p99  ;

enable_disable:
	ENABLE | DISABLE ;

# Give preference to MyISAM because key caching is specific to MyISAM

engine:
	MYISAM | MYISAM | MYISAM |
	ROCKSDB | MEMORY          ;

partition:
	partition_by_hash  |
	partition_by_key   ;

subpartition:
	|
	SUBPARTITION BY linear HASH ( _field ) SUBPARTITIONS partition_count ;

populate_digits:
	{ @digits = @{$prng->shuffleArray([0..9])} ; return undef };

shift_digit:
	{ shift @digits };

partition_by_hash:
	PARTITION BY linear HASH ( _field ) PARTITIONS partition_count;

linear:
	| LINEAR;

partition_by_key:
	PARTITION BY KEY(`col_int_key`) PARTITIONS partition_count ;

partition_hash_or_key:
	HASH ( field_name ) PARTITIONS partition_count |
	KEY  ( field_name ) PARTITIONS partition_count ;

limit_rows:
	1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 ;

partition_count:
	96 | 97 | 98 | 98 | 98 | 99 | 99 | 99 | 99 ;

if_exists:
	IF EXISTS ;   

if_not_exists:
	IF NOT EXISTS ;
