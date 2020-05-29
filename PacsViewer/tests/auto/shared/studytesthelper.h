#ifndef STUDYTESTHELPER_H
#define STUDYTESTHELPER_H

#include <QString>

namespace udg{
    class Study;
}

namespace testing {

/// Classe que retorna Studies per utilitzar per testing
class StudyTestHelper
{
public:
    /// Torna un Study amb UID 1 amb el número de series i imatges en cada sèrie indicat.
    static udg::Study* createStudy(int numberOfSeriesToAddToStudy = 0, int numberOfImagesToAddToSeries = 0);

    /// Torna un Study amb les dades passades per paràmetre, i el número de series Indicat.
    static udg::Study* createStudyByUID(QString instanceUID, int numberOfSeriesToAddToStudy = 0, int numberOfImagesToAddToSeries = 0);

    /// Torna un Study amb les dades passades per paràmetre, i el número de series Indicat.
    static udg::Study* createStudyWithDescriptionAndSeriesModality(QString description, QString modality, int numberOfSeriesToAddToStudy = 0, int numberOfImagesToAddToSeries = 0);


    /// Elimina els objectes de l'estructura Patient/Study/Series/Image que pertany la serie
    static void cleanUp(udg::Study *study);
};

}

#endif // STUDYTESTHELPER_H
