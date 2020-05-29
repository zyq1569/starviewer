#ifndef UDGEXPERIMENTAL3DSETTINGS_H
#define UDGEXPERIMENTAL3DSETTINGS_H

#include "defaultsettings.h"

namespace udg {

class Experimental3DSettings : public DefaultSettings {

public:

    Experimental3DSettings();
    virtual ~Experimental3DSettings();

    virtual void init();

    /// Declaració de claus
    static const QString TransferFunctionDir;
    static const QString ColorTransferFunctionDir;
    static const QString ImportanceDir;
    static const QString ViewedVolumeDir;
    static const QString HVzDir;
    static const QString HVDir;
    static const QString HZvDir;
    static const QString HZVDir;
    static const QString HZDir;
    static const QString ViewpointMutualInformationDir;
    static const QString ViewpointMutualInformation2Dir;
    static const QString ViewpointMutualInformation3Dir;
    static const QString MutualInformationDir;
    static const QString ViewpointUnstabilitiesDir;
    static const QString VoxelMutualInformationDir;
    static const QString VoxelMutualInformation2Dir;
    static const QString VoxelMutualInformation3Dir;
    static const QString BestViewsDir;
    static const QString GuidedTourDir;
    static const QString ExploratoryTourDir;
    static const QString CameraDir;
    static const QString ObscuranceDir;
    static const QString ProgramDir;
    static const QString TourDir;
    static const QString ViewpointVoxelMutualInformationDir;
    static const QString ViewpointVoxelMutualInformation2Dir;
    static const QString ColorVoxelMutualInformationPaletteDir;
    static const QString ColorVoxelMutualInformationDir;
    static const QString EnhancedViewpointMutualInformationOpacityDir;
    static const QString EnhancedViewpointMutualInformationVoxelMutualInformationDir;
    static const QString ViewedVolumeIntensityDir;
    static const QString HIDir;
    static const QString HIvDir;
    static const QString HIVDir;
    static const QString ViewpointMutualInformationIntensityDir;
    static const QString MutualInformationIntensityDir;
    static const QString ViewpointUnstabilitiesIntensityDir;
    static const QString IntensityMutualInformationDir;

};

} // end namespace udg 

#endif // UDGEXPERIMENTAL3DSETTINGS_H
