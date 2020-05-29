#include "autotest.h"

#include "series.h"
#include "image.h"
#include "dicomsource.h"
#include "seriestesthelper.h"
#include "pacsdevicetesthelper.h"
#include "imagetesthelper.h"

using namespace udg;
using namespace testing;

class test_Series : public QObject {
Q_OBJECT

private slots:

    /// Test que comprova que es retorni el DICOMSource correctament
    void getDICOMSource_ShouldReturnMergedPACSDeviceList_data();
    void getDICOMSource_ShouldReturnMergedPACSDeviceList();

    /// Test que comprova si la data introduida és vàlida o no
    void setDate_ShouldValidatePassedDate_data();
    void setDate_ShouldValidatePassedDate();

    /// Test que comprova si la data passada com a String és vàlida o no
    void setDate_ShouldValidatePassedDateAsString_data();
    void setDate_ShouldValidatePassedDateAsString();

    /// Test que comprova si el temps introduit és vàlid o no
    void setTime_ShouldValidatePassedTime_data();
    void setTime_ShouldValidatePassedTime();

    /// Test que comprova si el temps introduit com a String és vàlid o no
    void setTime_ShouldValidatePassedTimeAsString_data();
    void setTime_ShouldValidatePassedTimeAsString();

    /// Test que comprova si el metode retorna correctament si una image es visible o no
    void isViewable_ShouldReturnCorrectAnswer_data();
    void isViewable_ShouldReturnCorrectAnswer();

    /// Test que comprova si el metode retorna correctament si una image existeix
    void imageExists_ShouldReturnCorrectAnswer_data();
    void imageExists_ShouldReturnCorrectAnswer();

    /// Test que comprova si s'afegeix una imatge correctament
    void addImage_ShouldReturnCorrectAnswer_data();
    void addImage_ShouldReturnCorrectAnswer();

    void isCTLocalizer_ReturnsExpectedValues_data();
    void isCTLocalizer_ReturnsExpectedValues();

    void isMRSurvey_ReturnsExpectedValues_data();
    void isMRSurvey_ReturnsExpectedValues();
};

Q_DECLARE_METATYPE(DICOMSource)
Q_DECLARE_METATYPE(Series*)
Q_DECLARE_METATYPE(Image*)

void test_Series::getDICOMSource_ShouldReturnMergedPACSDeviceList_data()
{
    QTest::addColumn<Series*>("series");
    QTest::addColumn<DICOMSource>("result");

    DICOMSource DICOMSourceWithPACSIDOne;
    DICOMSourceWithPACSIDOne.addRetrievePACS(PACSDeviceTestHelper::createPACSDeviceByID("1"));

    DICOMSource DICOMSourceWithPACSIDTwo;
    DICOMSourceWithPACSIDTwo.addRetrievePACS(PACSDeviceTestHelper::createPACSDeviceByID("2"));

    DICOMSource DICOMSourceResults;
    DICOMSourceResults.addPACSDeviceFromDICOMSource(DICOMSourceWithPACSIDOne);
    DICOMSourceResults.addPACSDeviceFromDICOMSource(DICOMSourceWithPACSIDTwo);

    Series *series = SeriesTestHelper::createSeries(2);
    series->setDICOMSource(DICOMSourceWithPACSIDOne);
    series->getImageByIndex(0)->setDICOMSource(DICOMSourceWithPACSIDOne);
    series->getImageByIndex(1)->setDICOMSource(DICOMSourceWithPACSIDTwo);

    QTest::newRow("Series and Images with DICOMSource") << series  << DICOMSourceResults;
}

void test_Series::getDICOMSource_ShouldReturnMergedPACSDeviceList()
{
    QFETCH(Series*, series);
    QFETCH(DICOMSource, result);

    QCOMPARE(series->getDICOMSource() == result, true);

    SeriesTestHelper::cleanUp(series);
}

void test_Series::setDate_ShouldValidatePassedDate_data()
{
    QTest::addColumn<QDate>("date");
    QTest::addColumn<bool>("result");

    // Una data nula es una data invalida segons Qt
    QDate emptyDate;
    QDate invalidDate(1000, 24, 32);
    QDate validPastDate(2011, 1, 1);
    QDate validFutureDate(2012, 12, 24);

    QTest::newRow("set a null Date") << emptyDate << false;
    QTest::newRow("set a invalid Date") << invalidDate << false;
    QTest::newRow("set a valid past Date") << validPastDate << true;
    QTest::newRow("set a valid future Date") << validFutureDate << true;
}

void test_Series::setDate_ShouldValidatePassedDate()
{
    QFETCH(QDate, date);
    QFETCH(bool, result);
    Series series;

    QCOMPARE(series.setDate(date), result);
}

void test_Series::setDate_ShouldValidatePassedDateAsString_data()
{
    QTest::addColumn<QString>("date");
    QTest::addColumn<bool>("result");

    // Una data nula es una data invalida segons Qt
    QString emptyDate;
    QString invalidFormatDate("trolling.Date");
    QString invalidDate("20120032");
    QString validPointDate("2012.01.31");
    QString validDate("20120131");

    QTest::newRow("set a null Date") << emptyDate << false;
    QTest::newRow("set a invalid Date") << invalidFormatDate << false;
    QTest::newRow("set a invalid Date") << invalidDate << false;
    QTest::newRow("set a valid Date with points") << validPointDate << true;
    QTest::newRow("set a valid Date without points") << validDate << true;
}

void test_Series::setDate_ShouldValidatePassedDateAsString()
{
    QFETCH(QString, date);
    QFETCH(bool, result);
    Series series;

    QCOMPARE(series.setDate(date), result);
}

void test_Series::setTime_ShouldValidatePassedTime_data()
{
    QTest::addColumn<QTime>("time");
    QTest::addColumn<bool>("result");

    // Un time nul es un time invalid segons Qt
    QTime emptyTime;
    QTime invalidTime(24, 61, 61, 1000);
    QTime validPastTime(0, 0, 0, 0);
    QTime validFutureTime(12, 30, 30, 500);

    QTest::newRow("set a null Time") << emptyTime << false;
    QTest::newRow("set a invalid Time") << invalidTime << false;
    QTest::newRow("set a valid past Time") << validPastTime << true;
    QTest::newRow("set a valid future Time") << validFutureTime << true;
}

void test_Series::setTime_ShouldValidatePassedTime()
{
    QFETCH(QTime, time);
    QFETCH(bool, result);
    Series series;

    QCOMPARE(series.setTime(time), result);
}

void test_Series::setTime_ShouldValidatePassedTimeAsString_data()
{
    QTest::addColumn<QString>("time");
    QTest::addColumn<bool>("result");

    // Un time nul es un time invalid segons Qt
    QString emptyTime;
    QString invalidPointsTime("22:5:62.1000");
    QString invalidTime("220562");
    QString validPointsTime("21:10:30.509");
    QString validTime("211030");

    QTest::newRow("set a null Time") << emptyTime << false;
    QTest::newRow("set a invalid Time with :") << invalidPointsTime << false;
    QTest::newRow("set a invalid Time without :") << invalidTime << false;
    QTest::newRow("set a valid past Time with :") << validPointsTime << true;
    QTest::newRow("set a valid future Time without :") << validTime << true;
}

void test_Series::setTime_ShouldValidatePassedTimeAsString()
{
    QFETCH(QString, time);
    QFETCH(bool, result);
    Series series;

    QCOMPARE(series.setTime(time), result);
}

void test_Series::isViewable_ShouldReturnCorrectAnswer_data()
{
    QTest::addColumn<Series*>("series");
    QTest::addColumn<bool>("result");

    // PR amb imatge
    Series *fullPRSeries = SeriesTestHelper::createSeries(2);
    fullPRSeries->setModality("PR");

    // KO amb imatge
    Series *fullKOSeries = SeriesTestHelper::createSeries(2);
    fullKOSeries->setModality("KO");

    // SR amb imatge
    Series *fullSRSeries = SeriesTestHelper::createSeries(2);
    fullSRSeries->setModality("SR");    

    // VALIDA amb Imatge
    Series *fullViewableSeries = SeriesTestHelper::createSeries(2);
    fullViewableSeries->setModality("MR");

    // PR sense imatge
    Series *emptyPRSeries = new Series();
    fullPRSeries->setModality("PR");    

    // KO sense imatge
    Series *emptyKOSeries = new Series();
    fullKOSeries->setModality("KO");

    // SR sense imatge
    Series *emptySRSeries = new Series();
    fullSRSeries->setModality("SR");

    // VALIDA sense Imatge
    Series *emptyViewableSeries = new Series();
    emptyViewableSeries->setModality("MR");

    QTest::newRow("Series PR with Image") << fullPRSeries  << false;
    QTest::newRow("Series KO with Image") << fullKOSeries  << false;
    QTest::newRow("Series SR with Image") << fullSRSeries  << false;
    QTest::newRow("Series PR without Images") << emptyPRSeries  << false;
    QTest::newRow("Series KO without Images") << emptyKOSeries  << false;
    QTest::newRow("Series SR without Images") << emptySRSeries  << false;
    QTest::newRow("Series viewable with Images") << fullViewableSeries  << true;
    QTest::newRow("Series viewable without Images") << emptyViewableSeries  << false;
}

void test_Series::isViewable_ShouldReturnCorrectAnswer()
{
    QFETCH(Series*, series);
    QFETCH(bool, result);

    QCOMPARE(series->isViewable(), result);
    SeriesTestHelper::cleanUp(series);
}

void test_Series::imageExists_ShouldReturnCorrectAnswer_data()
{
    QTest::addColumn<Series*>("series");
    QTest::addColumn<QString>("identifier");
    QTest::addColumn<bool>("result");

    Series *emptySeries = SeriesTestHelper::createSeriesByUID("empty", 0);
    QTest::newRow("Correct identifier in a empty Series") << emptySeries << "0#0" << false;

    Series *oneImageSeries1 = SeriesTestHelper::createSeriesByUID("one1", 1);
    QTest::newRow("Unexisting identifier in a one Image Series") << oneImageSeries1 << "1#0" << false;

    Series *oneImageSeries2 = SeriesTestHelper::createSeriesByUID("one2", 1);
    QTest::newRow("Existing identifier in a one Image Series") << oneImageSeries2 << "0#0" << true;

    Series *manyImageSeries1 = SeriesTestHelper::createSeriesByUID("many1", 10);
    QTest::newRow("Unexisting identifier in a many images Series") << manyImageSeries1 << "0#1" << false;

    Series *manyImageSeries2 = SeriesTestHelper::createSeriesByUID("many2", 10);
    QTest::newRow("Existing identifier in a many images Series") << manyImageSeries2 << "9#0" << true;
}

void test_Series::imageExists_ShouldReturnCorrectAnswer()
{
    QFETCH(Series*, series);
    QFETCH(QString, identifier);
    QFETCH(bool, result);

    QCOMPARE(series->imageExists(identifier), result);

    SeriesTestHelper::cleanUp(series);
}

void test_Series::addImage_ShouldReturnCorrectAnswer_data()
{
    QTest::addColumn<Series*>("series");
    QTest::addColumn<Image*>("images");
    QTest::addColumn<bool>("result");

    Series *emptySeries = SeriesTestHelper::createSeries(0);
    Image *emptyImage = ImageTestHelper::createImageByUID("");

    Series *seriesWithOneImage = SeriesTestHelper::createSeries(1);
    Image *testImage = ImageTestHelper::createImageByUID("0");

    Series *anotherSeriesWithOneImage = SeriesTestHelper::createSeries(1); 
    Image *testImage2 = ImageTestHelper::createImageByUID("1");

    QTest::newRow("Adding an Empty Image") << emptySeries << emptyImage  << true;
    QTest::newRow("Adding an existing Image") << seriesWithOneImage << testImage  << false;
    QTest::newRow("Adding a new Image") << anotherSeriesWithOneImage << testImage2  << true;
}

void test_Series::addImage_ShouldReturnCorrectAnswer()
{
    QFETCH(Series*, series);
    QFETCH(Image*, images);
    QFETCH(bool, result);

    QCOMPARE(series->addImage(images), result);

    SeriesTestHelper::cleanUp(series);
}

void test_Series::isCTLocalizer_ReturnsExpectedValues_data()
{
    QTest::addColumn<Series*>("series");
    QTest::addColumn<bool>("expectedValue");

    QTest::newRow("Empty series") << SeriesTestHelper::createSeries(0) << false;
    QTest::newRow("Series with 1 image, no modality") << SeriesTestHelper::createSeries(1) << false;
    QTest::newRow("Series with 2 image, no modality") << SeriesTestHelper::createSeries(2) << false;

    QString localizerImageType = "VALUE1\\VALUE2\\LOCALIZER";
    Image *localizerImage = new Image();
    localizerImage->setImageType(localizerImageType);

    QString noLocalizerImageType = "VALUE1\\VALUE2\\VALUE3";
    Image *noLocalizerImage = new Image();
    noLocalizerImage->setImageType(noLocalizerImageType);

    QString invalidImageType = "NUMBER_OF_VALUES_IS_!=3";
    Image *imageWithInvalidImageType = new Image();
    imageWithInvalidImageType->setImageType(invalidImageType);

    Series *series1 = new Series();
    series1->addImage(localizerImage);
    series1->setModality("CT");
    QTest::newRow("CT Series with one localizer image") << series1 << true;

    Series *series2 = new Series();
    series2->addImage(localizerImage);
    series2->setModality("RF");
    QTest::newRow("Non-CT Series with one localizer image") << series2 << false;

    Series *series3 = SeriesTestHelper::createSeries(1);
    series3->addImage(localizerImage);
    series3->setModality("CT");
    QTest::newRow("CT Series with 2 images, second is localizer image, first not") << series3 << false;

    Series *series4 = new Series();
    series4->addImage(localizerImage);
    series4->addImage(noLocalizerImage);
    series4->setModality("CT");
    QTest::newRow("CT Series with 2 images, fisrt is localizer image, second not") << series4 << true;

    Series *series5 = new Series();
    series5->addImage(noLocalizerImage);
    series5->setModality("CT");
    QTest::newRow("CT Series, 1 image, no localizer") << series5 << false;

    Series *series6 = new Series();
    series6->addImage(imageWithInvalidImageType);
    series6->setModality("CT");
    QTest::newRow("CT Series, 1 image, invalid image type") << series6 << false;
}

void test_Series::isCTLocalizer_ReturnsExpectedValues()
{
    QFETCH(Series*, series);
    QFETCH(bool, expectedValue);

    QCOMPARE(series->isCTLocalizer(), expectedValue);
    
    SeriesTestHelper::cleanUp(series);
}

void test_Series::isMRSurvey_ReturnsExpectedValues_data()
{
    QTest::addColumn<Series*>("series");
    QTest::addColumn<bool>("expectedValue");

    QTest::newRow("Empty series") << new Series() << false;

    Series *series1 = new Series();
    series1->setDescription("Survey");
    QTest::newRow("Non-MR with survey description") << series1 << false;

    Series *series2 = new Series();
    series2->setDescription("Survey");
    series2->setModality("MR");
    QTest::newRow("MR with survey description") << series2 << true;

    Series *series3 = new Series();
    series3->setDescription("artywebhsSURVEYloryance");
    series3->setModality("MR");
    QTest::newRow("MR, contains survey in description inside another word") << series3 << true;

    Series *series4 = new Series();
    series4->setDescription("artywebhs\\SURVEY\\loryance");
    series4->setModality("MR");
    QTest::newRow("MR, contains survey in description separated by '\\'") << series4 << true;

    Series *series5 = new Series();
    series5->setDescription("ghsgajs\\klsjhjshk\\SURV\\EY");
    series5->setModality("MR");
    QTest::newRow("MR, not contains survey as standalone word") << series5 << false;

    Series *series6 = new Series();
    series6->setDescription("SURVEY_SAG");
    series6->setModality("MR");
    QTest::newRow("MR, contains survey + underscore + word") << series6 << true;
}

void test_Series::isMRSurvey_ReturnsExpectedValues()
{
    QFETCH(Series*, series);
    QFETCH(bool, expectedValue);

    QCOMPARE(series->isMRSurvey(), expectedValue);
    
    SeriesTestHelper::cleanUp(series);
}

DECLARE_TEST(test_Series)

#include "test_series.moc"
