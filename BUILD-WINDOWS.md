# Building pgAdmin III on Windows (PostgreSQL 13–18)

This branch builds pgAdmin III with Visual Studio 2022/2026 (x64), wxWidgets 3.3
and current libraries from vcpkg. The old `pgAdmin3.sln` / `.vcxproj` files
(VS 2010, wxWidgets 2.8/3.0) are kept for reference but are no longer maintained.

## Requirements

- Visual Studio 2022 or 2026 with the "Desktop development with C++" workload
  (includes CMake).
- PostgreSQL 13 or newer installed (only `libpq` and the headers are used),
  e.g. `C:\Program Files\PostgreSQL\18`.
- [vcpkg](https://github.com/microsoft/vcpkg).

## 1. Dependencies (once, about 50 minutes)

```bat
git clone https://github.com/microsoft/vcpkg C:\software\vcpkg
C:\software\vcpkg\bootstrap-vcpkg.bat -disableMetrics
C:\software\vcpkg\vcpkg install wxwidgets libxml2 libxslt libiconv libssh2 --triplet x64-windows
```

## 2. Configure and build

Run from the repository root:

```bat
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 ^
      -DCMAKE_TOOLCHAIN_FILE=C:/software/vcpkg/scripts/buildsystems/vcpkg.cmake ^
      -DVCPKG_TARGET_TRIPLET=x64-windows ^
      -DPostgreSQL_ROOT="C:/Program Files/PostgreSQL/18"
cmake --build build --config Release
```

For Visual Studio 2022 use `-G "Visual Studio 17 2022"`. The program and every
DLL it needs end up in `build\Release\pgAdmin3.exe`.

## Editing dialogs (XRC)

The dialogs in `pgadmin/ui/*.xrc` are embedded into `pgadmin/ui/xrcDialogs.cpp`.
After changing an `.xrc` file, regenerate it from `pgadmin/ui`:

```bat
C:\software\vcpkg\installed\x64-windows\tools\wxwidgets\wxrc.exe -c -o xrcDialogs.cpp *.xrc
```

wxWidgets 3.x rejects sizer flags that 2.8 ignored (for example
`wxALIGN_CENTRE` together with `wxEXPAND`). `python xtra/fix-xrc-flags.py pgadmin/ui`
removes those combinations.

## Tests

`tests/pg_compat_queries.sql` runs the changed catalog queries against a server;
it must end with `== ALL QUERIES OK`:

```bat
psql -h localhost -U postgres -f tests/pg_compat_queries.sql
```

`tests/demo_objects.sql` creates a `demo` database with partitioned tables,
identity and generated columns, a procedure and a sequence for testing in the GUI.
