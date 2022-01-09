#include "AudioSpeaker.hpp"

#include "g0/Config.hpp"

#include <QDebug>
#include <QFile>
#include <QtEndian>

using namespace gtmy;
//const int BufferSize = 4096;

AudioInfo::AudioInfo(const QAudioFormat &format, QObject *parent)
    :   QIODevice(parent)
    ,   _format(format)
    ,   _maxAmplitude(0)
    ,   _level(0.0)

{
    switch (_format.sampleSize()) {
    case 8:
        switch (_format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            _maxAmplitude = 255;
            break;
        case QAudioFormat::SignedInt:
            _maxAmplitude = 127;
            break;
        default:
            break;
        }
        break;
    case 16:
        switch (_format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            _maxAmplitude = 65535;
            break;
        case QAudioFormat::SignedInt:
            _maxAmplitude = 32767;
            break;
        default:
            break;
        }
        break;

    case 32:
        switch (_format.sampleType()) {
        case QAudioFormat::UnSignedInt:
            _maxAmplitude = 0xffffffff;
            break;
        case QAudioFormat::SignedInt:
            _maxAmplitude = 0x7fffffff;
            break;
        case QAudioFormat::Float:
            _maxAmplitude = 0x7fffffff; // Kind of
        default:
            break;
        }
        break;

    default:
        break;
    }
}

AudioInfo::~AudioInfo()
{
}

void AudioInfo::start()
{ open(QIODevice::WriteOnly);}

void AudioInfo::stop()
{close();}

qint64 AudioInfo::readData(char *data, qint64 maxlen)
{   Q_UNUSED(data)
    Q_UNUSED(maxlen)

    //used for output data stream

    return 0;
}



qint64 AudioInfo::writeData(const char *data, qint64 len)
{

    _collector += QByteArray(data,len);
    int border = 16000*30; //each 60 seconds in smallest
    if (_collector.size() > border*10) //format*bitrate*minute
    {
        ///QByteArray compress = qCompress(collector,7);
        QString defaultRecFile = QString(AConfig::getInst().testsLocation.c_str()) + QString("record.temp");
        QFile f; f.setFileName(defaultRecFile);
        ///int compressedSize = compress.size();
        if (f.open(QIODevice::Append))
        {
            f.write(_collector);
            f.flush();
            f.close();
        }
        else
            qDebug() << "Open file for raw record error;";
        _collector.clear();
    }

    int fullLen = _collector.size();
    int cutLen = len;
    qDebug() << "Wroten audio data "<<cutLen<<"; in bufer "<<fullLen;
    return len;
}

void AudioInfo::setLevel(qreal newLevel)
{
    _level = newLevel;
}
/// AUDIO INPUT FINISHED
// NOW OUTPUT

AudioSpeaker::AudioSpeaker([[maybe_unused]]const QAudioFormat &format,
                     QObject *parent)
    :   QIODevice(parent)
    ,   _pos(0)
{
}

AudioSpeaker::~AudioSpeaker()
{
}

void AudioSpeaker::start()
{
    open(QIODevice::ReadOnly);
}

void AudioSpeaker::stop()
{
    _pos = 0;
    close();
}

void AudioSpeaker::generateData(const QAudioFormat &format, qint64 durationUs, [[maybe_unused]] int sampleRate)
{
    const int channelBytes = format.sampleSize() / 8;
    const int sampleBytes = format.channelCount() * channelBytes;

    qint64 length = (format.sampleRate() * format.channelCount() * (format.sampleSize() / 8))
                        * durationUs / 100000;

    Q_ASSERT(length % sampleBytes == 0);
    Q_UNUSED(sampleBytes); // suppress warning in release builds

    _buffer.resize(length);
    unsigned char *ptr = reinterpret_cast<unsigned char *>(_buffer.data());
    int sampleIndex = 0;

    while (length) {
        const qreal x = 0; //qSin(2 * M_PI * sampleRate * qreal(sampleIndex % format.sampleRate()) / format.sampleRate());
        for (int i=0; i<format.channelCount(); ++i) {
            if (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::UnSignedInt) {
                const quint8 value = static_cast<quint8>((1.0 + x) / 2 * 255);
                *reinterpret_cast<quint8*>(ptr) = value;
            } else if (format.sampleSize() == 8 && format.sampleType() == QAudioFormat::SignedInt) {
                const qint8 value = static_cast<qint8>(x * 127);
                *reinterpret_cast<quint8*>(ptr) = value;
            } else if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::UnSignedInt) {
                quint16 value = static_cast<quint16>((1.0 + x) / 2 * 65535);
                if (format.byteOrder() == QAudioFormat::LittleEndian)
                    qToLittleEndian<quint16>(value, ptr);
                else
                    qToBigEndian<quint16>(value, ptr);
            } else if (format.sampleSize() == 16 && format.sampleType() == QAudioFormat::SignedInt) {
                qint16 value = static_cast<qint16>(x * 32767);
                if (format.byteOrder() == QAudioFormat::LittleEndian)
                    qToLittleEndian<qint16>(value, ptr);
                else
                    qToBigEndian<qint16>(value, ptr);
            }

            ptr += channelBytes;
            length -= channelBytes;
        }
        ++sampleIndex;
    }
}

qint64 AudioSpeaker::readData(char *data, qint64 len)
{
    qint64 total = 0;
    if (!_buffer.isEmpty()) {
        while (len - total > 0) {
            const qint64 chunk = qMin((_buffer.size() - _pos), len - total);
            memcpy(data + total, _buffer.constData() + _pos, chunk);
            _pos = (_pos + chunk) % _buffer.size();
            total += chunk;
        }
    }
    return total;
}

qint64 AudioSpeaker::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

qint64 AudioSpeaker::bytesAvailable() const
{
    return _buffer.size() + QIODevice::bytesAvailable();
}


void AudioSpeaker::setAudioBufer(QByteArray &aStream)
{
    _buffer.clear();
    _buffer += aStream;
}
