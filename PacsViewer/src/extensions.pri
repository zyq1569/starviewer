# Here we need to add the extensions we want to be included in the executable
#

# Exclude playground extensions from official releases
!official_release {
    PLAYGROUND_EXTENSIONS = diffusionperfusionsegmentation \
                            edemasegmentation \
                            angiosubstraction \
                            perfusionmapreconstruction \
                            rectumsegmentation \
                            experimental3d \
                            example
}

#CONTRIB_EXTENSIONS =

MAIN_EXTENSIONS = mpr \
                  dicomprint \
                  q2dviewer \
                  q3dviewer \
                  pdf

#
# In case we do a build of the "lite" version we will only have the 2D extension and that's it
#
lite_version {
    PLAYGROUND_EXTENSIONS = ''
    MAIN_EXTENSIONS = q2dviewer
}

#Extension is not included because it fails to duplicate symbols when linking
macx {
        PLAYGROUND_EXTENSIONS -= perfusionmapreconstruction \
}
