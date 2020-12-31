#ifndef WIDGETMAIN_H
#define WIDGETMAIN_H

#include <QWidget>
#include <QTcpSocket>
#include <QMessageBox>
#include <QImageReader>
#include <QImage>
#include <QBuffer>
#include <QPicture>
#include <QDataStream>
#include <QTcpServer>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QFileDialog>
#include <QStandardPaths>
#include <QListWidgetItem>
#include <QDesktopServices>
#include "../commonparams.h"

namespace Ui {
class WidgetMain;
}

class WidgetMain : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetMain(QWidget *parent = nullptr);
    ~WidgetMain();

    QString extractAbsoluteDir(QString fullName);
    QString extractBaseName(QString fullName);
    QString extractBaseExt(QString fullName);
    QString extractFileName(QString fullName);
    QString extractFilePath(QString fullName);
    void sendText(QTcpSocket *tcpCliente, QByteArray mensaje);
    void sendFile(QTcpSocket *tcpCliente, QString fileName);

    QTcpSocket tcpClienteSendFileName;
    QTcpSocket tcpClienteSendFile;
    QByteArray qByteArrayReceivedFile,qByteArrayReceivedFileName;
    QString carpetaHome,carpetaDownloads,selectedFileName,baseName,fileNameReceived;

public slots:
    void conectado();
    void desconectado();
    void readReceivedFileName();
    void readReceivedFile();
    void bytesWriten(qint64 cuantos);

private slots:
    void on_btnSelectFile_clicked();

    void on_btnSendFile_clicked();

    void on_btnConectarChat_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::WidgetMain *ui;
};

#endif // WIDGETMAIN_H
