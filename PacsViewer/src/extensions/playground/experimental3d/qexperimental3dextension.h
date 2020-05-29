#ifndef UDGQEXPERIMENTAL3DEXTENSION_H
#define UDGQEXPERIMENTAL3DEXTENSION_H

#include "ui_qexperimental3dextensionbase.h"

class QStringListModel;

namespace udg {

class Experimental3DVolume;
class Obscurance;
class ObscuranceMainThread;
class Volume;

/**
    Aquesta extensió serveix per fer tot tipus d'experiments relacionats amb la visualització 3D, fent un ray casting amb CPU.
 */
class QExperimental3DExtension : public QWidget, private ::Ui::QExperimental3DExtensionBase {

    Q_OBJECT

public:

    QExperimental3DExtension(QWidget *parent = 0);
    ~QExperimental3DExtension();

    /// Assigna el volum d'entrada.
    void setInput(Volume *input);

    void optimizeTransferFunctionForOneViewpoint();
    int cluster2DIndex(int intensity, int gradient) const;

public slots:

    /// Desa el volum vist a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveViewedVolume(QString fileName = QString());
    /// Carrega l'entropia dels punts de vista H(V) des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadHV(QString fileName = QString());
    /// Desa l'entropia dels punts de vista H(V) a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveHV(QString fileName = QString());
    /// Carrega H(V|z) des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadHVz(QString fileName = QString());
    /// Desa H(V|z) a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveHVz(QString fileName = QString());
    /// Carrega l'entropia dels vòxels H(Z) des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadHZ(QString fileName = QString());
    /// Desa l'entropia dels vòxels H(Z) a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveHZ(QString fileName = QString());
    /// Carrega H(Z|v) des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadHZv(QString fileName = QString());
    /// Desa H(Z|v) a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveHZv(QString fileName = QString());
    /// Carrega H(Z|V) des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadHZV(QString fileName = QString());
    /// Desa H(Z|V) a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveHZV(QString fileName = QString());
    /// Carrega la VMI I₁(v;Z) des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadVmi(QString fileName = QString());
    /// Desa la VMI I₁(v;Z) a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveVmi(QString fileName = QString());
    /// Carrega la VMI2 I₂(v;Z) des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadVmi2(QString fileName = QString());
    /// Desa la VMI2 I₂(v;Z) a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveVmi2(QString fileName = QString());
    /// Carrega la VMI3 I₃(v;Z) des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadVmi3(QString fileName = QString());
    /// Desa la VMI3 I₃(v;Z) a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveVmi3(QString fileName = QString());
    /// Carrega la informació mútua des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadMi(QString fileName = QString());
    /// Desa la informació mútua a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveMi(QString fileName = QString());
    /// Carrega les inestabilitats de les vistes des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadViewpointUnstabilities(QString fileName = QString());
    /// Desa les inestabilitats de les vistes a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveViewpointUnstabilities(QString fileName = QString());
    /// Carrega la VoMI I₁(z;V) des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadVomi(QString fileName = QString());
    /// Desa la VoMI I₁(z;V) a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveVomi(QString fileName = QString());
    /// Carrega la VoMI2 I₂(z;V) des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadVomi2(QString fileName = QString());
    /// Desa la VoMI2 I₂(z;V) a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveVomi2(QString fileName = QString());
    /// Carrega la VoMI3 I₃(z;V) des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadVomi3(QString fileName = QString());
    /// Desa la VoMI3 I₃(z;V) a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveVomi3(QString fileName = QString());
    /// Carrega la viewpoint VoMI (INF) des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadViewpointVomi(QString fileName = QString());
    /// Desa la viewpoint VoMI (INF) a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveViewpointVomi(QString fileName = QString());
    /// Carrega la viewpoint VoMI2 (INF2) des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadViewpointVomi2(QString fileName = QString());
    /// Desa la viewpoint VoMI2 (INF2) a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveViewpointVomi2(QString fileName = QString());
    /// Carrega la paleta per la color VoMI des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadColorVomiPalette(QString fileName = QString());
    /// Carrega la color VoMI des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadColorVomi(QString fileName = QString());
    /// Desa la color VoMI a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveColorVomi(QString fileName = QString());
    /// Carrega des d'un fitxer la funció de transferència d'on s'agafarà l'opacitat per calcular l'EVMI amb opacitat.
    /// Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadEvmiOpacityOtherTransferFunction(QString fileName = QString());
    /// Carrega l'EVMI amb opacitat des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadEvmiOpacity(QString fileName = QString());
    /// Desa la l'EVMI amb opacitat a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveEvmiOpacity(QString fileName = QString());
    /// Carrega l'EVMI amb VoMI des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadEvmiVomi(QString fileName = QString());
    /// Desa la l'EVMI amb VoMI a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveEvmiVomi(QString fileName = QString());
    /// Carrega les millors vistes des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadBestViews(QString fileName = QString());
    /// Desa les millors vistes a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveBestViews(QString fileName = QString());
    /// Carrega el guided tour des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadGuidedTour(QString fileName = QString());
    /// Desa el guided tour a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveGuidedTour(QString fileName = QString());
    /// Carrega l'exploratory tour des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadExploratoryTour(QString fileName = QString());
    /// Desa l'exploratory tour a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveExploratoryTour(QString fileName = QString());

    // Per VMIi etc.
    /// Desa el volum vist a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveViewedVolumeI(QString fileName = QString());
    /// Carrega l'entropia de les intensitats H(I) des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadHI(QString fileName = QString());
    /// Desa l'entropia de les intensitats H(I) a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveHI(QString fileName = QString());
    /// Carrega H(I|v) des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadHIv(QString fileName = QString());
    /// Desa H(I|v) a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveHIv(QString fileName = QString());
    /// Carrega H(I|V) des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadHIV(QString fileName = QString());
    /// Desa H(I|V) a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveHIV(QString fileName = QString());
    void loadVmii(QString fileName = QString());
    /// Desa la VMIi a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveVmii(QString fileName = QString());
    /// Carrega la informació mútua des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadMii(QString fileName = QString());
    /// Desa la informació mútua a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveMii(QString fileName = QString());
    /// Carrega les inestabilitats de les vistes des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadViewpointUnstabilitiesI(QString fileName = QString());
    /// Desa les inestabilitats de les vistes a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveViewpointUnstabilitiesI(QString fileName = QString());
    /// Carrega l'IMI des d'un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void loadImi(QString fileName = QString());
    /// Desa l'IMI a un fitxer. Si no es dóna el nom de fitxer com a paràmetre el demana amb un diàleg.
    void saveImi(QString fileName = QString());

private:

    static const QString FileExtensionsDatAll;
    static const QString FileExtensionsTxtAll;
    static const QString FileExtensionsTxtDatAll;

    /// Carrega dades de tipus T del fitxer a \a data. Retorna cert si s'han carregat les dades.
    template <class T> bool loadData(QString &fileName, const QString &setting, const QString &name, const QString &extensions, T &data);
    template <class T> static void loadData(QDataStream &in, T &data);
    template <class T> static void loadData(QDataStream &in, QVector<T> &data);
    /// Desa dades de tipus T de \a data al fitxer. Retorna cert si s'han desat les dades.
    template <class T> bool saveData(QString &fileName, const QString &setting, const QString &name, const QString &extensions, const QString &defaultSuffix,
                                     const T &data, const QString &textFormat = QString(), int base = 0);
    template <class T> static void saveData(QDataStream &out, const T &data);
    template <class T> static void saveData(QTextStream &out, const T &data, const QString &textFormat, int base);
    template <class T> static void saveData(QDataStream &out, const QVector<T> &data);
    template <class T> static void saveData(QTextStream &out, const QVector<T> &data, const QString &textFormat, int base);
    /// Carrega dades de tipus T del fitxer a la llista. Retorna cert si tot va bé i fals si hi ha error.
    template <class T> static bool loadData(const QString &fileName, QList<T> &list);
    /// Desa dades de tipus T de la llista al fitxer. Retorna cert si tot va bé i fals si hi ha error.
    template <class T> static bool saveData(const QList<T> &list, const QString &fileName);
    /// Desa dades de tipus QPair<int, Vector3> de la llista al fitxer en forma de text amb un format. Retorna cert si tot va bé i fals si hi ha error.
    static bool saveDataAsText(const QList< QPair<int, Vector3> > &list, const QString &fileName, const QString &format, int base1 = 0, int base2 = 0);

    /// Crea les connexions de signals i slots.
    void createConnections();

    /// Llança un diàleg per obtenir un nom de fitxer per llegir.
    QString getFileNameToLoad(const QString &settingsDirKey, const QString &caption, const QString &filter);
    /// Llança un diàleg per obtenir un nom de fitxer per escriure.
    QString getFileNameToSave(const QString &settingsDirKey, const QString &caption, const QString &filter, const QString &defaultSuffix);

    void loadTransferFunction(const QString &fileName);
    void loadColorTransferFunction(const QString &fileName);
    void saveTransferFunction(const QString &fileName);
    void loadCamera(const QString &fileName);
    void transferFunctionOptimizationLoadImportance(const QString &fileName);
    void transferFunctionOptimizationSaveImportance(const QString &fileName);

    /// Fa un recorregut pels viewpoints en ordre i amb suavitat.
    void tour(const QList<Vector3> &viewpoints, double speedFactor = 1.0);

    /// Escriu al log un error d'un programa.
    void logProgramError(int lineNumber, const QString &error, const QString &line) const;
    /// Comprova que la línia tingi un nombre determinat de paraules.
    bool programCheckWordCount(int lineNumber, const QString &line, int wordCount) const;
    bool programTab(int lineNumber, const QString &line, bool run);
    bool programRenderingInterpolation(int lineNumber, const QString &line, bool run);
    bool programRenderingGradientEstimator(int lineNumber, const QString &line, bool run);
    bool programRenderingBaseShading(int lineNumber, const QString &line, bool run);
    bool programRenderingCheckOrUncheck(int lineNumber, const QString &line, bool run);
    bool programVmiViewpoints(int lineNumber, const QString &line, bool run);
    bool programVmiCheckOrUncheck(int lineNumber, const QString &line, bool run);
    bool programVmiLoadOrSave(int lineNumber, const QString &line, bool run);

    void checkData();
    QVector<float> getWeights() const;
    OpacityTransferFunction innernessProportionalOpacityTransferFunction(const Experimental3DVolume *volume) const;

    void syncNormalToGradientTransferFunction();
    void syncGradientToNormalTransferFunction();
    int numberOfClusters() const;
    int cluster2DIndexFromBins(int intensityBin, int gradientBin) const;
    int intensityBinFromCluster2D(int cluster) const;
    int gradientBinFromCluster2D(int cluster) const;

private slots:

    void setNewVolume(Volume *volume);

    /// Obre un diàleg per triar el color de fons.
    void chooseBackgroundColor();
    /// Obre un diàleg per carregar una funció de transferència.
    void loadTransferFunction();
    void loadColorTransferFunction();
    /// Obre un diàleg per desar una funció de transferència.
    void saveTransferFunction();
    void addRecentTransferFunction();
    void setRecentTransferFunction(const QModelIndex &index);
    void setTransferFunction(bool render = true);
    /// Renderitza amb les opcions seleccionades.
    void render();

    /// Posa els paràmetres de la càmera actuals als spinboxs corresponents.
    void getCamera();
    /// Assigna els paràmetres de la càmera d'acord amb els spinboxs corresponents.
    void setCamera();
    /// Carrega els paràmetres de la càmera des d'un fitxer.
    void loadCamera();
    /// Desa els paràmetres de la càmera a un fitxer.
    void saveCamera();
    /// Estableix el nombre de punts de vista.
    void setNumberOfViewpoints(int numberOfViewpoints);
    /// Situa la càmera al punt de vista seleccionat.
    void setViewpoint();
    /// Situa la càmera al punt de vista passat.
    void setViewpoint(const Vector3 &viewpoint);
    /// Fa un recorregut pels punts de vista triats per l'usuari.
    void tour();

    /// Comença el càlcul de les obscurances si no s'estan calculant i el cancel·la si s'estan calculant.
    void computeCancelObscurance();
    /// Fa el que cal quan s'acaba el càlcul de les obscurances correctament.
    void endComputeObscurance();
    /// Fa el que cal quan s'acaba el càlcul de les obscurances cancel·lat.
    void endCancelObscurance();
    /// Carrega les obscurances des d'un fitxer.
    void loadObscurance();
    /// Desa les obscurances a un fitxer.
    void saveObscurance();

    /// Estableix el nombre de punts de vista de l'SMI.
    void setNumberOfSmiViewpoints(int numberOfViewpoints);
    /// Calcula les SMI amb el/s punt/s de vista seleccionat/s.
    void computeSmi();
    /// Calcula les inestabilitats de les llesques.
    void computeSliceUnstabilities();
    /// Calcula les PMI amb el/s punt/s de vista seleccionat/s.
    void computePmi();
    /// Calcula les saliencies dels valors de propietat.
    void computePropertySaliencies();

    /// Calcula les mesures del grup VMI seleccionades.
    void computeSelectedVmi();
    void computeSelectedVmii();
    void generateColorTransferFunctionFromImi();
    void generateOpacityTransferFunctionFromImi();
    void generateTransferFunctionFromImi();
    void generateTransferFunctionFromIntensityClusters();
    void generateAndEvolveTransferFunctionFromIntensityClusters();
    void fineTuneGeneticTransferFunctionFromIntensityClusters();
    void optimizeByDerivativeTransferFunctionFromIntensityClusters();
    void optimizeTransferFunctionAutomaticallyForOneViewpoint(bool on);

    void tourBestViews();
    void guidedTour();
    void exploratoryTour();

    void computeVomiGradient();
    void computeVomi2Gradient();

    void loadAndRunProgram();

    void opacityVomiChecked(bool checked);
    void opacitySaliencyChecked(bool checked);
    void opacityFilteringChecked(bool checked);
    void opacityProbabilisticAmbientOcclusionChecked(bool checked);

    void setVmiOneViewpointMaximum(int maximum);
    void setVmiiOneViewpointMaximum(int maximum);

    void getFileNameToSaveTour();

    // Filtering
    void gaussianFilter();
    void boxMeanFilter();
    // Càlculs de PAO
    void probabilisticAmbientOcclusionGaussianChebychev();
    void probabilisticAmbientOcclusionBoxMeanChebychev();
    void probabilisticAmbientOcclusionGaussian();
    void probabilisticAmbientOcclusionCube();
    void probabilisticAmbientOcclusionSphere();
    void probabilisticAmbientOcclusionTangentSphereVariance();
    void probabilisticAmbientOcclusionTangentSphereCdf();
    void probabilisticAmbientOcclusionTangentSphereGaussian();
    // Per modular opacitat
    void probabilisticAmbientOcclusionGradient();
    void volumeVariance();

    void generateInnernessProportionalOpacityTransferFunction();
    void createClusterizedVolume();
    void fillWeightsEditor();

    void enableBaseVomi(bool on);
    void enableVomi(bool on);

    TransferFunction normalToClusterizedTransferFunction(const TransferFunction &normalTransferFunction) const;
    void normalToClusterizedTransferFunction();
    void clusterizedToNormalTransferFunction();
    void viewNormalVolume();
    void viewClusterizedVolume();

    void updateIntensityGradientClusteringTotal();
    void intensityGradientClustering();
    void create2DClusterizedVolume();
    TransferFunction normalToClusterizedTransferFunction2D(const TransferFunction &normalTransferFunction) const;
    void normalToClusterizedTransferFunction2D();
    void clusterizedToNormalTransferFunction2D();

    void importanceClustering();
    void normalToClusterizedTransferFunction2DImportance();
    void clusterizedToNormalTransferFunction2DImportance();

    void transferFunctionOptimizationLoadImportance();
    void transferFunctionOptimizationSaveImportance();

    void setTransferFunction2();

private:

    Experimental3DVolume *m_volume;
    Experimental3DVolume *m_normalVolume;
    Experimental3DVolume *m_clusterizedVolume;

    QList<TransferFunction> m_recentTransferFunctions;
    QStringListModel *m_recentTransferFunctionsModel;

    bool m_computingObscurance;
    ObscuranceMainThread *m_obscuranceMainThread;
    Obscurance *m_obscurance;

    QVector<float> m_viewedVolume;
    float m_HV;             // H(V)
    QVector<float> m_HVz;   // H(V|z)
    float m_HZ;             // H(Z)
    QVector<float> m_HZv;   // H(Z|v)
    float m_HZV;            // H(Z|V)
    QVector<float> m_vmi;   // I₁(v;Z)
    QVector<float> m_vmi2;  // I₂(v;Z)
    QVector<float> m_vmi3;  // I₃(v;Z)
    float m_mi;
    QVector<float> m_viewpointUnstabilities;
    QVector<float> m_vomi;  // I₁(z;V)
    float m_minimumVomi;
    float m_maximumVomi;
    QVector<float> m_vomi2; // I₂(z;V)
    float m_minimumVomi2;
    float m_maximumVomi2;
    QVector<float> m_vomi3; // I₃(z;V)
    float m_minimumVomi3;
    float m_maximumVomi3;
    QVector<float> m_viewpointVomi;     // INF
    QVector<float> m_viewpointVomi2;    // INF2
    QVector<Vector3Float> m_colorVomiPalette;
    QVector<Vector3Float> m_colorVomi;
    float m_maximumColorVomi;
    TransferFunction m_evmiOpacityTransferFunction;
    QVector<float> m_evmiOpacity;
    QVector<float> m_evmiVomi;
    QList< QPair<int, Vector3> > m_bestViews;
    QList< QPair<int, Vector3> > m_guidedTour;
    QList< QPair<int, Vector3> > m_exploratoryTour;
    QVector<float> m_voxelSaliencies;
    float m_maximumSaliency;

    QVector<float> m_viewedVolumeI;
    float m_HI;             // H(I)
    QVector<float> m_HIv;   // H(I|v)
    float m_HIV;            // H(I|V)
    QVector<float> m_vmii;
    float m_mii;
    QVector<float> m_viewpointUnstabilitiesI;
    QVector<float> m_imi;
    float m_maximumImi;
    enum ClusteringType { None, Intensity, IntensityGradient, Importance };
    ClusteringType m_clusteringType;
    QList< QList<int> > m_intensityClusters;
    QList< QList<int> > m_gradientClusters;
    QVector< QVector<unsigned short> > m_intensityGradientMap;
    int m_importantStart;
    QVector<bool> m_clusterHasData;
    TransferFunction m_normalTransferFunction;
    TransferFunction m_clusterizedTransferFunction;
    bool m_viewClusterizedVolume;
    bool m_optimizeTransferFunctionAutomaticallyForOneViewpoint;
    bool m_optimizing, m_stopOptimization, m_pendingOptimization;

    // Filtering
    QVector<float> m_spatialImportanceFunction; // ΔD = G * D − D
    float m_maximumSpatialImportanceFunction;
    QVector<float> m_probabilisticAmbientOcclusion;
    QVector<float> m_volumeVariance;

    /// Cert quan estiguem executant el l'extensió interactivament.
    bool m_interactive;

};

} // namespace udg

#endif // UDGQEXPERIMENTAL3DEXTENSION_H
