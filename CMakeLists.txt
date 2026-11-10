# Modern Windows build for pgAdmin III (Visual Studio 2022/2026 + vcpkg).
#
# Configure:
#   cmake -S . -B build -G "Visual Studio 18 2026" -A x64 ^
#         -DCMAKE_TOOLCHAIN_FILE=C:/software/vcpkg/scripts/buildsystems/vcpkg.cmake ^
#         -DPostgreSQL_ROOT="C:/Program Files/PostgreSQL/15"
# Build:
#   cmake --build build --config Release

cmake_minimum_required(VERSION 3.21)
project(pgAdmin3 LANGUAGES C CXX)

if(NOT WIN32)
  message(FATAL_ERROR "This CMake build only targets Windows; use autotools elsewhere.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PGA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/pgadmin)
set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/gen)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
find_package(wxWidgets CONFIG REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(LibXslt REQUIRED)
find_package(Iconv REQUIRED)
find_package(libssh2 CONFIG REQUIRED)
find_package(PostgreSQL REQUIRED)   # libpq; set PostgreSQL_ROOT

# PostgreSQL server headers are needed only for parser/kwlist.h
find_path(PG_SERVER_INCLUDE_DIR parser/kwlist.h
          HINTS ${PostgreSQL_ROOT}/include/server ${PostgreSQL_INCLUDE_DIRS}/server
          REQUIRED)

# ---------------------------------------------------------------------------
# png2c: converts the PNG icons into C arrays (*.pngc) included by the code
# ---------------------------------------------------------------------------
add_executable(png2c xtra/png2c/png2c.c)
target_compile_definitions(png2c PRIVATE _CRT_SECURE_NO_WARNINGS)

file(GLOB PGA_PNGS ${PGA_DIR}/include/images/*.png)
set(PGA_PNGCS)
foreach(png IN LISTS PGA_PNGS)
  get_filename_component(name ${png} NAME)
  set(out ${GEN_DIR}/images/${name}c)
  add_custom_command(OUTPUT ${out}
                     COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/images
                     COMMAND png2c ${png} ${out}
                     DEPENDS png2c ${png}
                     VERBATIM)
  list(APPEND PGA_PNGCS ${out})
endforeach()
add_custom_target(pgadmin3_images DEPENDS ${PGA_PNGCS})

# ---------------------------------------------------------------------------
# pgAdmin3
# ---------------------------------------------------------------------------
file(GLOB_RECURSE PGA_SOURCES CONFIGURE_DEPENDS
     ${PGA_DIR}/*.cpp ${PGA_DIR}/*.c ${PGA_DIR}/*.cc)
# The bundled libssh2 (2010, OpenSSL 0.9 API) is replaced by vcpkg's libssh2.
list(FILTER PGA_SOURCES EXCLUDE REGEX "/pgadmin/libssh2/")
list(FILTER PGA_SOURCES EXCLUDE REGEX "/pgadmin/precomp\\.cpp$")

# svnversion.h (revision shown in the About box) - now taken from git
find_package(Git QUIET)
set(PGA_REVISION "unknown")
if(GIT_FOUND)
  execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                  OUTPUT_VARIABLE PGA_REVISION OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
endif()
file(WRITE ${GEN_DIR}/svnversion.h.tmp "#define VERSION_SVN \"${PGA_REVISION}\"\n")
file(COPY_FILE ${GEN_DIR}/svnversion.h.tmp ${GEN_DIR}/svnversion.h ONLY_IF_DIFFERENT)

add_executable(pgAdmin3 WIN32 ${PGA_SOURCES} ${PGA_DIR}/pgAdmin3.rc)
add_dependencies(pgAdmin3 pgadmin3_images)

target_include_directories(pgAdmin3 PRIVATE
  ${GEN_DIR}
  ${PGA_DIR}/include
  ${PGA_DIR}
  ${PG_SERVER_INCLUDE_DIR})

target_compile_definitions(pgAdmin3 PRIVATE
  _CRT_SECURE_NO_DEPRECATE=1 _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS
  WIN32 _WINDOWS __WINDOWS__ __WIN32__ STRICT NOMINMAX
  UNICODE _UNICODE wxUSE_UNICODE=1
  EMBED_XRC PG_SSL HAVE_CONNINFO_PARSE
  HAVE_OPENSSL_CRYPTO)   # enables the SSH tunnel (libssh2 + OpenSSL)

if(MSVC)
  target_compile_options(pgAdmin3 PRIVATE /MP /W3 /utf-8 /permissive /Zc:wchar_t)
  # Embed wxWidgets' manifest (common controls v6 = modern Windows look, and
  # per-monitor v2 DPI awareness = sharp rendering at 125%/150%/200%) instead
  # of the linker's default one.
  set_source_files_properties(${PGA_DIR}/pgAdmin3.rc PROPERTIES
    COMPILE_DEFINITIONS "wxUSE_RC_MANIFEST=1;wxUSE_DPI_AWARE_MANIFEST=2")
  target_link_options(pgAdmin3 PRIVATE /MANIFEST:NO)
endif()

target_link_libraries(pgAdmin3 PRIVATE
  wx::core wx::base wx::adv wx::html wx::aui wx::stc wx::xrc wx::xml wx::net
  LibXml2::LibXml2 LibXslt::LibXslt Iconv::Iconv
  libssh2::libssh2
  PostgreSQL::PostgreSQL
  comctl32 rpcrt4 ws2_32 winmm)

# Copy libpq and its runtime DLLs next to the executable
file(GLOB PG_RUNTIME_DLLS ${PostgreSQL_ROOT}/bin/libpq.dll
     ${PostgreSQL_ROOT}/bin/libssl-*.dll ${PostgreSQL_ROOT}/bin/libcrypto-*.dll
     ${PostgreSQL_ROOT}/bin/libintl-*.dll ${PostgreSQL_ROOT}/bin/libiconv-*.dll
     ${PostgreSQL_ROOT}/bin/libwinpthread-*.dll)
foreach(dll IN LISTS PG_RUNTIME_DLLS)
  add_custom_command(TARGET pgAdmin3 POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${dll} $<TARGET_FILE_DIR:pgAdmin3>)
endforeach()
