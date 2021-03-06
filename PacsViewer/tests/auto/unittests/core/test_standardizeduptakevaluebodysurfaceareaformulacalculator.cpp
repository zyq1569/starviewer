#include "autotest.h"
#include "standardizeduptakevaluebodysurfaceareaformulacalculator.h"
#include "testingdecaycorrectionfactorformulacalculator.h"
#include "bodysurfaceareaformulacalculator.h"

#include "testingdicomtagreader.h"
#include "imagetesthelper.h"
#include "dicomsequenceattribute.h"
#include "dicomsequenceitem.h"
#include "dicomvalueattribute.h"
#include "mathtools.h"

#include <QString>
#include <QtCore/qmath.h>

using namespace udg;
using namespace testing;

class TestingBodySurfaceAreaFormulaCalculator : public BodySurfaceAreaFormulaCalculator {
public:
    bool m_canCompute;
    double m_computeValue;

public:
    bool canCompute()
    {
        return m_canCompute;
    }

    double compute()
    {
        return m_computeValue;
    }
};

class TestingStandardizedUptakeValueBodySurfaceAreaFormulaCalculator : public StandardizedUptakeValueBodySurfaceAreaFormulaCalculator {

public:
    void setDecayCorrectionCalculator(DecayCorrectionFactorFormulaCalculator *calculator)
    {
        m_decayCorrectionCalculator = calculator;
    }

    void setBodySurfaceAreaFormulaCalculator(BodySurfaceAreaFormulaCalculator *calculator)
    {
        m_bodySurfaceAreaCalculator = calculator;
    }

};

class test_StandardizedUptakeValueBodySurfaceAreaFormulaCalculator : public QObject {
    Q_OBJECT

private slots:
    void compute_ShouldReturnExpectedResultUsingTagReaderAsDataSource_data();
    void compute_ShouldReturnExpectedResultUsingTagReaderAsDataSource();

    void canCompute_ShouldReturnExpectedResult_data();
    void canCompute_ShouldReturnExpectedResult();
};

void test_StandardizedUptakeValueBodySurfaceAreaFormulaCalculator::compute_ShouldReturnExpectedResultUsingTagReaderAsDataSource_data()
{
    QTest::addColumn<bool>("canComputeDecayFactor");
    QTest::addColumn<double>("decayFactor");
    QTest::addColumn<double>("radionuclideTotalDose");
    QTest::addColumn<bool>("canComputeBodySurfaceArea");
    QTest::addColumn<double>("bodySurfaceArea");
    QTest::addColumn<double>("activityConcentrationValue");
    QTest::addColumn<double>("expectedResult");

    QTest::newRow("valid values") <<  true << 34.54 << 7373.22 << true << 873.83 << 32.987 << 32.987 / (7373.22 * 34.54) * 873.83;
}

void test_StandardizedUptakeValueBodySurfaceAreaFormulaCalculator::compute_ShouldReturnExpectedResultUsingTagReaderAsDataSource()
{
    QFETCH(bool, canComputeDecayFactor);
    QFETCH(double, decayFactor);
    QFETCH(double, radionuclideTotalDose);
    QFETCH(bool, canComputeBodySurfaceArea);
    QFETCH(double, bodySurfaceArea);
    QFETCH(double, activityConcentrationValue);
    QFETCH(double, expectedResult);

    TestingDICOMTagReader tagReader;
    tagReader.addTag(DICOMRadionuclideTotalDose, radionuclideTotalDose);

    DICOMSequenceAttribute *sequence = new DICOMSequenceAttribute();
    sequence->setTag(DICOMRadiopharmaceuticalInformationSequence);
    tagReader.addSequence(sequence);
    DICOMSequenceItem *item = new DICOMSequenceItem();
    sequence->addItem(item);

    DICOMValueAttribute *radionuclideTotalDoseAttribute = new DICOMValueAttribute();
    radionuclideTotalDoseAttribute->setTag(DICOMRadionuclideTotalDose);
    radionuclideTotalDoseAttribute->setValue(radionuclideTotalDose);
    item->addAttribute(radionuclideTotalDoseAttribute);

    TestingBodySurfaceAreaFormulaCalculator *bodySurfaceAreaFormulaCalculator = new TestingBodySurfaceAreaFormulaCalculator();
    bodySurfaceAreaFormulaCalculator->m_canCompute = canComputeBodySurfaceArea;
    bodySurfaceAreaFormulaCalculator->m_computeValue = bodySurfaceArea;

    TestingDecayCorrectionFactorFormulaCalculator *decayCorrectionCalculator = new TestingDecayCorrectionFactorFormulaCalculator();
    decayCorrectionCalculator->m_canCompute = canComputeDecayFactor;
    decayCorrectionCalculator->m_computeValue = decayFactor;

    TestingStandardizedUptakeValueBodySurfaceAreaFormulaCalculator formulaCalculator;
    formulaCalculator.setDataSource(&tagReader);
    formulaCalculator.setActivityConcentrationInImageUnits(activityConcentrationValue);
    formulaCalculator.setBodySurfaceAreaFormulaCalculator(bodySurfaceAreaFormulaCalculator);
    formulaCalculator.setDecayCorrectionCalculator(decayCorrectionCalculator);

    double computedValue = formulaCalculator.compute();

    if (MathTools::isNaN(computedValue))
    {
        QVERIFY2(MathTools::isNaN(computedValue) == MathTools::isNaN(expectedResult), "No both values are NaN");
    }
    else
    {
        QCOMPARE(computedValue, expectedResult);
    }

    delete sequence;

}

void test_StandardizedUptakeValueBodySurfaceAreaFormulaCalculator::canCompute_ShouldReturnExpectedResult_data()
{
    QTest::addColumn<bool>("canComputeDecayFactor");
    QTest::addColumn<double>("decayFactor");
    QTest::addColumn<double>("radionuclideTotalDose");
    QTest::addColumn<bool>("canComputeBodySurfaceArea");
    QTest::addColumn<double>("bodySurfaceArea");
    QTest::addColumn<double>("activityConcentrationValue");
    QTest::addColumn<QString>("pixelUnits");
    QTest::addColumn<bool>("expectedResult");

    double notUsedDouble;

    QTest::newRow("valid values") <<  true << 34.54 << 7373.22 << true << 873.83 << notUsedDouble << "BQML" << true;
    QTest::newRow("valid values, negative injectedDoseInBq") <<  true << 34.54 << -348.43 << true << 873.83 << notUsedDouble << "BQML" << true;
    QTest::newRow("valid values, negative decayFactor") <<  true << -0.54 << 87.3 << true << 0.9 << notUsedDouble << "BQML" << true;
    QTest::newRow("cannot compute decay factor") <<  false << notUsedDouble << 7373.22 << true << 873.83 << notUsedDouble << "BQML" << false;
    QTest::newRow("cannot compute body surface area") <<  true << 34.54 << 48.43 << false << notUsedDouble << notUsedDouble << "BQML" << false;
    QTest::newRow("invalid injectedDoseInBq") <<  true << 34.54 << 0.0 << true << 873.83 << notUsedDouble << "BQML" << false;
    QTest::newRow("invalid bodySurfaceArea") <<  true << 34.54 << 32.11 << true << 0.0 << notUsedDouble << "BQML" << false;
    QTest::newRow("invalid units") <<  true << 34.54 << 7373.22 << true << 873.83 << notUsedDouble << "diferent de BQML" << false;
    QTest::newRow("all invalid values") <<  true << 0.0 << 0.0 << true << -3.90 << notUsedDouble << "CNTS" << false;
}

void test_StandardizedUptakeValueBodySurfaceAreaFormulaCalculator::canCompute_ShouldReturnExpectedResult()
{
    QFETCH(bool, canComputeDecayFactor);
    QFETCH(double, decayFactor);
    QFETCH(double, radionuclideTotalDose);
    QFETCH(bool, canComputeBodySurfaceArea);
    QFETCH(double, bodySurfaceArea);
    QFETCH(double, activityConcentrationValue);
    QFETCH(QString, pixelUnits);
    QFETCH(bool, expectedResult);

    TestingDICOMTagReader tagReader;
    tagReader.addTag(DICOMRadionuclideTotalDose, radionuclideTotalDose);
    tagReader.addTag(DICOMUnits, pixelUnits);

    DICOMSequenceAttribute *sequence = new DICOMSequenceAttribute();
    sequence->setTag(DICOMRadiopharmaceuticalInformationSequence);
    tagReader.addSequence(sequence);
    DICOMSequenceItem *item = new DICOMSequenceItem();
    sequence->addItem(item);

    DICOMValueAttribute *radionuclideTotalDoseAttribute = new DICOMValueAttribute();
    radionuclideTotalDoseAttribute->setTag(DICOMRadionuclideTotalDose);
    radionuclideTotalDoseAttribute->setValue(radionuclideTotalDose);
    item->addAttribute(radionuclideTotalDoseAttribute);

    TestingBodySurfaceAreaFormulaCalculator *bodySurfaceAreaFormulaCalculator = new TestingBodySurfaceAreaFormulaCalculator();
    bodySurfaceAreaFormulaCalculator->m_canCompute = canComputeBodySurfaceArea;
    bodySurfaceAreaFormulaCalculator->m_computeValue = bodySurfaceArea;

    TestingDecayCorrectionFactorFormulaCalculator *decayCorrectionCalculator = new TestingDecayCorrectionFactorFormulaCalculator();
    decayCorrectionCalculator->m_canCompute = canComputeDecayFactor;
    decayCorrectionCalculator->m_computeValue = decayFactor;

    TestingStandardizedUptakeValueBodySurfaceAreaFormulaCalculator formulaCalculator;
    formulaCalculator.setDataSource(&tagReader);
    formulaCalculator.setActivityConcentrationInImageUnits(activityConcentrationValue);
    formulaCalculator.setBodySurfaceAreaFormulaCalculator(bodySurfaceAreaFormulaCalculator);
    formulaCalculator.setDecayCorrectionCalculator(decayCorrectionCalculator);

    QCOMPARE(formulaCalculator.canCompute(), expectedResult);
}


DECLARE_TEST(test_StandardizedUptakeValueBodySurfaceAreaFormulaCalculator)

#include "test_standardizeduptakevaluebodysurfaceareaformulacalculator.moc"
