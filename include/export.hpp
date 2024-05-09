#ifndef EXPORT_HPP
#define EXPORT_HPP

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__) || defined(__MINGW32__)
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#endif /* EXPORT_HPP */

