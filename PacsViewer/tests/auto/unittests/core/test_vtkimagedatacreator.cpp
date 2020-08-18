#include "autotest.h"
#include "vtkimagedatacreator.h"

#include "itkandvtkimagetesthelper.h"

#include <vtkImageData.h>

using namespace udg;
using namespace testing;

class test_VtkImageDataCreator : public QObject {
Q_OBJECT

private slots:

    void create_ShouldReturnNull();

    void create_ShouldReturnExpectedVtkImageData();

    void createVtkImageData_ShouldReturnNull();

    void createVtkImageData_ShouldReturnExpectedVtkImageData_data();
    void createVtkImageData_ShouldReturnExpectedVtkImageData();
};

Q_DECLARE_METATYPE(QSharedPointer<VtkImageDataCreator>)
Q_DECLARE_METATYPE(QVector<unsigned char>)
Q_DECLARE_METATYPE(QVector<unsigned short>)
Q_DECLARE_METATYPE(vtkSmartPointer<vtkImageData>)

void test_VtkImageDataCreator::create_ShouldReturnNull()
{
    QCOMPARE(VtkImageDataCreator().create(static_cast<int*>(nullptr)), vtkSmartPointer<vtkImageData>());
}

void test_VtkImageDataCreator::create_ShouldReturnExpectedVtkImageData()
{
    double origin[3] = { 1.0, 0.0, -1.0 };
    double spacing[3] = { 0.1, 0.1, 2.0 };
    constexpr int dimX = 256, dimY = 256, dimZ = 500;
    constexpr int nComponents = 2;
    QVector<float> data(dimX * dimY * dimZ * nComponents);

    for (int i = 0; i < data.size(); i++)
    {
        data[i] = i * 0.5f;
    }

    auto imageData = VtkImageDataCreator().setOrigin(origin).setSpacing(spacing).setDimensions({{dimX, dimY, dimZ}}).setNumberOfComponents(nComponents)
            .create(data.constData());

    auto expectedImageData = vtkSmartPointer<vtkImageData>::New();
    expectedImageData->SetOrigin(origin);
    expectedImageData->SetSpacing(spacing);
    expectedImageData->SetDimensions(dimX, dimY, dimZ);
    expectedImageData->AllocateScalars(VTK_FLOAT, nComponents);
    memcpy(expectedImageData->GetScalarPointer(), data.constData(), static_cast<size_t>(data.size()) * sizeof(float));

    bool equal;
    ItkAndVtkImageTestHelper::compareVtkImageData(imageData, expectedImageData, equal);

    QVERIFY2(equal, "compared vtkImageDatas are not equal");
}

void test_VtkImageDataCreator::createVtkImageData_ShouldReturnNull()
{
    VtkImageDataCreator imageDataCreator;
    QCOMPARE(imageDataCreator.createVtkImageData(1, 1, 1, static_cast<char*>(NULL)), vtkSmartPointer<vtkImageData>());
}

void test_VtkImageDataCreator::createVtkImageData_ShouldReturnExpectedVtkImageData_data()
{
    QTest::addColumn< QSharedPointer<VtkImageDataCreator> >("imageDataCreator");
    QTest::addColumn<int>("width");
    QTest::addColumn<int>("height");
    QTest::addColumn<int>("depth");
    QTest::addColumn< QVector<unsigned char> >("ucharData");
    QTest::addColumn< QVector<unsigned short> >("ushortData");
    QTest::addColumn< vtkSmartPointer<vtkImageData> >("expectedUcharVtkImageData");
    QTest::addColumn< vtkSmartPointer<vtkImageData> >("expectedUshortVtkImageData");

    double origin[3] = { 1.0, 2.0, 3.0 };
    double spacing[3] = { 0.5, 0.5, 2.5 };
    int width = 256;
    int height = 256;
    int depth = 100;
    int size = width * height * depth;
    QVector<unsigned char> ucharData(size);
    QVector<unsigned short> ushortData(size);
    for (int i = 0; i < size; i++)
    {
        ucharData[i] = i;
        ushortData[i] = i;
    }

    QSharedPointer<VtkImageDataCreator> imageDataCreator(new VtkImageDataCreator());
    imageDataCreator->setOrigin(origin);
    imageDataCreator->setSpacing(spacing);

    vtkSmartPointer<vtkImageData> expectedUcharVtkImageData = vtkSmartPointer<vtkImageData>::New();
    expectedUcharVtkImageData->SetOrigin(origin);
    expectedUcharVtkImageData->SetSpacing(spacing);
    expectedUcharVtkImageData->SetExtent(0, width - 1, 0, height - 1, 0, depth - 1);
    expectedUcharVtkImageData->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    memcpy(expectedUcharVtkImageData->GetScalarPointer(), ucharData.constData(), size * sizeof(unsigned char));

    vtkSmartPointer<vtkImageData> expectedUshortVtkImageData = vtkSmartPointer<vtkImageData>::New();
    expectedUshortVtkImageData->SetOrigin(origin);
    expectedUshortVtkImageData->SetSpacing(spacing);
    expectedUshortVtkImageData->SetExtent(0, width - 1, 0, height - 1, 0, depth - 1);
    expectedUshortVtkImageData->AllocateScalars(VTK_UNSIGNED_SHORT, 1);
    memcpy(expectedUshortVtkImageData->GetScalarPointer(), ushortData.constData(), size * sizeof(unsigned short));

    QTest::newRow("non-null data") << imageDataCreator << width << height << depth << ucharData << ushortData << expectedUcharVtkImageData
                                   << expectedUshortVtkImageData;
}

void test_VtkImageDataCreator::createVtkImageData_ShouldReturnExpectedVtkImageData()
{
    QFETCH(QSharedPointer<VtkImageDataCreator>, imageDataCreator);
    QFETCH(int, width);
    QFETCH(int, height);
    QFETCH(int, depth);
    QFETCH(QVector<unsigned char>, ucharData);
    QFETCH(QVector<unsigned short>, ushortData);
    QFETCH(vtkSmartPointer<vtkImageData>, expectedUcharVtkImageData);
    QFETCH(vtkSmartPointer<vtkImageData>, expectedUshortVtkImageData);

    bool equal;
    ItkAndVtkImageTestHelper::compareVtkImageData(imageDataCreator->createVtkImageData(width, height, depth, ucharData.constData()),
                                                  expectedUcharVtkImageData, equal);
    QVERIFY2(equal, "not equal for uchar");
    ItkAndVtkImageTestHelper::compareVtkImageData(imageDataCreator->createVtkImageData(width, height, depth, ushortData.constData()),
                                                  expectedUshortVtkImageData, equal);
    QVERIFY2(equal, "not equal for ushort");
}

DECLARE_TEST(test_VtkImageDataCreator)

#include "test_vtkimagedatacreator.moc"
