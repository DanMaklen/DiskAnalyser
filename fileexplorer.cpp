#include "fileexplorer.h"
#include "ui_fileexplorer.h"
#include <QDebug>
#include <QToolButton>
#include <QGroupBox>
#include <QVBoxLayout>

FileExplorer::FileExplorer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FileExplorer)
{
    ui->setupUi(this);
    //QMainWindow::showFullScreen();
    QWidget::showMaximized();

    directoryThread = NULL;

    treeView = new QTreeView(this);
    listView = new QListView(this);
    tabWidget = new QTabWidget(this);
    toolBar = new QToolBar(this);
    QGroupBox* groupBox = new QGroupBox(this);
    QToolButton *upButton = new QToolButton(this);
    QVBoxLayout *vbox = new QVBoxLayout(this);
    toolBar->addWidget(upButton);
    upButton->setIcon(QIcon(":/folder/icons/up.png"));

    listView->setResizeMode(QListView::Adjust);


    vbox->addWidget(toolBar);
    vbox->addWidget(listView);
    groupBox->setLayout(vbox);

    tabWidget->addTab(treeView, "");
    tabWidget->setTabIcon(0, QIcon(""));
    tabWidget->addTab(groupBox, "");
    tabWidget->setTabIcon(1, QIcon(""));

    ui->treeDockWidget->setWidget(tabWidget);

    QString filePath = "/";
    dirModel = new QFileSystemModel(this);
    dirModel->setRootPath(filePath);
    treeView->setModel(dirModel);

    listView->setModel(dirModel);
    listView->setViewMode(QListView::IconMode);
    listView->setSpacing(10);
    listView->setUniformItemSizes(true);
    listView->setRootIndex(dirModel->index("/"));

    connect(listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onListItemDoubleClicked(QModelIndex)));
    connect(upButton, SIGNAL(clicked ()), this, SLOT(upButtonPressed()));
}

FileExplorer::~FileExplorer()
{
    delete ui;
}



void FileExplorer::onListItemDoubleClicked(QModelIndex index)
{
    if(dirModel->fileInfo(index).isDir()){
        if(directoryThread != NULL){
            directoryThread = new DirectoryExplorerThread();
            directoryThread->filePath = dirModel->filePath(index);
            connect(directoryThread, SIGNAL(resultReady(quint64)), this, SLOT(resultsFinished(quint64)) );
            directoryThread->start();
        }
        //qDebug() << dir_size(dirModel->filePath(index));
        listView->setRootIndex(index);
    }
    else{
        if(dirModel->fileInfo(index).isExecutable()){
            QString program = dirModel->filePath(index);
            QStringList arguments;
            QProcess *myProcess = new QProcess(this);
            myProcess->start(program, arguments);
        }
        else{
            QDesktopServices::openUrl(QUrl::fromLocalFile(dirModel->filePath(index)));
        }
    }
}

void FileExplorer::upButtonPressed()
{
    QDir dir (dirModel->filePath(listView->rootIndex()));
    dir.cdUp();

    listView->setRootIndex(dirModel->index(dir.path()));
}

void FileExplorer::resultsFinished(quint64 size)
{
    delete directoryThread;
    directoryThread = NULL;

    qDebug() << size;
}
