#ifndef EXTENSIONS_H
#define EXTENSIONS_H
#include "../extensions/main/mpr3d/mpr3dextensionmediator.h"
#include "../extensions/main/dicomprint/dicomprintextensionmediator.h"
#include "../extensions/main/q2dviewer/q2dviewerextensionmediator.h"
#include "../extensions/main/q3dviewer/q3dviewerextensionmediator.h"
void initExtensionsResources()
{
Q_INIT_RESOURCE(mpr3d);
Q_INIT_RESOURCE(dicomprint);
Q_INIT_RESOURCE(q2dviewer);
Q_INIT_RESOURCE(q3dviewer);
}
#endif
