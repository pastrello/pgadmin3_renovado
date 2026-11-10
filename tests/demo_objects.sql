-- Sample objects to exercise pgAdmin III against PostgreSQL 13+ in the GUI.
-- psql -h localhost -p <port> -U postgres -f tests/demo_objects.sql
\set ON_ERROR_STOP 1
DROP DATABASE IF EXISTS demo;
CREATE DATABASE demo;
\c demo
CREATE TABLE vendas (id bigint GENERATED ALWAYS AS IDENTITY, dt date NOT NULL,
                     valor numeric(10,2), imposto numeric GENERATED ALWAYS AS (valor * 0.1) STORED)
  PARTITION BY RANGE (dt);
CREATE TABLE vendas_2025 PARTITION OF vendas FOR VALUES FROM ('2025-01-01') TO ('2026-01-01');
CREATE TABLE vendas_2026 PARTITION OF vendas FOR VALUES FROM ('2026-01-01') TO ('2027-01-01');
CREATE TABLE clientes (id serial PRIMARY KEY, nome text NOT NULL, criado timestamptz DEFAULT now());
CREATE SEQUENCE seq_pedidos INCREMENT 5 START 100;
CREATE PROCEDURE proc_teste() LANGUAGE sql AS 'SELECT 1';
CREATE FUNCTION soma(a int, b int) RETURNS int LANGUAGE sql AS 'SELECT a+b';
INSERT INTO vendas (dt, valor) VALUES ('2025-06-01', 100), ('2026-03-10', 250);
INSERT INTO clientes (nome) VALUES ('Ana'), ('Bruno');
