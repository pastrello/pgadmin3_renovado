# pgAdmin III renovado

O clássico **pgAdmin III**, leve e nativo, atualizado para funcionar com o
**PostgreSQL 13 a 18** e compilado para o Windows atual.

O pgAdmin III foi descontinuado em 2016 (última versão suportava até o
PostgreSQL 9.6) e, a partir do PostgreSQL 15, nem conseguia mais conectar.
Este projeto parte do código original
([pgadmin-org/pgadmin3](https://github.com/pgadmin-org/pgadmin3), com todo o
histórico preservado) e corrige o que mudou no catálogo do PostgreSQL e no
wxWidgets desde então.

*English: a maintained fork of pgAdmin III that works with PostgreSQL 13–18 and
builds on Windows with Visual Studio 2026 and wxWidgets 3.3.*

## Download

Baixe o zip para Windows x64 na página de
[**Releases**](https://github.com/pastrello/pgadmin3_renovado/releases/latest),
extraia em qualquer pasta e execute `pgAdmin3.exe`. Não precisa instalar nada.

## O que mudou

| Área | Mudança |
|---|---|
| Conexão | Funciona no PostgreSQL 15+ (`datlastsysoid` foi removido); faixa suportada 13–18 |
| Tabelas | Tabelas particionadas aparecem na árvore; DDL com `PARTITION BY` / `PARTITION OF` |
| Colunas | DDL de colunas identity e geradas (`STORED` e `VIRTUAL`) |
| Funções | Procedures não aparecem mais como funções (`prokind`, PG 11+) |
| Catálogo | Sequências (`pg_sequence`), replicação (`pg_wal_*`, `*_lsn`), `relhasoids` e `adsrc` removidos no PG 12 |
| Interface | Portado para wxWidgets 3.3: APIs removidas e 1059 flags de layout dos diálogos XRC |
| Build | CMake + vcpkg com Visual Studio 2022/2026 no lugar do projeto do VS 2010 |
| Bibliotecas | OpenSSL 3, libssh2 1.11, libxml2 2.15, libpq do PostgreSQL 18 |

Testado contra PostgreSQL 15.2 e 18.6.

## Limitações conhecidas

- Procedures, publications/subscriptions e outros objetos recentes ainda não
  têm tela própria.
- A ajuda (docs) não está incluída no pacote.
- O build mantido é o de Windows. O build original com autotools
  (Linux/macOS) continua no repositório, mas não foi testado.
- O suporte a Greenplum, EnterpriseDB e Slony-I veio do código original e não
  foi testado.

## Compilar

Veja [BUILD-WINDOWS.md](BUILD-WINDOWS.md). Resumo:

```bat
vcpkg install wxwidgets libxml2 libxslt libiconv libssh2 --triplet x64-windows
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 ^
      -DCMAKE_TOOLCHAIN_FILE=C:/software/vcpkg/scripts/buildsystems/vcpkg.cmake ^
      -DPostgreSQL_ROOT="C:/Program Files/PostgreSQL/18"
cmake --build build --config Release
```

## Testes

- [`tests/pg_compat_queries.sql`](tests/pg_compat_queries.sql) roda contra um
  servidor as consultas de catálogo que foram alteradas e precisa terminar com
  `== ALL QUERIES OK`.
- [`tests/demo_objects.sql`](tests/demo_objects.sql) cria um banco `demo` com
  partições, identity, coluna gerada, procedure e sequência, para testar na
  interface.

## Créditos e licença

pgAdmin III foi desenvolvido pelo pgAdmin Development Team entre 2002 e 2016.
O README original está em [README-original.txt](README-original.txt).

Distribuído sob a [PostgreSQL Licence](LICENSE), a mesma do projeto original.
