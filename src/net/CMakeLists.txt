# Aseprite Network Library
# Copyright (C) 2001-2015 David Capello

if(MORPHOS)
  # Sin CURL: implementación nula (HTTP::fetch de los scripts devuelve error).
  # LIBRESPRITE_NO_CURL fuerza la versión nula aunque exista <curl/curl.h> (el SDK la trae).
  add_library(net-lib
    http_request_null.cpp
    http_response.cpp)
  target_compile_definitions(net-lib PRIVATE LIBRESPRITE_NO_CURL)
else()
  add_library(net-lib
    http_request.cpp
    http_response.cpp)

  target_link_libraries(net-lib
    ${CURL_LIBRARIES})
endif()
