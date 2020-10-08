/// \cond
#include <QFile>
#include <QTextStream>

#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/document.hpp>
/// \endcond
///
#include <mongodb_document.h>

/**
 * Constructor of the class.
 *
 **/
mongodb_document::mongodb_document()
{

}

/**
 * Constructor of the class.
 *
 * @param document File containing the document.
 *
 **/
mongodb_document::mongodb_document(QJsonObject document):
    _document(document)
{
}

/**
 * Constructor of the class.
 *
 * @param document File containing the document.
 *
 **/
mongodb_document::mongodb_document(BsoncxxDocView document)
{
    _document = mongodb_document::fromBsoncxx(document);
}

/**
 * Constructor of the class.
 *
 * @param document File containing the document.
 *
 **/
mongodb_document::mongodb_document(std::string document)
{
    _document = mongodb_document::fromStdString(document);
}

/**
 * Constructor of the class.
 *
 * @param document File containing the document.
 *
 **/
QJsonObject mongodb_document::getDoc()
{
    return _document;
}

/**
 * Constructor of the class.
 *
 * @param document File containing the document.
 *
 **/
mongodb_document::mongodb_document(QString document)
{
    _document = mongodb_document::fromQString(document);
}

/**
 * Constructor of the class.
 *
 * @param document File containing the document.
 *
 **/
mongodb_document::mongodb_document(QByteArray document)
{
    _document = mongodb_document::fromQByteArray(document);
}

/**
 * Constructor of the class.
 *
 * @param document File containing the document.
 *
 **/
mongodb_document::mongodb_document(QJsonDocument document)
{
    _document = mongodb_document::fromQJsonDocument(document);
}


/**
 * Get the MongoDB id of the document.
 *
 * @return id Id of the document.
 *
 **/
QString mongodb_document::getId()
{
    QJsonValue id_Val = _document.value("_id");
    QJsonObject id_Object = id_Val.toObject();
    QJsonValue oid_Val = id_Object.value("$oid");
    QString output = oid_Val.toString();

    if(output == "")
    {
        if(id_Val.toString() == "")
        {
            return "NULL";
        }
        else
        {
            return id_Val.toString();
        }

    }
    else
    {
        return output;
    }
}

/**
 * Get the MongoDB id of the document in GridFS format (For files larger than 16Mb).
 *
 * @return id Id of the document.
 *
 **/
bsoncxx::types::bson_value::value mongodb_document::getIdGridfsFormat()
{
    QString id = mongodb_document::getId();
    // Build a valid mongodb id structure
    bsoncxx::builder::stream::document doc{};
    BsoncxxDocVal id_Val  = doc << "_id" << bsoncxx::oid(id.toStdString()) << bsoncxx::builder::stream::finalize;
    BsoncxxDocView id_View = id_Val.view();

    // The open_download_stream method requires bsoncxx::types::value, therefore a conversion from bsoncxx::document::value is needed
    bsoncxx::document::element doc_element = id_View["_id"];
    bsoncxx::types::bson_value::value id_types_value{doc_element.get_value()};
    return id_types_value;
}

/**
 * Update the MongoDB id of the document.
 *
 * @param id New id for the document.
 * @return document.
 *
 **/
QJsonObject mongodb_document::updateDocumentId(QString id)
{
    QString MONGO_ID = "mongo_id_format";

    // If the given object has a different id or does not have one, replace it with the given one and then use the function addDocument(obj)
    bsoncxx::builder::stream::document doc{};
    bsoncxx::document::value id_field  = doc << "_id" << bsoncxx::oid(id.toStdString()) << bsoncxx::builder::stream::finalize;
    QJsonObject newobj_id = mongodb_document::fromBsoncxx(id_field.view());

    // Take the value of the key "_id": it is a ObjectId type
    QJsonValue newid = newobj_id.take("_id");
    // Remove previous id if there is one
    _document.remove(QString("_id"));
    // Add the given id with the correct format
    _document.insert(QString("_id"),newid);

    return _document;
}

/**
 * Convert documnet to BsoncxxDocVal.
 *
 * @return output Converted document.
 *
 **/
BsoncxxDocVal mongodb_document::toBsoncxxDocVal()
{
    std::string output_stdString = mongodb_document::toStdString();
    BsoncxxDocVal output = bsoncxx::from_json(output_stdString);
    return output;
}

/**
 * Convert documnet to BsoncxxDocView.
 *
 * @return output Converted document.
 *
 **/
BsoncxxDocView mongodb_document::toBsoncxxDocView()
{
    std::string output_stdString = mongodb_document::toStdString();
    BsoncxxDocVal output = bsoncxx::from_json(output_stdString);
    return output.view();
}

/**
 * Convert documnet to QString.
 *
 * @return output Converted document.
 *
 **/
QString mongodb_document::toQString()
{
    std::string output = mongodb_document::toStdString();
    return  QString::fromStdString(output);
}

/**
 * Convert documnet to QVariantMap.
 *
 * @return output Converted document.
 *
 **/
QVariantMap mongodb_document::toQVariantMap()
{
    return _document.toVariantMap();
}

/**
 * Convert documnet to std::string.
 *
 * @return output Converted document.
 *
 **/
std::string mongodb_document::toStdString()
{
    QByteArray output = mongodb_document::toQByteArray();
    return  output.toStdString();
}

/**
 * Convert documnet to QByteArray.
 *
 * @return output Converted document.
 *
 **/
QByteArray mongodb_document::toQByteArray()
{
    QJsonDocument output_QJsonDoc =  mongodb_document::toQJsonDocument();
    QByteArray output(output_QJsonDoc.toJson());
    return output;
}

/**
 * Convert documnet to QJsonDocument.
 *
 * @return output Converted document.
 *
 **/
QJsonDocument mongodb_document::toQJsonDocument()
{
    QJsonDocument output(_document);
    return output;
}

/**
 * Convert documnet From QJsonDocument.
 *
 * @return output Converted document.
 *
 **/
QJsonObject mongodb_document::fromQJsonDocument(QJsonDocument input)
{
    return input.object();
}

/**
 * Convert documnet From QByteArray.
 *
 * @return output Converted document.
 *
 **/
QJsonObject mongodb_document::fromQByteArray(QByteArray input)
{
    QJsonDocument output = QJsonDocument::fromJson(input);
    return mongodb_document::fromQJsonDocument(output);
}

/**
 * Convert documnet From QString.
 *
 * @return output Converted document.
 *
 **/
QJsonObject mongodb_document::fromQString(QString input)
{
    QByteArray output = input.toLocal8Bit();
    return mongodb_document::fromQByteArray(output);
}

/**
 * Convert documnet From std::string .
 *
 * @return output Converted document.
 *
 **/
QJsonObject mongodb_document::fromStdString(std::string input)
{
    QString output = QString::fromStdString(input);
    return mongodb_document::fromQString(output);
}

/**
 * Convert documnet From BsoncxxDocView.
 *
 * @return output Converted document.
 *
 **/
QJsonObject mongodb_document::fromBsoncxx(BsoncxxDocView input)
{
    std::string output = bsoncxx::to_json(input);
    return mongodb_document::fromStdString(output);
}

/**
 * Load document from a file in disk.
 *
 * @param file_path Path to the file to be loaded.
 *
 **/
void mongodb_document::loadFromDisk(QString file_path)
{
    //load the file and convert its contents into a QbyteArray
    QFile file_obj(file_path);
    if(!file_obj.open(QIODevice::ReadOnly))
    {
        //_logger->add(m_type.INFO,"In function: loadJsonFromDisk,  Failed to open: ",file_path);
        throw(1);
    }

    QTextStream file_text(&file_obj);
    QString output = file_text.readAll();
    file_obj.close();

    _document = mongodb_document::fromQString(output);
}

/**
 * Save document to a file in disk.
 *
 * @param file_path Path to the file to be saved.
 *
 **/
void mongodb_document::saveToDisk(QString file_path)
{
    std::string file_path_str=file_path.toStdString();
    if(file_path_str.find_last_of(".") != std::string::npos)
    {
        if(!(file_path_str.substr(file_path_str.find_last_of(".")+1) == "json"))
        {
            //_logger->add(m_type.INFO,"In function: saveJsonToDisk, file name had the wrong extension, changing it to .json");
            file_path_str.erase(file_path_str.find_last_of("."),file_path_str.size());
            file_path_str.append(".json");
            file_path.clear();
            file_path.append(QString::fromStdString(file_path_str));
        }
    }
    else
    {
        file_path.append(".json");
    }

    QFile jsonFile(file_path);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(mongodb_document::toQByteArray());
    jsonFile.close();
}

/**
 * Insert a Key/Value pair in the document.
 *
 * @param key
 * @param value
 *
 **/
void mongodb_document::insertKeyValuePair(QString &key, QJsonObject &value)
{
    _document.insert(key, value);
}

/**
 * Insert a Key/Value pair in the document.
 *
 * @param key
 * @param value
 *
 **/
void mongodb_document::insertKeyValuePair(QString key, QString value)
{
    _document.insert(key, value);
}


