#ifndef QTHELPER_H
#define QTHELPER_H


#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QUrl>

#include <iostream>

#include <QByteArray>

QByteArray alphaCut(QByteArray &src);

class HttpFileLoader: public QObject
{
    Q_OBJECT
private:
    QNetworkAccessManager *accessManager;
    QNetworkReply *lastSend;

public:
    HttpFileLoader(QObject *p = 0):
        QObject(p)
    {accessManager = new QNetworkAccessManager(this);}

    ~HttpFileLoader() {

        std::cout <<std::endl << "http tool destroyed"<<std::endl;
    }

    void request(const QUrl &url)
    {
        lastSend = accessManager->get(QNetworkRequest(url));
        connect(lastSend, SIGNAL(finished()), SLOT(fileDownloaded()));
    }
public slots:
    void fileDownloaded()
    {
        QByteArray allFile = lastSend->readAll();
        std::cout << "Downloaded file size is : "<<allFile.size()<<std::endl ;
        lastSend->deleteLater();
    }
};





void notifyAndroid();


#endif // QTHELPER_H
