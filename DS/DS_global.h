#ifndef DS_GLOBAL_H
#define DS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(DS_LIBRARY)
#  define DSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define DSSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // DS_GLOBAL_H
