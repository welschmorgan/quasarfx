//
// Created by darkboss on 8/21/20.
//

#ifndef QUASARFX_PLATFORM_H
# define QUASARFX_PLATFORM_H

# define QUASAR_DLL_EXPORT __declspec(dllexport)
# define QUASAR_DLL_IMPORT __declspec(dllimport)

# ifndef QUASAR_BUILD_STATIC_LIBS
#  ifdef QUASAR_BUILD_SHARED_LIBS
#   define QUASAR_API QUASAR_DLL_EXPORT
#  else
#   define QUASAR_API QUASAR_DLL_IMPORT
#  endif
# else
#  define QUASAR_API
# endif

#endif //QUASARFX_PLATFORM_H
