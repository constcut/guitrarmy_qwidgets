#ifndef AUDIOSPEAKER_H
#define AUDIOSPEAKER_H

#include <QIODevice>
#include <QAudioFormat>


class AudioInfo : public QIODevice
{
    Q_OBJECT

public:
    AudioInfo(const QAudioFormat &format, QObject *parent);
    ~AudioInfo();

    void start();
    void stop();

    qreal level() const { return _level; }

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

    QByteArray& collector() { return _collector; }

    void setLevel(qreal newLevel);

private:
    const QAudioFormat _format;
    quint32 _maxAmplitude;
    qreal _level; // 0.0 <= m_level <= 1.0
    QByteArray _collector;

signals:
    void update();
};


class AudioSpeaker : public QIODevice
{
    Q_OBJECT

public:
    AudioSpeaker(const QAudioFormat &format, QObject *parent=0);
    ~AudioSpeaker();

    void start();
    void stop();

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;

    void setAudioBufer(QByteArray &aStream);

private:
    void generateData(const QAudioFormat &format, qint64 durationUs, int sampleRate);

private:
    qint64 _pos;
    QByteArray _buffer;
};



#endif // AUDIOSPEAKER_H
