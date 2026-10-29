/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
   c
  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qJMainWindow_h
#define __qJMainWindow_h

// Slicer includes
#include "qSlicerAppExport.h"
#include "qSlicerMainWindow.h"
#include "qJMainWindow.h"
#include "qSlicerMainWindow_p.h"
#include "itksys/SystemTools.hxx"
#include "QDir.h"
#include "qsettings.h"
#include "qtranslator.h"
#include "qSlicerApplication.h"
#include "qSlicerModuleFactoryManager.h"
#include "qSlicerModuleManager.h"
#include "QTextCodec.h"
#include "qscreen.h"
#include "qtimer.h"
#include "qthread.h"
#include "vtkPolyData.h"
#include "vtkMRMLROINode.h"
#include "QtPlatformHeaders/QWindowsWindowFunctions.h"
#include "QProcess.h"
struct ThreadInfo {
    QStringList info;
    double seconds;
};

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(const QString& filename, QObject* parent = nullptr)
        : QObject(parent), m_filename(filename) {}

public:
    void emit_thread_info(QStringList info1, double seconds1) {
        m_Info.info = info1;
        m_Info.seconds = seconds1;
        auto threadid = QThread::currentThreadId();
    }

public slots:
    void playback();

public:
    QString m_filename;
    ThreadInfo m_Info;
};


class qJMainWindowPrivate;
class Q_SLICER_APP_EXPORT qJMainWindow : public qSlicerMainWindow
{
	Q_OBJECT
        
private:
    int m_Page = -1;
    int m_OldPage = 1;
    int m_MaxPage = 4;
    void updateTopMenuDocker(QString name1);
    void updateMeasureDocker();
    void updateRightTopDocker(QString name1);
public:
	typedef qSlicerMainWindow Superclass;
    static int argc;
    static char** argv;
	qJMainWindow(QWidget* parent = nullptr);
	~qJMainWindow() override;
    static QString ProjectName;
    static bool IsPackage;
    static QSettings* ProjectSettings;
    Q_INVOKABLE int get_argc() {
        return argc; 
    }
    Q_INVOKABLE QString get_argv() {
        if (argv == nullptr) {
            return "";
        }
        if (*argv == nullptr) {
            return "";
        }

        QStringList stringList;
        for (int i = 0; i < argc; i++) {
            stringList << argv[i];
        }
        QString joinedString = stringList.join(",");
        return joinedString;
    }
    Q_INVOKABLE static QString GetProjectName() { return ProjectName; }
    Q_INVOKABLE static QSettings* GetProjectSettings() { return ProjectSettings; }
    static void CheckPackage() {
        if (qSlicerApplication::application()->isInstalled()) {
            qJMainWindow::IsPackage = true;
        }
        else {
            qJMainWindow::IsPackage = false;
        }

    }
     
    static void SetProjectName(QString in_name) {
        qJMainWindow::ProjectName = in_name;
       
        
        QString config_path = GetProjectIniPath();
        qDebug() << "qJMainWindow::config_path :" << config_path;
        
        //Ìí¼ÓºöÂÔÁÐ±í
        qSlicerModuleManager* moduleManager = qSlicerApplication::application()->moduleManager();
        qSlicerModuleFactoryManager* moduleFactoryManager = moduleManager->factoryManager();
        ProjectSettings = new QSettings(config_path,QSettings::IniFormat);
        ProjectSettings->setIniCodec(QTextCodec::codecForName("utf-8"));
        QString ignoreListstr = ProjectSettings->value("PAAA/IgnoreModules").toString();
        QStringList ignoreList = ignoreListstr.split(", ");
        qInfo() << "Add ignoreList:" << ignoreList;
        moduleFactoryManager->setModulesToIgnore(ignoreList);
        
        qInfo() << "Add Project Search Path:" << GetProjectBasePath();
        moduleFactoryManager->addSearchPath(GetPythonBasePath()+"/JExtension/JBase");
        moduleFactoryManager->addSearchPath(GetProjectBasePath());
    };

    Q_INVOKABLE QString slicerhome() {
        return qSlicerApplication::application()->slicerHome();
    };

    Q_INVOKABLE qMRMLCaptureToolBar* GetCaptureToolBar();

    Q_INVOKABLE bool IsPackages() {
        return IsPackage;
    };

    Q_INVOKABLE void show_full_screen(bool val) {
        if (val) {
            this->showFullScreen();
            QWindowsWindowFunctions::setHasBorderInFullScreen(windowHandle(), true);
        }
        else {
                QTimer::singleShot(0, [=] {showNormal();
                showMaximized(); });

        }
    };

    Q_INVOKABLE bool isRelease() {
#ifdef _DEBUG
        return false;
#endif // DEBUG
        return true;
    }

    static QString GetPythonBasePath() {
        QString app_root = QApplication::applicationDirPath();
        if (app_root == "") {
            throw;
            return "";
        }
        //D:\S521\S4RO\Slicer-build\bin
        std::string parent_path = itksys::SystemTools::GetParentDirectory(app_root.toStdString().c_str());
        
        if (!qJMainWindow::IsPackage) {
            //D:\S521\S4RO\Slicer-build
            parent_path = itksys::SystemTools::GetParentDirectory(parent_path);
            //D:\S521\S4RO
            parent_path = itksys::SystemTools::GetParentDirectory(parent_path);
            //D:\S521
            parent_path = itksys::SystemTools::GetParentDirectory(parent_path);
            //D:\S521/GLPyModule
            auto python_root = QString::fromStdString(parent_path) + "/GLPyModule";
            return python_root;
        }
        else {
            auto python_root = QString::fromStdString(parent_path) + "/GLPyModule";
            return python_root;
        }
    }
    Q_INVOKABLE static QString GetProjectBasePath() {
        return GetPythonBasePath() + "/Project/"+ qJMainWindow::ProjectName;
    }
    Q_INVOKABLE static QString GetProjectDBPath() {
        qDebug() << "GetProjectBasePath:" << GetProjectBasePath();
        return GetProjectBasePath()+"/Resources/db_jl.db";
    };
    Q_INVOKABLE static QString GetProjectQssPath(QString workName) {
        return GetProjectBasePath() + "/ProjectCache/" + workName + "/main.qss";
    };
    Q_INVOKABLE static QString GetProjectQMPath(QString workName) {
      return GetProjectBasePath() + "/ProjectCache/" + workName + "/app_qt_zh_CN.qm";
    };
    Q_INVOKABLE static QString GetProjectQMPathEn(QString workName) {
      return GetProjectBasePath() + "/ProjectCache/" + workName + "/app_qt_zh_EN.qm";
    };
    Q_INVOKABLE static QString GetProjectIniPath() {
        return GetProjectBasePath() + "/ProjectCache/settings.ini";
    }; 
    static QString GetProjectLoginIniPath() {
        return GetProjectBasePath() + "/Resources/config/Login.ini";
    }; 
    Q_INVOKABLE static QString GetProjectTestPath() {
        return GetProjectBasePath() + "/Test";
    };
    Q_INVOKABLE static QRect GetScreenSize() {
         QRect mRect1 = QGuiApplication::primaryScreen()->geometry();
         return mRect1;
    }

    QByteArray KEY = QByteArray::fromHex("1b5e302777346a503f633f2b443a6c52");
    void encryptFile(const QString& fileName);
    void encryptFolder(const QString& folderPath);
    void decryptFile(const QString& inputFilePath, const QString& outputFilePath);
    void decryptFolder(const QString& folderPath);
    void deleteFiles(const QDir& directory, QString sub);
	void show();
    QProcess* myProcess ;
    QProcess* extraProcess;
    Q_INVOKABLE QString GetMac();
    Q_INVOKABLE QString GetVolumeID();
    Q_INVOKABLE QWidget* GetCenterWidget();
    Q_INVOKABLE QWidget* GetCenterWidgetBorder();
    Q_INVOKABLE QDockWidget* GetDockWidget(QString direction);
    Q_INVOKABLE qSlicerModulePanel* GetModulePanel(QString direction);
    Q_INVOKABLE void ResetLayout();
    Q_INVOKABLE void rotate_to_vector(double dx, double dy, double dz, double dx1, double dy1, double dz1);
    QString m_GLFullPathFileName = "";
    bool close_flag = false;
    Q_INVOKABLE void SetPage(int i);
    Q_INVOKABLE void SetNextPage();
    Q_INVOKABLE void SetPreviewPage();
    Q_INVOKABLE int getOldPage() {
        return m_OldPage;
    };
     
    Q_INVOKABLE void ShowGifProgressBar() {
        if (myProcess) {
            if (myProcess->state() == QProcess::NotRunning) {
                auto path = GetProjectBasePath() + "/Resources/untitled3/untitled3.exe";
                myProcess->start(path);
            }
        }    
    }

    Q_INVOKABLE void ShowExtraPcocess(QString path) {
        if (extraProcess) {
            if (extraProcess->state() == QProcess::NotRunning) {
                extraProcess->start(path);
            }
        }
    }
    Q_INVOKABLE void HideExtraPcocess() {
        if (extraProcess) {
            if (extraProcess->state() == QProcess::Starting) {
                extraProcess->kill();
                extraProcess->waitForFinished();
            }
            if (extraProcess->state() == QProcess::Running) {
                extraProcess->kill();
                extraProcess->waitForFinished();
            }
        }
    }
    Q_INVOKABLE bool check_verification();
    Q_INVOKABLE void set_verification(bool b);
    Q_INVOKABLE QString qJMainWindow::GetHardDriveSerialNumber();
    Q_INVOKABLE QString qJMainWindow::GetHardDriveSerialNumberPython() {
        return m_HardDriveSerialNumber;
    };
    Q_INVOKABLE QString m_HardDriveSerialNumber = "";
    Q_INVOKABLE bool check_verification_from_file(QString str1);
    Q_INVOKABLE bool RemoveDuplicateCells(vtkPolyData* input);
public slots:
    void on_FileCloseSceneAction_triggered();
	void on_HelpKeyboardShortcutsAction_triggered();
	void on_HelpBrowseTutorialsAction_triggered();
	void on_HelpDocumentationAction_triggered();
	void on_HelpQuickStartAction_triggered();
	void on_HelpGetHelpAction_triggered();
	void on_HelpUserInterfaceAction_triggered();
	void on_HelpVisitSlicerForumAction_triggered();
	void on_HelpJoinUsOnTwitterAction_triggered();
	void on_HelpSearchFeatureRequestsAction_triggered();
	void on_HelpViewLicenseAction_triggered();
	void on_HelpHowToCiteAction_triggered();
	void on_HelpSlicerPublicationsAction_triggered();
	void on_HelpAcknowledgmentsAction_triggered();

	void on_HelpReportBugOrFeatureRequestAction_triggered();
	void on_HelpAboutSlicerAppAction_triggered();
private:
       signals:
           void keyPressed(int key, Qt::KeyboardModifiers modifiers);
           void animate_test_event(QStringList fields);
public slots:
    void process_animate_test_event();
private:
    int iViableDays = 1;
    Worker* worker;
    QTranslator* translator;
    QString VER_FOREVER = "jmainwindow_verification_forevera_1.0.1";
    QString VER_FORBID_FOREVER = "jmainwindow_verification_forbid_forever";
    QString VER_FIRST = "jmainwindow_verification_first";
    void writeRegedit(QString key, QString val);
    void playback(const QString& filename);
    QString getRegedit(QString key);
    void removeRegedit(QString key);
    
    int cal(int y, int m, int d);
protected:
	qJMainWindow(qJMainWindowPrivate* pimpl, QWidget* parent);
    int TitleWidgetGeometry[4] = { 0,0,1920,42 };
    int MeasureWidgetGeometry[4] = { 0,0,1441 - 11,34+56 };
    int FilterCenterWidgetSize[2] = { 1208,885-56 };
    int FilterBottomSize[2] = { 1208,120 };
    int FilterRightSize[2] = { 479,1038 };
    int FilterMiddleRightSize[2] = { 233,974 };

    int OutputRightSize[2] = { 488,1000 };
    int NavigationBarSize[2] = { 1920,40 };

    void closeEvent(QCloseEvent* event) override;
private:
	Q_DECLARE_PRIVATE(qJMainWindow);
	Q_DISABLE_COPY(qJMainWindow);
};

#endif
