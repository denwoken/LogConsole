#ifndef LOGGINGENCODER_H
#define LOGGINGENCODER_H
#include "Logging.h"


namespace Logging {
namespace Encoder {

    static const QByteArray PrefixMarker = QByteArray(1, 'E');



    inline bool isLineDataEncoded(const QByteArray& lineData){
        //check encryption by first marker
        return lineData.startsWith(PrefixMarker);
    }
    inline QByteArray encodeLineData(const QByteArray& lineData){
        QByteArray encoded;
        //add first marker for encoding recognition
        encoded = PrefixMarker + lineData.toBase64(QByteArray::Base64Encoding);
        return encoded;
    }
    inline QByteArray decodeLineData(const QByteArray& lineData){
        //skip first marker
        if(lineData.size() < PrefixMarker.size()) return QByteArray();

        QByteArray encodedPart = lineData.mid(PrefixMarker.size());
        QByteArray decoded = QByteArray::fromBase64(encodedPart, QByteArray::Base64Encoding);
        return decoded;
    }



    inline bool isLineStringEncoded(const QString& lineString){
        return lineString.startsWith(PrefixMarker);
    }
    inline QString encodeLineString(const QString& in){
        return QString::fromUtf8(encodeLineData(in.toUtf8()));
    }
    inline bool decodeLineString(const QString& in, QString& out){
        if(isLineStringEncoded(in)){
            out = QString::fromUtf8(decodeLineData(in.toUtf8()));
            return true;
        }
        return false;
    }
    inline bool decodeLineStringAndSwap(QString& lineString){
        if(isLineStringEncoded(lineString)){
            lineString = QString::fromUtf8(decodeLineData(lineString.toUtf8()));
            return true;
        }
        return false;
    }

} //namespace Encoder
} //namespace Logging


#endif // LOGGINGENCODER_H
