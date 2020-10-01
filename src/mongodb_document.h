#ifndef MONGODB_DOCUMENT_H
#define MONGODB_DOCUMENT_H

/// \cond
#include <QJsonObject>
#include <QByteArray>
#include <QJsonDocument>
#include <QString>
#include <QtCore/qiterator.h>

#include <string>

#ifndef Q_MOC_RUN
    #include <mongocxx/options/insert.hpp>
    #include <bsoncxx/document/value.hpp>
    #include <bsoncxx/types/value.hpp>
#endif
/// \endcond

typedef bsoncxx::document::view BsoncxxDocView;
typedef bsoncxx::document::value BsoncxxDocVal;

/**
 * @brief type used to store and operate with Json files in MongoDB.
 */

class mongodb_document
{
private:
    QJsonObject _document;
public:
    mongodb_document();
    mongodb_document(QJsonObject document);
    mongodb_document(QByteArray document);
    mongodb_document(QJsonDocument  document);
    mongodb_document(QString  document);
    mongodb_document(std::string  document);
    mongodb_document(BsoncxxDocView document);

    QJsonObject getDoc();
    QString getId();
    bsoncxx::types::value getIdGridfsFormat();

    QJsonObject updateDocumentId(QString id);
    void insertKeyValuePair(QString &key, QJsonObject &value);
    void insertKeyValuePair(QString key, QString value);

    QJsonDocument  toQJsonDocument();
    QByteArray     toQByteArray();
    std::string    toStdString();
    BsoncxxDocVal  toBsoncxxDocVal();
    BsoncxxDocView toBsoncxxDocView();
    QString        toQString();
    QVariantMap    toQVariantMap();

    QJsonObject fromQJsonDocument(QJsonDocument input);
    QJsonObject fromQByteArray(QByteArray input);
    QJsonObject fromQString(QString input);
    QJsonObject fromStdString(std::string input);
    QJsonObject fromBsoncxx(BsoncxxDocView input);

    void loadFromDisk(QString file_path);
    void saveToDisk(QString file_path);

};

#endif // MONGODB_DOCUMENT_H
