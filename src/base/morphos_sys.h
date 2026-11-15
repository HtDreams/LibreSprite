// LibreSprite | Copyright (C) 2026  LibreSprite contributors
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.
//
// MorphOS: funciones de sistema implementadas con las librerías nativas
// (dos.library, workbench.library, openurl.library). Sustituyen a las POSIX
// que libnix declara pero no implementa (readlink, realpath, posix_spawnp...).
// Las cabeceras de Amiga quedan aisladas en morphos_sys.cpp.

#pragma once

#ifdef __MORPHOS__

#include <string>
#include <vector>

namespace base {
namespace morphos {

// Ruta completa del ejecutable (equivale a readlink("/proc/self/exe") en Linux)
std::string get_app_path();

// Ruta absoluta y sin asignaciones lógicas (equivale a realpath()).
// Si el fichero no existe devuelve la ruta tal cual.
std::string get_canonical_path(const std::string& path);

// Abre una URL con el navegador configurado (openurl.library)
bool open_url(const std::string& url);

// Abre un fichero con su herramienta por defecto o una carpeta en Ambient,
// como un doble clic (workbench.library, OpenWorkbenchObject)
bool open_workbench_object(const std::string& path);

// Nombres de los volúmenes montados ("Work:", "RAM Disk:"...), como la lista de
// volúmenes del Workbench. Sirve de raíz del diálogo de ficheros (en Unix es "/").
std::vector<std::string> list_volumes();

} // namespace morphos
} // namespace base

#endif
