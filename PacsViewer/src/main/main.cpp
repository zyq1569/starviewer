/*************************************************************************************
  Copyright (C) 2014 Laboratori de Gràfics i Imatge, Universitat de Girona &
  Institut de Diagnòstic per la Imatge.
  Girona 2014. All rights reserved.
  http://starviewer.udg.edu

  This file is part of the Starviewer (Medical Imaging Software) open source project.
  It is subject to the license terms in the LICENSE file found in the top-level
  directory of this distribution and at http://starviewer.udg.edu/license. No part of
  the Starviewer (Medical Imaging Software) open source project, including this file,
  may be copied, modified, propagated, or distributed except according to the
  terms contained in the LICENSE file.
 *************************************************************************************/
#include "logging.h"
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP

#include "qapplicationmainwindow.h"

#include "statswatcher.h"
#include "extensions.h"
#include "extensionmediatorfactory.h"
#include "diagnosistests.h"
#include "syncactionsregister.h"
//全局应用程序定义
#include "starviewerapplication.h"
//全局应用程序定义
#include <djdecode.h>
#include <dcrledrg.h>
#include "../fmjpeg2k/fmjpeg2k/djdecode.h"
#include "../fmjpeg2k/fmjpeg2k/djencode.h"
#include <dcmjpls/djlsutil.h>   /* for dcmjpls typedefs */
 /* for class DJLSEncoderRegistration */
#include <dcmjpls/djdecode.h>
#include <dcmjpls/djrparam.h>   /* for class DJLSRepresentationParameter */

#include "applicationtranslationsloader.h"

#include "coresettings.h"
#include "inputoutputsettings.h"
#include "interfacesettings.h"
#include "shortcuts.h"
#include "starviewerapplicationcommandline.h"
#include "applicationcommandlineoptions.h"
#include "loggingoutputwindow.h"
#include "vtkinit.h"

#ifndef NO_CRASH_REPORTER
#include "crashhandler.h"
#endif

#include <QApplication>
#include <QLabel>
#include <QDesktopWidget>
#include <QLocale>
#include <QTextCodec>
#include <QDir>
#include <QMessageBox>
#include <QLibraryInfo>
#include <QScreen>
#include <qtsingleapplication.h>

#include <vtkNew.h>
#include <vtkOutputWindow.h>
#include <vtkOverrideInformation.h>
#include <vtkOverrideInformationCollection.h>

typedef udg::SingletonPointer<udg::StarviewerApplicationCommandLine> StarviewerSingleApplicationCommandLineSingleton;

void initializeTranslations(QApplication &app)
{
    udg::ApplicationTranslationsLoader translationsLoader(&app);
    // We indicate the corresponding premises
    QLocale defaultLocale = translationsLoader.getDefaultLocale();
    QLocale::setDefault(defaultLocale);

    translationsLoader.loadTranslation("qt_" + defaultLocale.name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    translationsLoader.loadTranslation(":/core/core_" + defaultLocale.name());
    translationsLoader.loadTranslation(":/interface/interface_" + defaultLocale.name());
    translationsLoader.loadTranslation(":/inputoutput/inputoutput_" + defaultLocale.name());
    translationsLoader.loadTranslation(":/main_" + defaultLocale.name());

    initExtensionsResources();//extensions.pri定义加载内容 自动生成extensions.h 
    INFO_LOG("Locales = " + defaultLocale.name());

    QStringList extensionsMediatorNames = udg::ExtensionMediatorFactory::instance()->getFactoryIdentifiersList();
    foreach (const QString &mediatorName, extensionsMediatorNames)
    {
        udg::ExtensionMediator *mediator = udg::ExtensionMediatorFactory::instance()->create(mediatorName);

        if (mediator)
        {
            QString translationFilePath = ":/extensions/" + mediator->getExtensionID().getID() + "/translations_" + defaultLocale.name();
            if (!translationsLoader.loadTranslation(translationFilePath))
            {
				WARN_LOG("The translator could not be loaded: " + translationFilePath);
            }
            delete mediator;
        }
        else
        {
            ERROR_LOG("Error loading mediator from: " + mediatorName);
        }
    }
}

/// Add the directories where to look for Qt plugins. Useful in windows.
void initQtPluginsDirectory()
{
#ifdef Q_OS_WIN32
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/plugins");
#endif
}

void sendToFirstStarviewerInstanceCommandLineOptions(QtSingleApplication &app)
{
    QString errorInvalidCommanLineArguments;

    if (!app.sendMessage(app.arguments().join(";"), 10000))
    {
        ERROR_LOG("The argument list could not be sent to the main instance, the primary instance does not appear to respond.");
        QMessageBox::critical(NULL, udg::ApplicationNameString, QObject::tr("%1 is already running, but is not responding. "
                              "To open %1, you must first close the existing %1 process, or restart your system.").arg(udg::ApplicationNameString));
    }
    else
    {
        INFO_LOG("The command line arguments were successfully sent to the main instance.");
    }
}

/// 20210104 error!!void QViewer::setupRenderWindow()
/// 20220907 影像服务端考虑将存储以时间段为目录存储,这样出现查询studyuid时间，后台需要先查找时间值
/// 20220909 增加参数判断，如果MHealthReport 启动，第一次窗体隐藏
///
#ifdef Q_OS_MAC
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>
#endif
int main(int argc, char *argv[])
{
	// 环境变量（最关键）
	//qputenv("QT_OPENGL", "desktop");

	// 强制桌面 OpenGL
	QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

	// 指定 OpenGL 版本
	//QSurfaceFormat fmt;
	//fmt.setRenderableType(QSurfaceFormat::OpenGL);
	//fmt.setVersion(3, 2);
	//fmt.setProfile(QSurfaceFormat::CoreProfile);
	//QSurfaceFormat::setDefaultFormat(fmt);

#ifdef Q_OS_MAC
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
#endif
    // Applying scale factor
    QVariant cfgValue = udg::Settings().getValue(udg::CoreSettings::ScaleFactor);
    bool exists;
    int scaleFactor = cfgValue.toInt(&exists);
    if (exists && scaleFactor != 1)
    {
        // Setting exists and is different than one
        QString envVar = QString::number(1 + (scaleFactor * 0.125),'f', 3);
        qputenv("QT_SCALE_FACTOR", envVar.toUtf8());
        QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    }

    // 设置 OpenGL 版本
    QSurfaceFormat format;
    format.setVersion(3, 2);  // 设置为 OpenGL 4.5
    format.setProfile(QSurfaceFormat::CoreProfile);  // 设置为核心配置
    QSurfaceFormat::setDefaultFormat(format);
    // We use QtSingleApplication instead of QtApplication, as it allows us
    // to always have a single instance of Starviewer running, if the user runs
    // a new instance of Starviewer detects this and sends the command
    // line with which the user has executed the new main instance.
    QtSingleApplication app(argc, argv);
    // ALL this initial process of "setups" should be encapsulated in a class dedicated to this purpose

    // Init log info
    udg::beginLogging();
    // We mark the start of the application in the log
    INFO_LOG("===================== BEGIN STARVIEWER =================================");
    INFO_LOG(QString("%1 Version %2 BuildID %3").arg(udg::ApplicationNameString).arg(udg::StarviewerVersionString).arg(udg::StarviewerBuildID));

    // We redirect VTK messages to the log.
    udg::LoggingOutputWindow *loggingOutputWindow = udg::LoggingOutputWindow::New();
    vtkOutputWindow::SetInstance(loggingOutputWindow);
    loggingOutputWindow->Delete();


    QPixmap splashPixmap;
    bool bloadfile;
#ifdef STARVIEWER_LITE
    bloadfile = splashPixmap.load(":/images/splash-lite.svg");
#else
    bloadfile = splashPixmap.load(":/images/splash.svg");
#endif
    if (!bloadfile)
    {
        ERROR_LOG(" splashPixmap.load fail! (:/images/splash.svg)");
    }
    // Note: We use Qt::Tool instead of Qt::SplashScreen because in Mac with the latter
    // if a message box was shown it appeared under the splash.
    QLabel splash(0, Qt::Tool|Qt::FramelessWindowHint);
    splash.setAttribute(Qt::WA_TranslucentBackground);
    splash.setPixmap(splashPixmap);
    splash.resize(splashPixmap.size());
    splash.move(QApplication::desktop()->screenGeometry().center() - splash.rect().center());

	//20240821
	bool commandLineDicomDirflag = false;
	QString CommDir;
	if (2 == argc)
	{
		QStringList commandArgumentsList = app.arguments();
		CommDir = commandArgumentsList[1];
		QDir Dcmdir(CommDir);
		if (Dcmdir.exists())
		{
			commandLineDicomDirflag = true;
		}
	}

    if (!app.isRunning())
    {
        splash.show();
    }
	else
	{
		if (commandLineDicomDirflag)
		{
			if (app.sendMessage(CommDir))
			{
				INFO_LOG("The command line arguments were successfully sent to the main instance:" + CommDir);
			}
			else
			{
				WARN_LOG("sendMessage timeout, sent to the main instance:" + CommDir);
			}
			return 0;
		}
	}
    app.setOrganizationName(udg::OrganizationNameString);
    app.setOrganizationDomain(udg::OrganizationDomainString);
    app.setApplicationName(udg::ApplicationNameString);

#ifndef Q_OS_MAC
#ifndef NO_CRASH_REPORTER
   // We initialize the crash handler in case we support it.
   // Just create the object so that it automatically auto-registers, so we mark it as unused to avoid a warning.
   CrashHandler *crashHandler = new CrashHandler();
   Q_UNUSED(crashHandler);
#endif
#endif



    // We initialize the settings
    udg::CoreSettings coreSettings;
    udg::InputOutputSettings inputoutputSettings;
    udg::InterfaceSettings interfaceSettings;
    udg::Shortcuts shortcuts;

    coreSettings.init();
    inputoutputSettings.init();
    interfaceSettings.init();
    shortcuts.init();

    initQtPluginsDirectory();
    initializeTranslations(app);

    // Registering the available sync actions
    udg::SyncActionsRegister::registerSyncActions();

    // ALL this is necessary to, among other things, be able to create thumbnails,
    // dicomdirs, etc. of compressed dicoms and treat them correctly with dcmtk
    // this is temporarily here, in the long run I will go to a setup class
    // register the JPEG and RLE decompressor codecs
    DJDecoderRegistration::registerCodecs();
    DcmRLEDecoderRegistration::registerCodecs();

	// register JPEG-LS codecs
	DJLSDecoderRegistration::registerCodecs();
	//jp2k
	FMJPEG2KDecoderRegistration::registerCodecs();

    // Following the recommendations of the Qt documentation,
    // we save the list of arguments in a variable, as this operation is expensive
    // http://doc.trolltech.com/4.7/qcoreapplication.html#arguments
    QStringList commandLineArgumentsList = app.arguments();

    QString commandLineCall = commandLineArgumentsList.join(" ");
    INFO_LOG("Started new Starviewer instance with the following command line arguments " + commandLineCall);

    if (commandLineArgumentsList.count() > 1  && !commandLineDicomDirflag)
    {
		if (commandLineArgumentsList[1] != "hide")
        {
            // We just parse the command line arguments to see if they are correct, we'll wait until everything is loaded by
            // process them, if the arguments are not correct show QMessagebox if there is another instance of Starviewer we end here.
            QString errorInvalidCommanLineArguments;
            if (!StarviewerSingleApplicationCommandLineSingleton::instance()->parse(commandLineArgumentsList, errorInvalidCommanLineArguments))
            {
                QString invalidCommandLine = QObject::tr("There were errors invoking %1 from the command line with the following call:\n\n%2")
                                             .arg(udg::ApplicationNameString).arg(commandLineCall) + "\n\n";
                invalidCommandLine += QObject::tr("Detected errors: ") + errorInvalidCommanLineArguments + "\n";
                invalidCommandLine += StarviewerSingleApplicationCommandLineSingleton::instance()->getStarviewerApplicationCommandLineOptions().getSynopsis();
                QMessageBox::warning(NULL, udg::ApplicationNameString, invalidCommandLine);

                ERROR_LOG("Invalid command line arguments, error : " + errorInvalidCommanLineArguments);

                // If there is already another instance running we give the error message and close Starviewer
                if (app.isRunning())
                {
                    return 0;
                }
            }
        }
    }

    int returnValue;
    if (app.isRunning())
    {
        // There is another instance of Starviewer running
        //starviewer已经正在运行
        WARN_LOG("Another instance of starviewer is running. Command line arguments will be sent to the main instance.");

        sendToFirstStarviewerInstanceCommandLineOptions(app);

        returnValue = 0;
    }
    else
    {
        // Main instance, no more running
        try
        {
            udg::QApplicationMainWindow *mainWin = new udg::QApplicationMainWindow;
            //We connect to receive arguments from other instances
			if (commandLineDicomDirflag)
			{
				QObject::connect(&app, SIGNAL(messageReceived(QString)), mainWin, SLOT(openCommandDirDcm(QString)));
			}
			else
			{
				QObject::connect(&app, SIGNAL(messageReceived(QString)), StarviewerSingleApplicationCommandLineSingleton::instance(), SLOT(parseAndRun(QString)));
			}

            INFO_LOG("Created main window");

            if (argc > 1 && !commandLineDicomDirflag)///20220912
            {
                mainWin->hide();
                INFO_LOG("MHealthReport.exe start!--main(int argc, char *argv[]):argc > 1  mainWin->hide()" + QString(argv[1]));
            }
            else
            {			
				if (commandLineDicomDirflag)
				{
					app.setActivationWindow(mainWin);
				}
                mainWin->show();
            }
            mainWin->checkNewVersionAndShowReleaseNotes();

            QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
            splash.close();

            // It is expected to have everything loaded to process the arguments received by command line,
            // this way by exemoke if it throws any
            // QMessageBox, already launched showing the MainWindow.
            if (commandLineArgumentsList.count() > 1 && !commandLineDicomDirflag)
            {
                QString errorInvalidCommanLineArguments;
                StarviewerSingleApplicationCommandLineSingleton::instance()->parseAndRun(commandLineArgumentsList, errorInvalidCommanLineArguments);
            }
			if (commandLineDicomDirflag)
			{
				mainWin->openCommandDirDcm(CommDir);
			}
            returnValue = app.exec();

        }
        // Handle special case when the database is newer than expected and the users prefers to quit.
        // In that case an int is thrown and catched here.
        // TODO Find a cleaner way to handle this case (this is already cleaner than the exit(0) that there was before).
        catch (int i)
        {
            returnValue = i;
        }
    }

	 
    //We mark the end of the application in the log
    INFO_LOG(QString("%1 Version %2 BuildID %3, returnValue %4").arg(udg::ApplicationNameString).arg(udg::StarviewerVersionString)
             .arg(udg::StarviewerBuildID).arg(returnValue));
    INFO_LOG("===================================================== END STARVIEWER =====================================================");

    return returnValue;
}

//xcopy D:\SDK\threadweaver-5.46.0_vc17\*.h /s D:\SDK\threadweaver-5.46.0_vc17_include
//xcopy D:\SDK\VTK-9.4.0\*.h /s D:\SDK\VTK-9.4.0VC17_include
//---fix
// error ：const LPWSTR WindowsSystemInformation::DesktopWindowManagerDLLName = L"Dwmapi.dll";
//  /Zc:strictStrings- 
//https://learn.microsoft.com/zh-cn/cpp/build/reference/zc-strictstrings-disable-string-literal-type-conversion?view=msvc-150
//-----

//InsightToolkit-5.0.1\Modules\ThirdParty\KWSys\src\KWSys
///EncodingCXX.cxx -->>std::wstring Encoding::ToWide(const std::string& str)
///std::wstring Encoding::ToWide(const std::string& str)
///{
///	std::wstring wstr;
///#  if defined(_WIN32)
///	/*
///	20240317 zyq
///	const int wlength = MultiByteToWideChar(
///	  KWSYS_ENCODING_DEFAULT_CODEPAGE, 0, str.data(), int(str.size()), NULL, 0);
///	if (wlength > 0) {
///	  wchar_t* wdata = new wchar_t[wlength];
///	  int r = MultiByteToWideChar(KWSYS_ENCODING_DEFAULT_CODEPAGE, 0, str.data(),
///								  int(str.size()), wdata, wlength);
///	  if (r > 0) {
///		wstr = std::wstring(wdata, wlength);
///	  }
///	  delete[] wdata;
///	}
///	*/
///	size_t    size = str.length();
///	const int wlength = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
///	if (wlength > 0)
///	{
///		wchar_t* wdata = new wchar_t[wlength];
///		memset(wdata, 0, (wlength + 1) * sizeof(wchar_t));
///		int r = MultiByteToWideChar(CP_ACP, 0, str.c_str(), size, (LPWSTR)wdata, wlength);
///		if (r > 0)
///		{
///			wstr = wdata;
///		}
///		delete[] wdata;
///	}
///
///#  else
///	size_t pos = 0;
///	size_t nullPos = 0;
///	do {
///		if (pos < str.size() && str.at(pos) != '\0') {
///			wstr += ToWide(str.c_str() + pos);
///		}
///		nullPos = str.find('\0', pos);
///		if (nullPos != std::string::npos) {
///			pos = nullPos + 1;
///			wstr += wchar_t('\0');
///		}
///	} while (nullPos != std::string::npos);
///#  endif
///	return wstr;
///}
//ubuntu18.04 后期的版本已经修改类似windows避免顺序
//LINUX :在静态链接（.a 文件）时通常会导致问题——链接器（ld）从左到右扫描库，只会把当前需要的符号拉进来，后续依赖的符号如果在前面已经扫描过的库里没找到，就会报 undefined reference。

//    LIBS +=  -Wl,--start-group
//for (lib, ITKLIBS) {
//    LIBS += $${ ITKLIBDIR } / lib$${ lib }$${ ITKLIBSUFFIX }.a
//}
//LIBS += -Wl, --end - group
//Linux（使用 GCC / binutils ld）和 Windows（使用 MSVC linker）在静态库链接行为上的核心区别，导致了“链接顺序重要性”的差异。
//为什么 Linux 上顺序这么重要（传统行为）GNU linker（ld，通常是 binutils 的）在处理**静态库（.a 文件）**时的规则是：链接器从左到右单向扫描命令行上列出的.o 和.a 文件。
//遇到主程序的.o（或前面的.o）时，会把里面未解析的符号（undefined symbols）记录到一个“待解析列表”。
//遇到一个静态库（.a）时，只会拉取当前待解析列表里需要的符号对应的.o 文件进最终可执行文件。
//如果某个.a 里的符号目前没人要，它就被完全忽略（不拉任何东西进来）。
//扫描完这个.a 后，不会回头再看它，即使后面又出现了需要它符号的地方。
//
//→ 结果：如果依赖关系是“后面的库需要前面的库的符号”，但前面的库已经被扫描过且当时没人要它 → 符号缺失 → undefined reference。
//这也是为什么 ITK 这种模块化严重、互相依赖的库特别容易中招：ITKIOJPEG.a 需要 ITKCommon.a 的符号，但如果 ITKCommon 排在前面，链接器早早就扫描它、
//发现当时没人要，就扔掉了。为什么 Windows（MSVC）上顺序不那么重要（甚至经常感觉“没影响”）Microsoft 的链接器（link.exe）在处理静态库（.lib 文件）时，采用更宽松、更智能的策略：它不严格要求单向扫描。
//链接器会多次扫描所有静态库（或至少记住所有.lib 里有哪些符号可用）。
//即使某个.lib 排在前面，它里面的符号不会因为当时没人引用就被永久丢弃。
//链接器会等到所有输入都处理完，再根据最终的未解析符号需求，从所有.lib 里拉取需要的部分。
//很多情况下，它甚至自动处理循环依赖或顺序问题（虽然不是 100 % 可靠，但比 GNU ld 宽容得多）。
//
//→ 所以在 MSVC 下，你经常可以把库随便乱排顺序（甚至反着排），仍然能链接成功，而不会报 unresolved external symbol（相当于 Linux 的 undefined reference）。总结对比表方面
//Linux(GNU ld / gold / bfd)
//Windows(MSVC link.exe)
//静态库扫描方式
//单向、从左到右，一次扫描
//多遍扫描，或记住所有符号可用性
//库里未立即需要的符号
//被忽略，不会拉入最终 exe
//通常保留可用，直到最终需求确认
//链接顺序重要性
//非常重要（经典坑）
//不那么重要（经常能容忍乱序）
//常见错误表现
//undefined reference
//unresolved external symbol（但较少因顺序引起）
//解决乱序的常用手段
//--start - group / --end - group 或手动精确排序
//基本不用管顺序，偶尔加 / FORCE:MULTIPLE 等
//ITK / VTK 等大库体验
//经常报错，必须调顺序或用 group
//通常直接成功（尤其 CMake + Visual Studio）
//
//额外说明：现代 linker 的变化LLVM 的 lld（Clang 默认 linker）已经部分打破传统，它对静态库的处理更接近 MSVC 的宽松风格（会记住符号，不严格单向丢弃）。
//mold（超快 linker）也类似。
//所以如果你在 Linux 上用 Clang + lld，顺序问题也会变少。
//
//但在 Ubuntu 18.04 + g++（默认用 binutils ld）这种经典环境里，还是严格遵守“被依赖的库要尽量放后面”或用 --start - group。
//简单说：Linux 的链接器更“抠门”、更“传统”、更“性能导向”（只拉真正需要的东西，避免膨胀），而 Windows 的链接器更“宽容”、更“用户友好”（自动帮你处理更多情况）。

//ubuntu18.04  20.04
//最推荐：强制程序使用 core profile 3.3 + （如果程序支持）大多数现代 OpenGL 程序可以用 core profile 运行得更好（性能更高、bug 更少）。临时测试（替换 your_program 为实际命令）：bash
////MESA_GL_VERSION_OVERRIDE = 3.3 MESA_GLSL_VERSION_OVERRIDE = 330 your_program
//
//或更高：bash
////MESA_GL_VERSION_OVERRIDE = 4.5 MESA_GLSL_VERSION_OVERRIDE = 450 your_program
//
//如果成功，说明程序能用 core profile——永久加到启动脚本或 desktop 文件中。Steam 游戏可在启动选项加：
////MESA_GL_VERSION_OVERRIDE = 4.5 %command%
