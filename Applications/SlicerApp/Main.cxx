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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerApplicationHelper.h"
#include "qSlicerStyle.h"
#include "vtkSlicerVersionConfigure.h" // For Slicer_VERSION_FULL

// SlicerApp includes
#include "qSlicerAppMainWindow.h"
#include "qJMainWindow.h"
#include <windows.h>
QByteArray KEY = QByteArray::fromHex("1b5e302777346a503f633f2b443a6c52");

static void setFileHidden(const QString& filePath)
{
#ifdef Q_OS_WIN
  const wchar_t* wPath = reinterpret_cast<const wchar_t*>(filePath.utf16());
  DWORD attr = GetFileAttributesW(wPath);
  if (attr != INVALID_FILE_ATTRIBUTES)
  {
    SetFileAttributesW(wPath, attr | FILE_ATTRIBUTE_HIDDEN);
  }
#endif
}
void decryptFile(const QString& inputFilePath, const QString& outputFilePath) {
  QFile inputFile(inputFilePath);
  if (inputFile.open(QIODevice::ReadOnly)) {
    QByteArray inputData = inputFile.readAll();
    inputFile.close();

    QByteArray outputData(inputData.size(), 0);
    for (int i = 0; i < inputData.size(); i++) {
      outputData[i] = inputData[i] ^ KEY[i % KEY.size()];
    }

    QFile output(outputFilePath);
    if (output.open(QIODevice::WriteOnly)) {
      output.write(outputData);
      output.close();
    }
  }
  //setFileHidden(outputFilePath);
}

void decryptFolder(const QString& folderPath)
{
  QDir folder(folderPath);
  QFileInfoList fileList = folder.entryInfoList(QDir::Files);
  for (int i = 0; i < fileList.size(); ++i) {
    const QFileInfo& fileInfo = fileList.at(i);
    if (fileInfo.suffix() == "yp") {
      QString inputFilePath = fileInfo.absoluteFilePath();
      QString outputFilePath = fileInfo.absoluteFilePath().left(fileInfo.absoluteFilePath().length() - 3) + ".py";
      decryptFile(inputFilePath, outputFilePath);
    }
  }

  QFileInfoList folderList = folder.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
  for (int i = 0; i < folderList.size(); ++i) {
    const QFileInfo& folderInfo = folderList.at(i);
    decryptFolder(folderInfo.absoluteFilePath());
  }
}

void decryptFolderSTL(const QString& folderPath)
{
  QDir folder(folderPath);
  QFileInfoList fileList = folder.entryInfoList(QDir::Files);
  for (int i = 0; i < fileList.size(); ++i) {
    const QFileInfo& fileInfo = fileList.at(i);
    if (fileInfo.suffix() == "lts") {
      QString inputFilePath = fileInfo.absoluteFilePath();
      QString outputFilePath = fileInfo.absoluteFilePath().left(fileInfo.absoluteFilePath().length() - 3) + ".stl";
      decryptFile(inputFilePath, outputFilePath);
    }
  }

  QFileInfoList folderList = folder.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
  for (int i = 0; i < folderList.size(); ++i) {
    const QFileInfo& folderInfo = folderList.at(i);
    decryptFolder(folderInfo.absoluteFilePath());
  }
}
namespace
{

//----------------------------------------------------------------------------
int SlicerAppMain(int argc, char* argv[])
{
  typedef qJMainWindow SlicerMainWindowType;
  typedef qSlicerStyle SlicerAppStyle;
  QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
  qSlicerApplicationHelper::preInitializeApplication(argv[0], new SlicerAppStyle);

  qSlicerApplication app(argc, argv);
  if (app.returnCode() != -1)
  {
    return app.returnCode();
  }
  HWND consoleWindow = GetConsoleWindow();
  if (consoleWindow)
    ShowWindow(consoleWindow, SW_HIDE);
  QScopedPointer<SlicerMainWindowType> window;
  QScopedPointer<QSplashScreen> splashScreen;

  qJMainWindow::CheckPackage();
  auto path = qJMainWindow::GetPythonBasePath();
  auto gloabl_config = path + "/JGlobal/config.ini";
  qInfo() << "gloabl_config:" << gloabl_config;
  QSettings set(gloabl_config, QSettings::IniFormat);
  QString ProjectName = set.value("INIT/project").toString();
  qJMainWindow::SetProjectName(ProjectName);

  //function elapsed decryptFolder  "0" ms
  qJMainWindow* mw = dynamic_cast<qJMainWindow*>(qSlicerApplication::application()->mainWindow());
  decryptFolder(qJMainWindow::GetPythonBasePath());
  if (qJMainWindow::IsPackage)
  {    
    // decryptFolder(qJMainWindow::GetPythonBasePath());
  }

  app.setURIArgumentHandlingEnabled(true);

  int exitCode = qSlicerApplicationHelper::postInitializeApplication<SlicerMainWindowType>(
        app, splashScreen, window);
  if (exitCode != 0)
  {
    return exitCode;
  }

  if (!window.isNull())
  {

  }

  return app.exec();
}

} // end of anonymous namespace

#include "qSlicerApplicationMainWrapper.cxx"
