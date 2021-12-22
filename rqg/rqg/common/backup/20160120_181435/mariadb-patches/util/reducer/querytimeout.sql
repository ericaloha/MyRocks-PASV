# querytimeout.sql: this file is here to be merged with testcases as generated by reducer.sh, when necessary. reducer.sh uses this same sql code, and in some odd cases testcases may not replay correctly, as this sql code is _not_ included in the final testcase. In such cases, you can add this sql code back in at the top of the testcase as generated by reducer. Note that this code requires --event-scheduler=ON option to mysqld.
DELIMITER ||
CREATE EVENT querytimeout ON SCHEDULE EVERY 20 SECOND DO BEGIN
SET @id:='';
SET @id:=(SELECT id FROM INFORMATION_SCHEMA.PROCESSLIST WHERE ID<>CONNECTION_ID() AND STATE<>'killed' AND TIME>$QUERYTIMEOUT ORDER BY TIME DESC LIMIT 1);
IF @id > 1 THEN KILL QUERY @id; END IF;
END ||
DELIMITER ;