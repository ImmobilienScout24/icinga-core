-- -----------------------------------------
-- upgrade path for Icinga IDOUtils 1.4.0
--
-- run it as icinga database user whithin  current directory 
-- sqlplus icinga@<instance> @ oracle-upgrade.1-4.0.sql
-- -----------------------------------------
-- Copyright (c) 2010-2011 Icinga Development Team (http://www.icinga.org)
--
-- Please check http://docs.icinga.org for upgrading information!
-- -----------------------------------------
set sqlprompt "&&_USER@&&_CONNECT_IDENTIFIER SQL>"
set pagesize 200;
set linesize 200;
set heading on;
set echo on;
set feedback on;

define ICINGA_VERSION=1.4.0

-- --------------------------------------------------------
-- warning:edit this script to define existing tablespaces
-- this particular step can be skipped safetly
-- --------------------------------------------------------
/* set real TBS names on which you have quota, no checks are implemented!*/
define DATATBS='ICINGA_DATA';
define LOBTBS='ICINGA_DATA';
define IXTBS='ICINGA_IDX';

-- --------------------------------------------------------
-- rewrite objects
-- --------------------------------------------------------
@alter_icinga13_objects.sql
-- --------------------------------------------------------
-- recreate functions
-- --------------------------------------------------------
@recreate_icinga13_functions.sql

-- --------------------------------------------------------
-- remove number limitations
-- fixes Bug #1173: int(11) to small for some of ido tables
-- https://dev.icinga.org/issues/1173
-- --------------------------------------------------------
@alter_icinga13_numbers.sql

/* script will be terminated on the first error */
whenever sqlerror exit failure
spool oracle-upgrade-&&ICINGA_VERSION..log


-- -----------------------------------------
-- finally update dbversion
-- -----------------------------------------

MERGE INTO dbversion
	USING DUAL ON (name='idoutils')
	WHEN MATCHED THEN
		UPDATE SET version='&&ICINGA_VERSION'
	WHEN NOT MATCHED THEN
		INSERT (id, name, version) VALUES ('1', 'idoutils', '&&ICINGA_VERSION');

/* last check */
select object_name,object_type,status  from user_objects where status !='VALID';

/* goodbye */
spool off 
exit;

