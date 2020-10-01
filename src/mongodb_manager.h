#ifndef MONGODB_COLLECTION_H
#define MONGODB_COLLECTION_H

/// \cond
#include <QVariantMap>
#include <QByteArray>
#include <QString>

#ifndef Q_MOC_RUN
    #include <mongocxx/client.hpp>
    #include <mongocxx/database.hpp>
    #include <mongocxx/exception/server_error_code.hpp>
    #include <mongocxx/instance.hpp>
#endif

#include <string>
#include <vector>
/// \endcond

#include <mongodb_structures.h>
#include <mongodb_logger.h>
#include <mongodb_document.h>

/**
 * @brief Backbone class to manage connection and acces to MongoDB.
 */

class mongodb_manager{

public:

    mongodb_manager(QString database_MongoDB_name, QString collection_MongoDB_name);
    mongodb_manager();

    // Connection management:
    void configureConnection(QString user, QString password, QString database, QString port, QString host);
    void configureConnection(std::string user, std::string password, std::string database, std::string port, std::string host);
    void connectToCollection(QString database_MongoDB_name, QString collection_MongoDB_name);
    void connectToCollection(std::string database_MongoDB_name, std::string collection_MongoDB_name);
    void connectToDatabase(QString database_MongoDB_name);
    void connectGridFS(QString database_MongoDB_name);
    QVariantMap loadCredentialsFile(QString file_path);

    // Document management:
    mongodb_document getDocument(QString id);
    mongodb_document getDocumentGridFS(QString file_id);
    void getDocumentList(QStringList *id_list, std::vector<mongodb_document> *document_list);
    bool exportDocument(QString id, QString file_name);
    void importDocument(QString file_path);
    QString addDocument(mongodb_document document, QString id);
    QString addDocument(mongodb_document document);
    QString addDocumentGridFS(QString document, std::string file_name);
    bool deleteDocument(QString id);

    // Collection management:
    void getCollectionList(QString database, QStringList *collection_list);
    bool addCollection(QString database, QString collection);
    bool deleteCollection(QString database, QString collection);
    void downloadCollection(QString file_name);
    bool verifyCollection(QString database, QString collection);

    // Database management:
    virtual void getDatabaseList(QStringList *database_list);
    virtual int  getNumberOfDatabases();
    virtual bool addDatabase(QString database);
    virtual bool deleteDatabase(QString database);
    virtual bool verifyDatabase(QString database);

    // Users management:
    virtual void getUsersList(QStringList *user_list);
    virtual int getNumberOfUsers();
    QString getAdminUser();
    virtual bool addUser(QString username, QString password);
    virtual bool deleteUser(QString username);
    virtual bool verifyUser(QString user);

    // User's roles management:
    void getUserRolesList(QString username, QStringList *user_roles_list);
    QString getUserRole(QString user,QString database);
    void getRolesTable(std::vector<QStringList> *roles_table);
    void saveUsersAndRolesTable();
    virtual bool revokeRoleFromUser(QString username, QString database, QString role);
    virtual bool grantRoleToUser(QString username, QString database, QString role);
    void clearDatabaseRoles(QString database);
    bool verifyRole(QString role);

    // Logger:
    virtual void setCustomLogger(mongodb_logger *custom_logger);
    void printlog();

    // Utilities:
    bsoncxx::document::value createTemplate(QString option, QString field1 = QString(""), QString field2 = QString(""), QString field3 = QString(""));


private:
    // Connection variables:
    mongocxx::client *_conn;
    mongocxx::database _database_MDB;
    mongocxx::collection _collection_MDB;
    mongocxx::gridfs::bucket _gridfs_bucket;

    // General information:
    QString _user;
    QString _current_database_name;
    QString _current_collection_name;
    QString ADMIN_DB = "admin";
    QString ADMIN_DB_EXTEND = "admin.";
    QString USERS_COLLECTION = "system.users";

    // Utilities:
    mongodb_actions _actions;
    mongodb_message_types _m_type;
    mongodb_logger *_logger;
    bool _logger_custom = false;

};

#endif // MONGODB_COLLECTION_H
