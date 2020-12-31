#include "widgetmain.h"
#include "ui_widgetmain.h"

WidgetMain::WidgetMain(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetMain)
{
    ui->setupUi(this);

    tcpClienteSendFileName=nullptr;
    tcpClienteSendFile=nullptr;

    connect(&tcpServerSendFileName,SIGNAL(newConnection()),this,SLOT(sendFileNameConectado()));
    connect(&tcpServerSendFile,SIGNAL(newConnection()),this,SLOT(sendFileConectado()));

    carpetaHome=QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0]+"/";
    carpetaDownloads=QStandardPaths::standardLocations(QStandardPaths::DownloadLocation)[0]+"/";

    ui->btnSelectFile->setEnabled(false);
    ui->btnSendFile->setEnabled(false);

    listarDireccionesIP();
    //ui->listWidget->addItem("Solicitud Yare Lopezz 20201207_133532.png");
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

void WidgetMain::listarDireccionesIP()
{
    QList<QHostAddress> ipListAddress = QNetworkInterface::allAddresses();

    for(int i=0; i<ipListAddress.count(); i++)
    {
        if (ipListAddress[i].protocol()==QAbstractSocket::IPv4Protocol && !ipListAddress[i].isLoopback())
        {
            ui->cbxIPs->addItem(ipListAddress[i].toString());
        }
    }
    ui->cbxIPs->setCurrentIndex(0);
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

void WidgetMain::sendFileNameConectado()
{
    tcpClienteSendFileName=tcpServerSendFileName.nextPendingConnection();
    connect(tcpClienteSendFileName,SIGNAL(disconnected()),this,SLOT(clienteDesconectado()));
    connect(tcpClienteSendFileName,SIGNAL(readyRead()),this,SLOT(readReceivedFileName()));

    ui->btnSelectFile->setEnabled(true);
    ui->btnSendFile->setEnabled(true);
}

void WidgetMain::sendFileConectado()
{
    tcpClienteSendFile=tcpServerSendFile.nextPendingConnection();
    connect(tcpClienteSendFile,SIGNAL(disconnected()),this,SLOT(clienteDesconectado()));
    connect(tcpClienteSendFile,SIGNAL(readyRead()),this,SLOT(readReceivedFile()));

    ui->btnSelectFile->setEnabled(true);
    ui->btnSendFile->setEnabled(true);
}

void WidgetMain::readReceivedFileName()
{
    QByteArray qByteArray;

    qByteArray=tcpClienteSendFileName->readAll();
    fileNameReceived=qByteArray;
}

void WidgetMain::readReceivedFile()
{
    QByteArray qByteArray;
    QString fullFileName;
    QFile qFile;

    qByteArray=tcpClienteSendFile->readAll();

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

void WidgetMain::clienteDesconectado()
{
    ui->btnSelectFile->setEnabled(false);
    ui->btnSendFile->setEnabled(false);
}

void WidgetMain::bytesWriten(qint64 cuantos)
{
    ui->listWidget->addItem(QString::number(cuantos));
}

void WidgetMain::on_btnEncenderServer_clicked()
{
    QString ipServer,qString;
    bool bResFileName,bResFile;
    int i;

    if (tcpServerSendFileName.isListening())
    {
        tcpServerSendFileName.close();
        tcpServerSendFile.close();
        ui->labelServerStatus->setText("Servidor Apagado");
        ui->btnEncenderServer->setText("Encender Servidor");
        return;
    }

    i=ui->cbxIPs->currentIndex();
    if (i==-1)
    {
        return;
    }

    ipServer=ui->cbxIPs->currentText();
    bResFileName=tcpServerSendFileName.listen(QHostAddress(ipServer),__PUERTO_FILE_NAME__);
    bResFile=tcpServerSendFile.listen(QHostAddress(ipServer),__PUERTO_FILE__);
    if (bResFileName && bResFile)
    {
        qString="Servidor Activo en IP "+ipServer;
        ui->btnEncenderServer->setText("Apagar Servidor");
        ui->labelServerStatus->setText(qString);
    }
    if (!bResFileName)
    {
        qString="Error "+tcpServerSendFileName.errorString()+" __PUERTO_FILE_NAME__";
        ui->listWidget->addItem(qString);
        tcpServerSendFileName.close();
    }
    if (!bResFile)
    {
        qString="Error "+tcpServerSendFile.errorString()+" __PUERTO_FILE__";
        ui->listWidget->addItem(qString);
        tcpServerSendFile.close();
    }
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
    sendText(tcpClienteSendFileName, baseName.toUtf8());
    sendText(tcpClienteSendFile, __SOF__);
    sendFile(tcpClienteSendFile, selectedFileName);
    sendText(tcpClienteSendFile, __EOF__);
}

void WidgetMain::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    QString fileName,fullName;

    fileName=item->text();
    fullName=carpetaDownloads+fileName;
    QDesktopServices::openUrl(QUrl::fromLocalFile(fullName));
}
