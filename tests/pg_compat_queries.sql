-- Validates the catalog queries changed for PostgreSQL 13+ compatibility.
-- Run with: psql -v ON_ERROR_STOP=1 -f tests/pg_compat_queries.sql
\set ON_ERROR_STOP 1
DROP SCHEMA IF EXISTS pga_test CASCADE;
CREATE SCHEMA pga_test;
SET search_path = pga_test;

CREATE TABLE t_plain (id serial PRIMARY KEY, v text DEFAULT 'x', CHECK (length(v) < 100));
CREATE TABLE t_part (id int, dt date) PARTITION BY RANGE (dt);
CREATE TABLE t_part_2026 PARTITION OF t_part FOR VALUES FROM ('2026-01-01') TO ('2027-01-01');
CREATE SEQUENCE s_test INCREMENT 5 MINVALUE 10 MAXVALUE 1000 CACHE 3 CYCLE;
CREATE FUNCTION f_plain(int) RETURNS int LANGUAGE sql AS 'SELECT $1';
CREATE PROCEDURE p_test() LANGUAGE sql AS 'SELECT 1';
CREATE AGGREGATE a_test(int) (SFUNC = int4pl, STYPE = int);

\echo '== pgConn: connection startup (datlastsysoid replacement)'
SELECT oid, pg_encoding_to_char(encoding) AS encoding, 16383::oid AS datlastsysoid
  FROM pg_database WHERE datname = current_database();

\echo '== pgServer: WAL functions'
SELECT CASE WHEN usesuper THEN pg_is_in_recovery() ELSE NULL END as inrecovery,
       CASE WHEN usesuper THEN pg_last_wal_receive_lsn() ELSE NULL END as receiveloc,
       CASE WHEN usesuper THEN pg_last_wal_replay_lsn() ELSE NULL END as replayloc,
       CASE WHEN usesuper AND pg_is_in_recovery() THEN pg_is_wal_replay_paused() ELSE NULL END as isreplaypaused
  FROM pg_user WHERE usename = current_user;
SELECT state || ' (' || sent_lsn || ' sent, ' || write_lsn || ' written, ' || flush_lsn || ' flushed, ' || replay_lsn || ' applied)' AS query
  FROM pg_stat_replication;

\echo '== pgSequence: values from pg_sequence'
SELECT s.last_value, p.seqmin AS min_value, p.seqmax AS max_value, p.seqcache AS cache_value,
       p.seqcycle AS is_cycled, p.seqincrement AS increment_by, s.is_called
  FROM pga_test.s_test s, pg_catalog.pg_sequence p
 WHERE p.seqrelid = 'pga_test.s_test'::regclass;

\echo '== pgFunction: plain functions only (expect f_plain only)'
SELECT proname, prokind FROM pg_proc
 WHERE prokind IN ('f', 'w') AND pronamespace = 'pga_test'::regnamespace;

\echo '== pgObject: dependencies with pg_get_expr instead of adsrc'
SELECT DISTINCT ad.adbin IS NOT NULL AS has_adbin, pg_get_expr(ad.adbin, ad.adrelid) AS adsrc
  FROM pg_attrdef ad WHERE ad.adrelid = 'pga_test.t_plain'::regclass;

\echo '== pgTable: listing with partitions (expect 3 tables)'
SELECT rel.relname, rel.relkind, false AS relhasoids,
       CASE WHEN rel.relkind = 'p' THEN pg_get_partkeydef(rel.oid) END AS partkeydef,
       CASE WHEN rel.relispartition THEN pg_get_expr(rel.relpartbound, rel.oid) END AS partbound
  FROM pg_class rel
 WHERE rel.relkind IN ('r','s','t','p') AND rel.relnamespace = 'pga_test'::regnamespace
 ORDER BY rel.relname;

\echo '== Generated DDL must be accepted by the server'
CREATE TABLE ddl_plain (id int) WITH (OIDS=FALSE);
CREATE TABLE ddl_part (id int, dt date) PARTITION BY RANGE (dt);
CREATE TABLE ddl_part_2026 PARTITION OF ddl_part
FOR VALUES FROM ('2026-01-01') TO ('2027-01-01')
WITH (OIDS=FALSE);

\echo '== ALL QUERIES OK'
DROP SCHEMA pga_test CASCADE;
