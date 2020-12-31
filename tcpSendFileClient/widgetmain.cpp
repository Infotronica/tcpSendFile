#include "widgetmain.h"
#include "ui_widgetmain.h"

WidgetMain::WidgetMain(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetMain)
{
    ui->setupUi(this);

    connect(&tcpClienteSendFileName,SIGNAL(connected()),this,SLOT(conectado()));
    connect(&tcpClienteSendFileName,SIGNAL(disconnected()),this,SLOT(desconectado()));
    connect(&tcpClienteSendFileName,SIGNAL(readyRead()),this,SLOT(readReceivedFileName()));

    connect(&tcpClienteSendFile,SIGNAL(connected()),this,SLOT(conectado()));
    connect(&tcpClienteSendFile,SIGNAL(disconnected()),this,SLOT(desconectado()));
    connect(&tcpClienteSendFile,SIGNAL(bytesWritten(qint64)),this,SLOT(bytesWriten(qint64)));
    connect(&tcpClienteSendFile,SIGNAL(readyRead()),this,SLOT(readReceivedFile()));

    tcpClienteSendFileName.setReadBufferSize(400000);
    //ui->lineEditServer->setText("192.168.1.72");

    carpetaHome=QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0]+"/";
    carpetaDownloads=QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0]+"/";

    ui->btnSelectFile->setEnabled(false);
    ui->btnSendFile->setEnabled(false);
}

WidgetMain::~WidgetMain()
{
    delete ui;
}

QString WidgetMain::extractAbsoluteDir(QString fullName)
{
    QFileInfo fileInfo;
    QString nombre;

    fileInfo.setFile(fullName);
    nombre=fileInfo.absoluteDir().dirName();

    return nombre;
}

QString WidgetMain::extractBaseName(QString fullName)
{
    QFileInfo fileInfo;
    QString nombre;

    fileInfo.setFile(fullName);
    nombre=fileInfo.completeBaseName();

    return nombre;
}

QString WidgetMain::extractBaseExt(QString fullName)
{
    QFileInfo fileInfo;
    QString extension;

    fileInfo.setFile(fullName);
    extension=fileInfo.suffix();

    return extension;
}

QString WidgetMain::extractFileName(QString fullName)
{
    QFileInfo fileInfo;
    QString fileName;

    fileInfo.setFile(fullName);
    fileName=fileInfo.fileName();

    return fileName;
}

QString WidgetMain::extractFilePath(QString fullName)
{
    QFileInfo fileInfo;
    QString filePath;

    fileInfo.setFile(fullName);
    filePath=fileInfo.absolutePath()+"/";

    return filePath;
}

void WidgetMain::sendText(QTcpSocket *tcpCliente, QByteArray mensaje)
{
    tcpCliente->flush();
    tcpCliente->write(mensaje);
    tcpCliente->waitForBytesWritten(3000);
    tcpCliente->flush();
}

void WidgetMain::sendFile(QTcpSocket *tcpCliente, QString fileName)
{
    QByteArray qByteArray;
    QFile qFile;

    qFile.setFileName(fileName);
    qFile.open(QFile::ReadOnly);
    qByteArray=qFile.readAll();
    qFile.close();

    tcpCliente->flush();
    tcpCliente->write(qByteArray);
    tcpCliente->waitForBytesWritten();
    tcpCliente->flush();
}

void WidgetMain::conectado()
{
    ui->btnSelectFile->setEnabled(true);
    ui->btnSendFile->setEnabled(true);
}

void WidgetMain::desconectado()
{
    ui->btnSelectFile->setEnabled(false);
    ui->btnSendFile->setEnabled(false);
}

void WidgetMain::readReceivedFileName()
{
    QByteArray qByteArray;

    qByteArray=tcpClienteSendFileName.readAll();
    fileNameReceived=qByteArray;
}

void WidgetMain::readReceivedFile()
{
    QByteArray qByteArray;
    QString fullFileName;
    QFile qFile;

    qByteArray=tcpClienteSendFile.readAll();

    if (qByteArray==__SOF__)
    {
        qByteArrayReceivedFile.clear();
        qByteArrayReceivedFile.append(qByteArray);
    }
    else if (qByteArray.contains(__EOF__))
    {
        qByteArrayReceivedFile.append(qByteArray);
        qByteArrayReceivedFile.replace(__SOF__,"");
        qByteArrayReceivedFile.replace(__EOF__,"");

        fullFileName=carpetaDownloads+fileNameReceived;
        qFile.setFileName(fullFileName);
        qFile.open(QFile::WriteOnly);
        qFile.write(qByteArrayReceivedFile);
        qFile.flush();
        qFile.close();
        ui->listWidget->addItem(fileNameReceived);
    }
    else
    {
        qByteArrayReceivedFile.append(qByteArray);
    }
}

void WidgetMain::bytesWriten(qint64 cuantos)
{
    ////////
}

void WidgetMain::on_btnSelectFile_clicked()
{
    QString filtroMusic;

    filtroMusic="Todos (*.*)";
    filtroMusic+=";;Audio (*.mp3 *.mp2 *.wma *.wav)";
    filtroMusic+=";;Video(*.mp4 *.avi *.mkv *.3gp)";
    filtroMusic+=";;Documento(*.doc *.xls *.docx *.xlsx *.txt)";
    selectedFileName = QFileDialog::getOpenFileName(this, tr("Enviar archivo"), carpetaHome, filtroMusic);
    if (selectedFileName=="")
    {
        return;
    }
    baseName=extractFileName(selectedFileName);
    ui->labelSendFile->setText(baseName);
}

void WidgetMain::on_btnSendFile_clicked()
{
    sendText(&tcpClienteSendFileName, baseName.toUtf8());
    sendText(&tcpClienteSendFile, __SOF__);
    sendFile(&tcpClienteSendFile, selectedFileName);
    sendText(&tcpClienteSendFile, __EOF__);
}

void WidgetMain::on_btnConectarChat_clicked()
{
    QString remoteServer,qString;

    if (tcpClienteSendFileName.isOpen())
    {
        tcpClienteSendFileName.close();
        ui->btnConectarChat->setText("Conectar");
    }
    else
    {
        remoteServer=ui->lineEditServer->text();
        tcpClienteSendFileName.connectToHost(remoteServer,__PUERTO_FILE_NAME__);
        if (tcpClienteSendFileName.waitForConnected(3000))
        {
            ui->btnConectarChat->setText("Desconectar");
            ui->listWidget->addItem("Puerto File Name Conectado");
        }
        else
        {
            qString="Error "+tcpClienteSendFileName.errorString();
            ui->listWidget->addItem(qString);
            tcpClienteSendFileName.close();
        }
    }

    if (tcpClienteSendFile.isOpen())
    {
        tcpClienteSendFile.close();
        ui->btnConectarChat->setText("Conectar");
    }
    else
    {
        remoteServer=ui->lineEditServer->text();
        tcpClienteSendFile.connectToHost(remoteServer,__PUERTO_FILE__);
        if (tcpClienteSendFile.waitForConnected(3000))
        {
            ui->btnConectarChat->setText("Desconectar");
            ui->listWidget->addItem("Puerto File Content Conectado");
        }
        else
        {
            qString="Error "+tcpClienteSendFile.errorString();
            ui->listWidget->addItem(qString);
            tcpClienteSendFile.close();
        }
    }
}

void WidgetMain::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    QString fileName,fullName;

    fileName=item->text();
    fullName=carpetaDownloads+fileName;
    QDesktopServices::openUrl(QUrl::fromLocalFile(fullName));
}
