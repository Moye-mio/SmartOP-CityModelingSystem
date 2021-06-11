#include "pch.h"
#include "QTInterface.h"
#include <QtWidgets/qmdisubwindow.h>
#include <QSlider>
#include <QtWidgets/QFileDialog>
#include <QStandardItem>
#include <qobject.h>
#include <vtkRenderWindow.h>
#include <QDateTime>
#include <QColor>
#include <qlabel.h>
#include <iostream>
#include <string>
#include <QtWidgets/qmainwindow.h>
#include <QMainWindow>
#include <vtkAutoInit.h>
#include <common/ConfigCommon.h>
#include <common/ConfigInterface.h>

#include "QTDockWidgetTitleBar.h"
#include "ui_DisplayOptionsSettingDialog.h"
#include "DisplayOptionsSettingDialog.h"
#include "AutoRetouchConfig.h"
#include "SpatialClusterConfig.h"
#include "ObliquePhotographyDataInterface.h"
#include "AutoRetouchInterface.h"
#include "VisualizationInterface.h"

VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);

using namespace hiveObliquePhotography::QTInterface;

QTInterface::QTInterface(QWidget * vParent)
    : QMainWindow(vParent)
{
    ui.setupUi(this);

    __connectSignals();
    __initialResourceSpaceDockWidget();
    __initialWorkSpaceDockWidget();
    __initialMessageDockWidget();

    if (hiveConfig::hiveParseConfig("AutoRetouchConfig.xml", hiveConfig::EConfigType::XML, AutoRetouch::CAutoRetouchConfig::getInstance()); != hiveConfig::EParseResult::SUCCEED)
    	std::cout << "Failed to parse config file." << std::endl;
}

QTInterface::~QTInterface()
{
}

void QTInterface::__connectSignals()
{
    QObject::connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(onActionOpen()));
    QObject::connect(ui.actionSetting, SIGNAL(triggered()), this, SLOT(onActionSetting()));
}

void QTInterface::__initialVTKWidget()
{
    auto pViewer = static_cast<pcl::visualization::PCLVisualizer*>(Visualization::hiveGetPCLVisualizer());
    ui.VTKWidget->SetRenderWindow(pViewer->getRenderWindow());
    pViewer->setupInteractor(ui.VTKWidget->GetInteractor(), ui.VTKWidget->GetRenderWindow());
    ui.VTKWidget->update();
}

void QTInterface::__initialResourceSpaceDockWidget()
{
    m_pResourceSpaceStandardItemModels = new QStandardItemModel(ui.workSpaceTreeView);
    ui.workSpaceTreeView->setModel(m_pResourceSpaceStandardItemModels);
    m_pResourceSpaceStandardItemModels->setHorizontalHeaderLabels(QStringList() << QStringLiteral(""));

    QTDockWidgetTitleBar* dockWidgetTitleBar = new QTDockWidgetTitleBar(ui.workSpaceTreeView);
    dockWidgetTitleBar->setAttr(QColor(0, 122, 204, 255), QColor(255, 255, 255, 255), 9, "Resource Space");
    ui.resourceDockWidget->setTitleBarWidget(dockWidgetTitleBar);
}

void QTInterface::__initialWorkSpaceDockWidget()
{
    m_pResourceSpaceStandardItemModels = new QStandardItemModel(ui.workSpaceDockWidget);
    ui.resourceTreeView->setModel(m_pResourceSpaceStandardItemModels);
    m_pResourceSpaceStandardItemModels->setHorizontalHeaderLabels(QStringList() << QStringLiteral(""));

    QTDockWidgetTitleBar* dockWidgetTitleBar = new QTDockWidgetTitleBar(ui.workSpaceDockWidget);
    dockWidgetTitleBar->setAttr(QColor(0, 122, 204, 255), QColor(255, 255, 255, 255), 9, "Resource Space");
    ui.workSpaceDockWidget->setTitleBarWidget(dockWidgetTitleBar);
}

void QTInterface::__initialMessageDockWidget()
{
    QTDockWidgetTitleBar* dockWidgetTitleBar = new QTDockWidgetTitleBar(ui.messageDockWidget);
    dockWidgetTitleBar->setAttr(QColor(0, 122, 204, 255), QColor(255, 255, 255, 255), 9, "OutPut Message");
    ui.messageDockWidget->setTitleBarWidget(dockWidgetTitleBar);
}

// TODO::DockWidget复用
void QTInterface::__initialDockWidgetTitleBar()
{
    m_pDockWidgetTitleBar = new QTDockWidgetTitleBar(ui.messageDockWidget);
    m_pDockWidgetTitleBar->setAttr(QColor(0, 122, 204, 255), QColor(255, 255, 255, 255), 9, "OutPut Message");
}

// TODO::配置文件
void QTInterface::__initialSlider(const QStringList& vFilePathList)
{
    const std::string& FirstCloudFilePath = vFilePathList[0].toStdString();
    auto FileCloudFileName = QTInterface::__getFileName(FirstCloudFilePath);

    auto pSubWindow = new QMdiSubWindow(ui.VTKWidget);

    m_pPointSizeSlider = new QSlider(Qt::Horizontal);
    m_pPointSizeSlider->setMinimum(1);
    m_pPointSizeSlider->setMaximum(7);
    //m_pPointSizeSlider->setValue(m_pManager->getCloud(FirstCloudName)->PointSize);

    connect(m_pPointSizeSlider, &QSlider::valueChanged, [&]()
        {
            //m_pManager->fetchCloud(m_CurrentCloud)->PointSize = m_pPointSizeSlider->value();    //自己
        }
    );

    pSubWindow->setWidget(m_pPointSizeSlider);
    pSubWindow->resize(200, 50);
    pSubWindow->setWindowFlag(Qt::FramelessWindowHint);
    pSubWindow->show();
}

bool QTInterface::__addResourceSpaceCloudItem(const std::string& vFilePath)
{
    const auto& FileName = QTInterface::__getFileName(vFilePath);

    QStandardItem* StandardItem = new QStandardItem(QString::fromStdString(FileName));
    StandardItem->setCheckable(true);
    StandardItem->setCheckState(Qt::Checked);
    StandardItem->setEditable(false);
    m_pResourceSpaceStandardItemModels->appendRow(StandardItem);

    m_CurrentCloud = FileName;
    QTInterface::__MessageDockWidgetOutputText(QString::fromStdString(vFilePath + " is opened."));

    return true;
}

bool QTInterface::__deleteResourceSpaceCloudItem(const std::string& vFilePath)
{
    auto FileName = m_pResourceSpaceStandardItemModels->findItems(QString::fromStdString(QTInterface::__getFileName(vFilePath)));
    if (!FileName.empty())
    {
        auto row = FileName.first()->index().row();
        m_pResourceSpaceStandardItemModels->removeRow(row);
    }
    else
    {
        
    }


    return true;
}

bool QTInterface::__MessageDockWidgetOutputText(QString vString)
{
    QDateTime CurrentDateTime = QDateTime::currentDateTime();
    QString CurrentDateTimeString = CurrentDateTime.toString("[yyyy-MM-dd hh:mm:ss] ");
    ui.textBrowser->append(CurrentDateTimeString + vString);

    return true;
}


std::string QTInterface::__getFileName(const std::string& vFilePath)
{
    return vFilePath.substr(vFilePath.find_last_of('/') + 1, vFilePath.find_last_of('.') - vFilePath.find_last_of('/') - 1);
}

void QTInterface::onActionOpen()
{
    QStringList FilePathList = QFileDialog::getOpenFileNames(this, tr("Open PointCloud"), ".", tr("Open PointCloud files(*.pcd)"));
    std::vector<std::string> FileNameSet;
    bool FileOpenSuccessFlag = true;

    _ASSERT(FilePathList);
    if (FilePathList.empty())
        return;

    foreach(QString str, FilePathList)
    {
        FileNameSet.push_back(str.toStdString());
    }

    auto pCloud = hiveObliquePhotography::hiveInitPointCloudScene(FileNameSet);

    _ASSERT(pCloud);
    if (pCloud == nullptr)
        FileOpenSuccessFlag = false;

    if (FileOpenSuccessFlag)
    {
        AutoRetouch::hiveInitPointCloudScene(pCloud);
        Visualization::hiveInitVisualizer(pCloud);
        QTInterface::__initialVTKWidget();
        Visualization::hiveRefreshVisualizer(true);
        QTInterface::__initialSlider(FilePathList);

        if (FileNameSet.size() == 1)
        {
            QTInterface::__addResourceSpaceCloudItem(FileNameSet[0]);
        }
        else
        {
            QTInterface::__addResourceSpaceCloudItem("Scene 0");
        }
    }
}

void QTInterface::onActionSetting()
{
    std::shared_ptr<hiveQTInterface::CDisplayOptionsSettingDialog> pDisplayOptionsSettingDialog = std::make_shared<hiveQTInterface::CDisplayOptionsSettingDialog>(this);
    pDisplayOptionsSettingDialog->show();
    pDisplayOptionsSettingDialog->exec();

}

void QTInterface::onResourceSpaceItemDoubleClick(const QModelIndex& vIndex)
{
    const auto& CloudName = vIndex.data().toString();
    m_CurrentCloud = CloudName.toStdString();
    auto pViewer = static_cast<pcl::visualization::PCLVisualizer*>(Visualization::hiveGetPCLVisualizer());
    pViewer->removeAllPointClouds();
    //pViewer->addPointCloud(hiveObliquePhotography::);
    pViewer->resetCamera();
    pcl::visualization::Camera ResetCamera;
    pViewer->getCameraParameters(ResetCamera);
}

void QTInterface::closeEvent(QCloseEvent* vEvent)
{
    QDialog* ExitDialog = new QDialog(this);
    ExitDialog->setWindowFlag(Qt::FramelessWindowHint);
    ExitDialog->deleteLater();
    ExitDialog->resize(200, 100);
    QLabel* Label = new QLabel(ExitDialog);
    Label->setText("Are you sure?");

    Label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QPushButton* YesButton = new QPushButton(ExitDialog);
    QPushButton* NoButton = new QPushButton(ExitDialog);
    YesButton->setStyleSheet("QPushButton{color: black;min-width:75px;max-width:75px;min-height:20px;border:1px solid white;border-radius:5px;}"
        "QPushButton:hover{background-color: #2292DD;border-color: #000000;color:rgb(255, 255, 255);}"
        "QPushButton:pressed{background-color: #111111;border-color: #333333;color: blue;}");
    NoButton->setStyleSheet("QPushButton{color: black;min-width:75px;max-width:75px;min-height:20px;border:1px solid white;border-radius:5px;}"
        "QPushButton:hover{background-color: #2292DD;border-color: #000000;color:rgb(255, 255, 255);}"
        "QPushButton:pressed{background-color: #111111;border-color: #333333;color: blue;}");
    YesButton->setText("Yes");
    NoButton->setText("No");
    YesButton->setMaximumWidth(100);
    NoButton->setMaximumWidth(100);

    QObject::connect(YesButton, &QPushButton::clicked, [=]()
        {
            ExitDialog->done(1);
        });
    QObject::connect(NoButton, &QPushButton::clicked, [=]()
        {
            ExitDialog->done(0);
        });

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setSpacing(5);
    hLayout->addStretch();
    hLayout->addWidget(YesButton);
    hLayout->addWidget(NoButton);
    QVBoxLayout* v = new QVBoxLayout();
    v->addWidget(Label);
    v->addItem(hLayout);
    ExitDialog->setLayout(v);
    if (1 == ExitDialog->exec())
    {
        vEvent->accept();
    }
    else
    {
        vEvent->ignore();
    }
}
