// LibreSprite | Copyright (C) 2026  LibreSprite contributors
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __MORPHOS__

#include "base/morphos_sys.h"

#include <cstring>

#include <dos/dos.h>
#include <exec/tasks.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/openurl.h>
#include <proto/wb.h>

// Bases de las librerías que no abre el arranque de libnix. Se abren solo
// mientras se usan (abrir una URL o una carpeta es algo puntual).
struct Library* WorkbenchBase = nullptr;
struct Library* OpenURLBase = nullptr;

namespace base {
namespace morphos {

// Tamaño suficiente para rutas de AmigaDOS (volumen + directorios + fichero)
static const int kMaxPath = 1024;

std::string get_app_path()
{
  char dir[kMaxPath] = {0};
  char name[256] = {0};

  // Directorio del programa: el lock de PROGDIR: convertido a ruta absoluta
  BPTR lock = GetProgramDir();
  if (!lock || !NameFromLock(lock, (STRPTR)dir, sizeof(dir)))
    std::strcpy(dir, "PROGDIR:");

  // Nombre del ejecutable. Desde el Shell lo da GetProgramName(); desde
  // Workbench no hay CLI y se usa el nombre de la tarea.
  if (!GetProgramName((STRPTR)name, sizeof(name)) || !name[0]) {
    struct Task* task = FindTask(nullptr);
    const char* taskName = (task && task->tc_Node.ln_Name) ? task->tc_Node.ln_Name : "libresprite";
    std::strncpy(name, taskName, sizeof(name) - 1);
  }

  AddPart((STRPTR)dir, FilePart((STRPTR)name), sizeof(dir));
  return dir;
}

std::string get_canonical_path(const std::string& path)
{
  BPTR lock = Lock((CONST_STRPTR)path.c_str(), SHARED_LOCK);
  if (!lock)
    return path;

  char buffer[kMaxPath];
  std::string result = path;
  if (NameFromLock(lock, (STRPTR)buffer, sizeof(buffer)))
    result = buffer;
  UnLock(lock);
  return result;
}

bool open_url(const std::string& url)
{
  OpenURLBase = OpenLibrary((CONST_STRPTR)"openurl.library", 0);
  if (!OpenURLBase)
    return false;
  ULONG ok = URL_OpenA((STRPTR)url.c_str(), nullptr);
  CloseLibrary(OpenURLBase);
  OpenURLBase = nullptr;
  return ok != 0;
}

bool open_workbench_object(const std::string& path)
{
  // OpenWorkbenchObject está en workbench.library v44+
  WorkbenchBase = OpenLibrary((CONST_STRPTR)"workbench.library", 44);
  if (!WorkbenchBase)
    return false;
  BOOL ok = OpenWorkbenchObjectA((STRPTR)path.c_str(), nullptr);
  CloseLibrary(WorkbenchBase);
  WorkbenchBase = nullptr;
  return ok != FALSE;
}

} // namespace morphos
} // namespace base

// libnix/libc.a trae isnan() e isnanf(), pero no __isnanf() ni __isnanl(), que usa
// la macro isnan() de <math.h> con float y long double. Si faltan, el enlazador las
// busca en ppc-morphos/lib/libm.a y se trae s_isnan.o entero, que también define
// isnan(): "definiciones múltiples de isnan". Con estas dos, s_isnan.o no hace falta.
extern "C" {
int __isnanf(float x) { return __builtin_isnan(x); }
int __isnanl(long double x) { return __builtin_isnan(x); }
}

#endif // __MORPHOS__
