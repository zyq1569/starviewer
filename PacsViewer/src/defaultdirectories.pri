# Directoris per defecte de les diferents llibreries. Si vols que siguin uns altres, simplement has de declarar
# com a variables de sistema les que vulguis substituir. Ex.: export ITKLIBDIR=/usr/lib64/InsightToolkit

# Default install prefix is /usr/local
SDK_INSTALL_PREFIX = D:\SDK
isEmpty(SDK_INSTALL_PREFIX){
    unix:SDK_INSTALL_PREFIX = $$(HOME)/starviewer-sdk-0.15/usr/local
    win32:SDK_INSTALL_PREFIX = $$(USERPROFILE)/starviewer-sdk-0.15/32
    win32:contains(QMAKE_TARGET.arch, x86_64):SDK_INSTALL_PREFIX = $$(USERPROFILE)/starviewer-sdk-0.15/64
}

# DCMTK Libraries

DCMTKLIBDIR = $$(DCMTKLIBDIR)
isEmpty(DCMTKLIBDIR){
    unix:DCMTKLIBDIR = $$SDK_INSTALL_PREFIX/lib
    win32:DCMTKLIBDIR = $$SDK_INSTALL_PREFIX/starviewer-win64-5.0.1-8.2.0-3.0.0/dcmtk/3.6.5/lib
}
DCMTKINCLUDEDIR = $$(DCMTKINCLUDEDIR)
isEmpty(DCMTKINCLUDEDIR){
    unix:DCMTKINCLUDEDIR = $$SDK_INSTALL_PREFIX/include/dcmtk
    win32:DCMTKINCLUDEDIR = $$SDK_INSTALL_PREFIX/starviewer-win64-5.0.1-8.2.0-3.0.0/dcmtk/3.6.5/include/dcmtk
}


# VTK Libraries

VTKLIBDIR = $$(VTKLIBDIR)
isEmpty(VTKLIBDIR){
    unix:VTKLIBDIR = $$SDK_INSTALL_PREFIX/lib
    win32:VTKLIBDIR = $$SDK_INSTALL_PREFIX/VTK_8.2.0_lib
}
VTKINCLUDEDIR = $$(VTKINCLUDEDIR)
isEmpty(VTKINCLUDEDIR){
    unix:VTKINCLUDEDIR     = $$SDK_INSTALL_PREFIX/include/vtk-8.1
    win32:VTKINCLUDEDIR    = $$SDK_INSTALL_PREFIX/VTK-8.2.0_VS17_include
    #win32:VTKINCLUDEDIR_VC = $$SDK_INSTALL_PREFIX/VTK-8.2.0
}


# ITK Libraries

ITKLIBDIR = $$(ITKLIBDIR)
isEmpty(ITKLIBDIR){
    unix:ITKLIBDIR = $$SDK_INSTALL_PREFIX/lib
    win32:ITKLIBDIR = $$SDK_INSTALL_PREFIX/ITK_5.3.0_lib
}
ITKINCLUDEDIR = $$(ITKINCLUDEDIR)
isEmpty(ITKINCLUDEDIR){
    unix:ITKINCLUDEDIR     = $$SDK_INSTALL_PREFIX/include/ITK-4.13
    win32:ITKINCLUDEDIR    = $$SDK_INSTALL_PREFIX/ITK-5.3.0_vc17_include
    #win32:ITKINCLUDEDIR_VC = $$SDK_INSTALL_PREFIX/ITK-5.3.0
}



GDCMLIBDIR = $$(GDCMLIBDIR)
isEmpty(GDCMLIBDIR){
    unix:GDCMLIBDIR = $$SDK_INSTALL_PREFIX/lib
    win32:GDCMLIBDIR = $$SDK_INSTALL_PREFIX/starviewer-win64-5.0.1-8.2.0-3.0.0/gdcm/3.0.0/lib
}
GDCMINCLUDEDIR = $$(GDCMINCLUDEDIR)
isEmpty(GDCMINCLUDEDIR){
    unix:GDCMINCLUDEDIR = $$SDK_INSTALL_PREFIX/include/gdcm-3.0
    win32:GDCMINCLUDEDIR = $$SDK_INSTALL_PREFIX/starviewer-win64-5.0.1-8.2.0-3.0.0/gdcm/3.0.0/include/gdcm-3.0
}

# Threadweaver libraries

THREADWEAVERLIBDIR = $$(THREADWEAVERLIBDIR)
isEmpty(THREADWEAVERLIBDIR){
    exists(/etc/debian_version):unix:THREADWEAVERLIBDIR = $$SDK_INSTALL_PREFIX/lib/x86_64-linux-gnu # Debian-based systems
    !exists(/etc/debian_version):unix:THREADWEAVERLIBDIR = $$SDK_INSTALL_PREFIX/lib64               # Other systems
    macx:THREADWEAVERLIBDIR = $$SDK_INSTALL_PREFIX/lib
    win32:THREADWEAVERLIBDIR = $$SDK_INSTALL_PREFIX/threadweaver-5.46.0_lib
}
THREADWEAVERINCLUDEDIR = $$(THREADWEAVERINCLUDEDIR)
isEmpty(THREADWEAVERINCLUDEDIR){
    unix:THREADWEAVERINCLUDEDIR     = $$SDK_INSTALL_PREFIX/include/KF5
    win32:THREADWEAVERINCLUDEDIR    = $$SDK_INSTALL_PREFIX/threadweaver-5.46.0_vc17_include
    #win32:THREADWEAVERINCLUDEDIR_VC = $$SDK_INSTALL_PREFIX/threadweaver-5.46.0
}


# CUDA Libraries

CUDALIBDIR = $$(CUDALIBDIR)
isEmpty(CUDALIBDIR){
    unix:CUDALIBDIR = /usr/local/cuda/lib
}
CUDAINCLUDEDIR = $$(CUDAINCLUDEDIR)
isEmpty(CUDAINCLUDEDIR){
    unix:CUDAINCLUDEDIR = /usr/local/cuda/include
}
#The CUDA SDK is needed at the moment, but you should be able to work without it
CUDASDKINCLUDEDIR = $$(CUDASDKINCLUDEDIR)
isEmpty(CUDASDKINCLUDEDIR){
    unix:CUDASDKINCLUDEDIR = /usr/local/cuda-sdk/common/inc
}
