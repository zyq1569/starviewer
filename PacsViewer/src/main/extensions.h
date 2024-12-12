#ifndef EXTENSIONS_H
#define EXTENSIONS_H
//#include "../extensions/main/mpr/mprextensionmediator.h"
#include "../extensions/main/mpr3d/mpr3dextensionmediator.h"
//#include "../extensions/main/dicomprint/dicomprintextensionmediator.h"
#include "../extensions/main/q2dviewer/q2dviewerextensionmediator.h"
#include "../extensions/main/q3dviewer/q3dviewerextensionmediator.h"
//#include "../extensions/main/pdf/pdfextensionmediator.h"
//#include "../extensions/playground/diffusionperfusionsegmentation/diffusionperfusionsegmentationextensionmediator.h"
//#include "../extensions/playground/edemasegmentation/edemasegmentationextensionmediator.h"
//#include "../extensions/playground/angiosubstraction/angiosubstractionextensionmediator.h"
//#include "../extensions/playground/perfusionmapreconstruction/perfusionmapreconstructionextensionmediator.h"
//#include "../extensions/playground/rectumsegmentation/rectumsegmentationextensionmediator.h"
//#include "../extensions/playground/experimental3d/experimental3dextensionmediator.h"
//#include "../extensions/playground/example/exampleextensionmediator.h"
void initExtensionsResources()
{
//Q_INIT_RESOURCE(mpr);
//Q_INIT_RESOURCE(dicomprint);
Q_INIT_RESOURCE(q2dviewer);
//Q_INIT_RESOURCE(mpr3D);
Q_INIT_RESOURCE(q3dviewer);
//Q_INIT_RESOURCE(pdf);
//Q_INIT_RESOURCE(diffusionperfusionsegmentation);
//Q_INIT_RESOURCE(edemasegmentation);
//Q_INIT_RESOURCE(angiosubstraction);
//Q_INIT_RESOURCE(perfusionmapreconstruction);
//Q_INIT_RESOURCE(rectumsegmentation);
//Q_INIT_RESOURCE(experimental3d);
//Q_INIT_RESOURCE(example);
}
#endif 
