// LibreSprite | Copyright (C) 2026  LibreSprite contributors
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.
//
// MorphOS: corrección de setlocale() de libnix.
//
// setlocale(LC_ALL, NULL) de libnix devuelve una cadena compuesta con una línea
// por categoría ("C\nC\nC\nC\nC": LC_COLLATE..LC_TIME), pero setlocale(LC_ALL, ...)
// no acepta esa misma cadena: solo restaura la primera categoría y el resto pasa a
// la configuración regional del sistema (locale.prefs).
//
// La libstdc++ guarda y restaura así el locale cada vez que convierte números en un
// stream (istringstream >> double...). Tras el primer stream, LC_NUMERIC queda con el
// separador decimal de MorphOS y strtod("1.5") devuelve 1.0, snprintf("%.2f") escribe
// "1'50", etc. (error "version 1.5 or above required" al abrir .anim).
//
// Se enlaza con -Wl,--wrap=setlocale: todas las llamadas a setlocale(), también las de
// la libstdc++ precompilada, pasan por __wrap_setlocale(), que restaura la cadena
// compuesta categoría a categoría.

#ifdef __MORPHOS__

#include <clocale>
#include <cstring>
#include <string>

extern "C" {

char* __real_setlocale(int category, const char* locale);

char* __wrap_setlocale(int category, const char* locale)
{
  if (category == LC_ALL && locale && std::strchr(locale, '\n')) {
    // Cadena compuesta: una línea por categoría, de LC_COLLATE (1) a LC_TIME (5)
    const std::string all(locale);
    std::string::size_type start = 0;
    for (int cat = LC_COLLATE; cat <= LC_TIME; ++cat) {
      std::string::size_type end = all.find('\n', start);
      std::string part = all.substr(start, end == std::string::npos ? std::string::npos
                                                                    : end - start);
      if (!part.empty())
        __real_setlocale(cat, part.c_str());
      if (end == std::string::npos)
        break;
      start = end + 1;
    }
    return __real_setlocale(LC_ALL, nullptr);
  }
  return __real_setlocale(category, locale);
}

} // extern "C"

#endif // __MORPHOS__
