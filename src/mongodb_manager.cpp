/// \cond
#include <QDebug>
#include <QJsonArray>

#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/exception/query_exception.hpp>

#include <fstream>
/// \endcond

#include <mongodb_manager.h>

/**
 * Constructor of the class.
 *
 * @param database Name of the database to which the connection has to be established.
 * @param collection Name of the collection to which the connection has to be established.
 *
 **/

mongodb_manager::mongodb_manager(QString database, QString collection)
{
    _logger = new mongodb_logger;

    // Initialize connection with MongoDB
    _collection_MDB = (*_conn)[database.toStdString()][collection.toStdString()];
    _database_MDB = (*_conn)[database.toStdString()];
}

/**
 * Constructor of the class.
 *
 **/

mongodb_manager::mongodb_manager()
{
    _logger = new mongodb_logger;
}

/**
 * Establish a GidFS connection with an specific MongoDB collection.
 *
 * @param database Name of the database to which the connection has to be established.
 *
 */

void mongodb_manager::connectGridFS(QString database)
{
    _gridfs_bucket = (*_conn)[database.toStdString()].gridfs_bucket();
}

/**
 * Establish connection with an specific MongoDB database.
 *
 * @param database Name of the database to which the connection has to be established.
 *
 */

void mongodb_manager::connectToDatabase(QString database)
{
    // Initialize connection with MongoDB
    try
    {
        _database_MDB = (*_conn)[database.toStdString()];
        // Save names of the collection and database that we want to connect:
        _current_database_name = database;

    }
    catch(const mongocxx::query_exception& e)
    {
        if (e.code().category() == mongocxx::server_error_category())
        {
            _logger->add(_m_type.ERROR, "SERVER ERROR:", e.what());
        }
    }
}

/**
 * Establish connection with an specific MongoDB database an collection.
 *
 * @param database  Name of the database to which the connection has to be established.
 * @param collection Name of the collectio to which the connection has to be established.
 *
 */

void mongodb_manager::connectToCollection(QString database, QString collection)
{
    //QStringList id_list;
    // Initialize connection with MongoDB
    try
    {
        _collection_MDB = (*_conn)[database.toStdString()][collection.toStdString()];
        _database_MDB = (*_conn)[database.toStdString()];

        //mongodb_manager::getDocumentList(&id_list, &_document_list);
        // Save names of the collection and database that we want to connect:
        _current_database_name = database;
        _current_collection_name = collection;
    }
    catch(const mongocxx::v_noabi::query_exception& e)
    {
        if(e.code().category() == mongocxx::server_error_category())
        {
            _logger->add(_m_type.ERROR, "SERVER ERROR:", e.what());
        }
    }
}

/**
 * Establish connection with an specific MongoDB database an collection.
 *
 * @param database Name of the database to which the connection has to be established.
 * @param collection Name of the collection to which the connection has to be established.
 *
 */

void mongodb_manager::connectToCollection(std::string database, std::string collection)
{
    // Initialize connection with MongoDB
    try
    {
        _collection_MDB = (*_conn)[database][collection];
        _database_MDB = (*_conn)[database];

        //mongodb_manager::getDocumentList(&_id_list, &_document_list);
        // Save names of the collection and database that we want to connect:
        _current_database_name = QString::fromStdString(database);
        _current_collection_name = QString::fromStdString(collection);
    }
    catch(const mongocxx::v_noabi::query_exception& e)
    {
        if(e.code().category() == mongocxx::server_error_category())
        {
            _logger->add(_m_type.ERROR, "SERVER ERROR:", e.what());
        }
    }
}

/**
 * Obtain two lists, one with the id of all the documents in the collection, and the other one with the content of the documents.
 *
 * @param  id_list Container for the documents id.
 * @param  document_list Container for the documents.
 *
 */

void mongodb_manager::getDocumentList(QStringList *id_list, std::vector<mongodb_document> *document_list)
{
    /// ToDo: Divide this function in two different ones, one for getting the id's and one for getting the json files.
    // Clear the collection list
    id_list->clear();
    document_list->clear();
    // Initialize cursor
    mongocxx::cursor cursor = _collection_MDB.find({});
    // Iterate over all the elements in the collection
    for(bsoncxx::document::view doc : cursor)
    {
        // Obtain id of the Json object in the collection        
        mongodb_document document(doc);
        QString id = document.getId();
        // Add id to the collection list
        id_list->push_back(id);
        document_list->push_back(document);
    }
}

/**
 * Once connected to a database and collection, this function returns the document that corresponds to the input id.
 *
 * @param  id Document id.
 * @return Json document.
 *
 */

mongodb_document mongodb_manager::getDocument(QString id)
{
    QStringList id_list;
    std::vector<mongodb_document> document_list;

    // Update document list
    mongodb_manager::getDocumentList(&id_list, &document_list);

    /// ToDo: Check if this can be done using runcomand:

    // Iterate over all the elements in the collection
    for(QString id_it : id_list)
    {
        if(id_it == id)
        {
            int index = id_list.indexOf(id_it);
            mongodb_document doc_it = document_list.at(index);
            return doc_it.toQByteArray();
        }
    }
    _logger->add(_m_type.ERROR, " Json file with id ", id, " NOT found in the database.");

    mongodb_document null_doc;
    return null_doc;
}

/**
 * Delete selected user from the admin database.
 *
 * @param user Name of the user (remember to remove the .admin from the user).
 * @return True if the user was deleted, false otherwise.
 *
 */

bool mongodb_manager::deleteUser(QString user)
{    
    if(!mongodb_manager::verifyUser(user))
    {
        _logger->add(_m_type.ERROR, "In function: deleteUser, selected user: ", user, " doesn't exist.");
    }
    else if(user.isEmpty())
    {
        _logger->add(_m_type.ERROR, "In function: deleteUser, either selected user: ", user, " is empty.");
    }
    else
    {
        mongodb_manager::connectToDatabase(ADMIN_DB);
        bsoncxx::document::value command = mongodb_manager::createTemplate(_actions.DELETE_USER, user);
        _database_MDB.run_command(bsoncxx::document::view_or_value(command));
        _logger->add(_m_type.INFO, "Deleting user: ", user);
        return true;
    }
    return false;
}

/**
 * Add new user to the admin database.
 *
 * @param  user Name of the user (remember to remove the .admin from the user).
 * @return True if opertation was succesfull, false otherwise.
 *
 */

bool mongodb_manager::addUser(QString user, QString password)
{
    if(mongodb_manager::verifyUser(user))
    {
        _logger->add(_m_type.ERROR, "In function: addUser, selected user: ", user, " already exists");
    }
    else if(user.isEmpty() || password.isEmpty())
    {
        _logger->add(_m_type.ERROR, "In function: addUser, either selected user: ", user, " or password: ", password, " is empty");
    }
    else
    {
        mongodb_manager::connectToDatabase(ADMIN_DB);
        bsoncxx::document::value command = mongodb_manager::createTemplate(_actions.ADD_USER, user, password);
        _database_MDB.run_command(bsoncxx::document::view_or_value(command));
        _logger->add(_m_type.INFO, "Adding user: ", user);
        return true;
    }
    return false;
}

/**
 * Delete selected role off a user from the admin database.
 *
 * @param  user Name of the user (remember to remove the .admin from the user).
 * @return True if opertation was succesfull, false otherwise.
 *
 */

bool mongodb_manager::revokeRoleFromUser(QString user, QString database, QString role )
{
    if(database.isEmpty() || user.isEmpty() || role.isEmpty())
    {
        _logger->add(_m_type.ERROR, "In function: revokeRoleFromUser, either selected user: ", user, " database: ", database, " or role: ", role, " is empty.");
    }
    else if(!mongodb_manager::verifyDatabase(database))
    {
        _logger->add(_m_type.ERROR, "In function: revokeRoleFromUser, selected database: ", database, " doesn't exist in MongoDB");
    }
    else if(!mongodb_manager::verifyUser(user))
    {
        _logger->add(_m_type.ERROR, "In function: revokeRoleFromUser, selected user: ", user, " doesn't exist in MongoDB");
    }
    else if(!mongodb_manager::verifyRole(role))
    {
        _logger->add(_m_type.ERROR, "In function: revokeRoleFromUser, selected role: ", role, " is not a valid role");
    }
    else
    {
        mongodb_manager::connectToDatabase(ADMIN_DB);
        bsoncxx::document::value command = mongodb_manager::createTemplate(_actions.REVOKE_ROLE, user, database, role);
        _database_MDB.run_command(bsoncxx::document::view_or_value(command));
        _logger->add(_m_type.INFO, "Revoked role: ", role, " in database: ", database, " to user: ", user);
        return true;
    }
    return false;
}

/**
 * Grant a new role to a user from the admin database.
 *
 * @param  user User to be granted the role (remember to remove the .admin from the user).
 * @param  database Database in which the role will be granted.
 * @param  role Role to be granted.
 * @return True if opertation was succesfull, false otherwise.
 *
 */

bool mongodb_manager::grantRoleToUser(QString user, QString database, QString role)
{
    if(database.isEmpty() || user.isEmpty() || role.isEmpty())
    {
        _logger->add(_m_type.ERROR, "In function: grantRoleToUser, either selected user: ", user, " database: ", database, " or role: ", role, " is empty.");
    }
    else if(!mongodb_manager::verifyDatabase(database))
    {
        _logger->add(_m_type.ERROR, "In function: grantRoleToUser, selected database: ", database, " doesn't exist in MongoDB");
    }
    else if(!mongodb_manager::verifyUser(user))
    {
        _logger->add(_m_type.ERROR, "In function: grantRoleToUser, selected user: ", user, " doesn't exist in MongoDB");
    }
    else if(!mongodb_manager::verifyRole(role))
    {
        _logger->add(_m_type.ERROR, "In function: grantRoleToUser, selected role: ", role, " is not a valid role");
    }
    else
    {
        mongodb_manager::connectToDatabase(ADMIN_DB);
        bsoncxx::document::value command = mongodb_manager::createTemplate(_actions.GRANT_ROLE, user, database, role);
        _database_MDB.run_command(bsoncxx::document::view_or_value(command));
        _logger->add(_m_type.INFO, "Granted role: ", role, " in database ", database, " to user ", user);
        return true;
    }
    return false;
}

/**
 * Add a document to a collection specifying the id.
 *
 * @param  document File to be added to the collection.
 * @param  id Document id.
 * @return Id given to the added document.
 *
 */

QString mongodb_manager::addDocument(mongodb_document document, QString id)
{
    // Get the document id.
    QString document_id = document.getId();

    if(document_id == id)
    {
        _logger->add(_m_type.INFO, "The provided id [ ", id, " ] matches with the given document [ ", document_id, " ]. Updating the current database document with the given one");
        return mongodb_manager::addDocument(document);
    }
    else if(document_id == "NULL")
    {
        _logger->add(_m_type.INFO, "The provided document doesn't have a valid MongoDB Id [ ", document_id, " ]. Adding it to the database with a new id");
        return mongodb_manager::addDocument(document);
    }
    else
    {
        _logger->add(_m_type.INFO, "The provided id [ ", id, " ] does not match with the given document [ ", document_id, " ]. Assigning the given id to the document");
        document.updateDocumentId(id);
        return mongodb_manager::addDocument(document);
    }
    return NULL;
}

/**
 * Add a document to a collection.
 *
 * @param  document Filet to be added to the collection.
 * @return Id given to the added document.
 *
 */

QString mongodb_manager::addDocument(mongodb_document document)
{
    // Check if the json_Obj has a valid MongoDB id and it's part of the collection:
    QString id = document.getId();

    if(id != "NULL")
    {
        QStringList id_list;
        std::vector<mongodb_document> document_list;

        // Update document list
        mongodb_manager::getDocumentList(&id_list, &document_list);

        /// ToDo: Check if this can be done using runcomand:
        for(QString _id_it : id_list)
        {
            if(_id_it == id)
            {
                _logger->add(_m_type.INFO, "JSON file with ID: ", id, " Found in the database. Updating the document");
                bsoncxx::document::value replacement = document.toBsoncxxDocVal();
                bsoncxx::builder::stream::document doc{};
                bsoncxx::document::value filt  = doc << "_id" << bsoncxx::oid(id.toStdString()) << bsoncxx::builder::stream::finalize;

                // Replace the element in the collection
                _collection_MDB.replace_one(filt.view(), replacement.view());
                return id;
            }
        }
    }
    else
    {
        _logger->add(_m_type.INFO, " The provided JSON object does not have a valid MongoDB _id format, probably it is a new document and it has not been assigned an id yet");
    }

    // If id wasn't found in the database or the JSON object, add document to the database and return the id assigned to it.
    _logger->add(_m_type.INFO, "Json file with ID: ", id, " not found in the database. Adding it now");

    // Convert JSON objet to bsoncxx standard
    bsoncxx::document::value bsoncxx_doc = document.toBsoncxxDocVal();

    // Add to collection and save the returned _id
    auto result = _collection_MDB.insert_one(bsoncxx_doc.view());

    if (result->inserted_id().type() == bsoncxx::type::k_oid)
    {
        bsoncxx::oid oid = result->inserted_id().get_oid().value;
        QString id = QString::fromStdString(oid.to_string());
        _logger->add(_m_type.INFO, " Id of the added document : ", id);
        return id;
    }

    return NULL;
}

/**
 * Given a document id, save the specified document to disk.
 *
 * @param id Id of the document to be exported .
 * @param file_name Name to be given to the file.
 * @return True if the document was deleted, false otherwise.
 *
 */

bool mongodb_manager::exportDocument(QString id, QString file_name)
{
    if(id.isEmpty() || file_name.isEmpty())
    {
        _logger->add(_m_type.ERROR, "In function: exportDocument, either id: ", id, " or file_name: ", file_name, " is empty");
    }
    else
    {
        mongodb_document document = mongodb_manager::getDocument(id);
        document.saveToDisk(file_name);
        return true;
    }
    return false;
}

/**
 * Configure the uri to establish the connection
 *
 * @param user User to acces the database.
 * @param password Password for the user.
 * @param database Database to be accesed,
 * @param port
 * @param host
 *
 */

void mongodb_manager::configureConnection(std::string user, std::string password, std::string database, std::string port, std::string host)
{
    // Save connection information
    _user = QString::fromStdString(user);
    // Construct string with all the necessary information.
    std::string configuration = "mongodb://"  +  user + ":" + password + "@" + host + ":" + port + "/" + database;
    // Create the uri object (static so it's not erased)
    mongocxx::uri uri{configuration};
    // Establish connection
    _conn =  new mongocxx::client(uri);

    _logger->add(_m_type.INFO, "Configuration string is: ", QString::fromStdString(configuration));
    _logger->printMessagelog(_m_type.ALL);
}

/**
* Configure the uri to establish the connection
*
* @param user User to acces the database.
* @param password Password for the user.
* @param database Database to be accesed,
* @param port
* @param host
*
*/

void mongodb_manager::configureConnection(QString user, QString password, QString database, QString port, QString host)
{
    // Save connection information
    _user = user;
    // Construct string with all the necessary information.
    QString configuration = "mongodb://"  +  user + ":" + password + "@" + host + ":" + port + "/" + database;
    // Create the uri object (static so it's not erased)
    mongocxx::uri uri{configuration.toStdString()};
    // Establish connection
    _conn =  new mongocxx::client(uri);
}

/**
 * Get the admin user that logged into MongoDB.
 *
 * @return The admin user used to log in MongoDB
 *
 */

QString mongodb_manager::getAdminUser()
{
    return _user;
}

/**
 * Get a list with all the collections in the database.
 *
 * @param database Name to the database in which the collection lists will be obtained.
 * @param collection_list Container to save the list of collections.
 *
 */

void mongodb_manager::getCollectionList(QString database, QStringList *collection_list)
{
    // Clear the container for the collection list
    collection_list->clear();
    // Connect to the database
    mongodb_manager::connectToDatabase(database);
    // Get the cursor to loop trough all the collections in the database
    mongocxx::cursor cursor_collection = _database_MDB.list_collections();
    // Add all the collection names to the list
    for(const bsoncxx::document::view& collection :cursor_collection)
    {
        bsoncxx::document::element ele = collection["name"];
        std::string name = ele.get_utf8().value.to_string();

        QString collection_name = QString::fromStdString(name);
        collection_list->push_back(collection_name);
    }
}

/**
 * Get a list with all the databases in MongoDB.
 *
 * @param database_list Container to save the list of databases.
 *
 */

void mongodb_manager::getDatabaseList(QStringList *database_list)
{
    // Clear the container for the database list
    database_list->clear();
    // Get the cursor to loop trough all the databases in MongoDb
    mongocxx::cursor cursor_db = _conn->list_databases();
    // Add all the databses names to the list
    for(const bsoncxx::document::view& database :cursor_db)
    {
        bsoncxx::document::element ele = database["name"];
        std::string name = ele.get_utf8().value.to_string();

        // Avoid adding the admin, config and local databases since they are sytem related
        if((name != "config") & (name != "local") & (name != "admin"))
        {
            QString database_name = QString::fromStdString(name);
            database_list->push_back(database_name);
        }
    }
}

/**
 * Get a list with all the users in MongoDB.
 *
 * @param user_list Container to save the list of users.
 *
 */

void mongodb_manager::getUsersList(QStringList *user_list)
{
    // Clear the container for the users list
    user_list->clear();
    // Connect to the admin database and the users collection
    mongodb_manager::connectToCollection(ADMIN_DB, USERS_COLLECTION);
    // Get the cursor to loop trough all the users in MongoDb
    mongocxx::cursor cursor = _collection_MDB.find({});
    // Add all the user names to the list
    for(bsoncxx::document::view doc : cursor)
    {
        //Save the current user document as a QJsonObject
        mongodb_document doc_object(doc);
        // The id corresponds to the user
        QString user_id = doc_object.getId();
        user_list->push_back(user_id);
    }
}

/**
 * Add a collection to a specified database.
 *
 * @param database Name to the database in which the collection will be added.
 * @param collection Name of the collection to be added.
 * @return True if the document was deleted, false otherwise.
 *
 */

bool mongodb_manager::addCollection(QString database, QString collection)
{

    if(database.isEmpty() || collection.isEmpty())
    {
        _logger->add(_m_type.ERROR, "In function: addCollection, either datasbase: ", database, " or collection: ", collection, " is empty");
    }
    else if(!mongodb_manager::verifyDatabase(database))
    {
        _logger->add(_m_type.ERROR, "Database: ", database, " doesn't exist in MongoDB");
    }
    else if(mongodb_manager::verifyCollection(database, collection))
    {
        _logger->add(_m_type.ERROR, "Collection: ", database, " Already exists in MongoDB");
    }
    else
    {        
        mongodb_manager::connectToCollection(database, collection);
        _database_MDB.create_collection(collection.toStdString());
        return true;
    }

    return false;
}

/**
 * Delete a collection from a database.
 *
 * @param database Name to the database in which the collection will be deleted.
 * @param collection Name of the collection to be deleted.
 * @return True if the document was deleted, false otherwise.
 *
 */

bool mongodb_manager::deleteCollection(QString database, QString collection)
{
    if(database.isEmpty() || collection.isEmpty())
    {
        _logger->add(_m_type.ERROR, "In function: deleteCollection, either database: ", database, " or collection: ", collection, " is empty");
    }
    else if(!mongodb_manager::verifyDatabase(database))
    {
        _logger->add(_m_type.ERROR, "Database: ", database, " doesn't exist in MongoDB");
    }
    else if(!mongodb_manager::verifyCollection(database, collection))
    {
        _logger->add(_m_type.ERROR, "Collection: ", database, " doesn't exist in MongoDB");
    }
    else
    {
        mongodb_manager::connectToCollection(database, collection);
        _collection_MDB.drop();
        return true;
    }
    return false;
}

/**
 * Add a database to MongoDB.
 *
 * @param database Name to the database to be added.
 * @return True if the document was deleted, false otherwise.
 *
 */

bool mongodb_manager::addDatabase(QString database)
{
    if(database.isEmpty())
    {
        _logger->add(_m_type.ERROR, "In function addDatabase, field: database: ", database, " is empty");
    }
    else if(mongodb_manager::verifyDatabase(database))
    {
        _logger->add(_m_type.ERROR, "Database: ", database, " already exists in MongoDB");
    }
    else
    {
        mongodb_manager::connectToDatabase(database);
        // In order for the database to be visualized it needs to have a collection with a file in it.
        _collection_MDB = _database_MDB.create_collection(database.toStdString()+"_collection");
        mongodb_document dummy_document("{\"parent_database\":\"" + database.toStdString() + "\"}");
        QString id = mongodb_manager::addDocument(dummy_document);
        _logger->add(_m_type.INFO, "Database: ", database, " added to MongoDB");
        return true;
    }
    return false;
}

/**
 * Delete a database from MongoDB.
 *
 * @param database Name to the database to be deleted.
 * @return True if the document was deleted, false otherwise.
 *
 */

bool mongodb_manager::deleteDatabase(QString database)
{
    if(database.isEmpty())
    {
        _logger->add(_m_type.ERROR, "In function deleteDatabase, field: database: ", database, " is empty");
    }
    else if(!mongodb_manager::verifyDatabase(database))
    {
        _logger->add(_m_type.ERROR, "Database: ", database, " doesn't exist in MongoDB");
    }
    else
    {
        mongodb_manager::clearDatabaseRoles(database);
        mongodb_manager::connectToDatabase(database);
        _database_MDB.drop();
        _logger->add(_m_type.INFO, "Deleting database: ", database);
        return true;
    }
    return false;
}

/**
 * To download all the documents of a collection in a single json file.
 * The document will save all the files in the collection with the following structure:
 *
 * {
 *  "Document0": {...},
 *  "Document1": {...},
 *        ....
 *  "DocumentN": {...}
 * }
 *
 * @param  file_name Name for the file where to save the collection.
 *
 */

void mongodb_manager::downloadCollection(QString file_name)
{
    QStringList id_list;
    std::vector<mongodb_document> document_list;

    // Update document list
    mongodb_manager::getDocumentList(&id_list, &document_list);

    // New document to save the collection in
    mongodb_document collection_Object;
    QString doc_key;

    for(QString id_it : id_list)
    {
        //Update identifier:
        doc_key.clear();
        doc_key.append("Document" + QString::number(id_list.indexOf(id_it)));

        //Add document to the Json containing the collection:
        mongodb_document doc_val(mongodb_manager::getDocument(id_it));
        QJsonObject doc = doc_val.getDoc();
        collection_Object.insertKeyValuePair(doc_key,doc);
    }

    //Save file
    collection_Object.saveToDisk(file_name);
}

/**
 * Clear all the roles regarding one database. If this step is not performed, when a database with the same name is created,
 * the users will have the same rights like in the deleted one
 *
 * @param database Name of the database to be cleared.
 *
 */

void mongodb_manager::clearDatabaseRoles(QString database)
{
    QStringList users_list;
    QStringList database_list;
    QStringList user_roles_list;

    // Update database list
    mongodb_manager::getDatabaseList(&database_list);
    mongodb_manager::getUsersList(&users_list);

    // Get the index of the specified database
    int index = database_list.indexOf(database);

    QString role;

    // Loop through all the users and remove the roles regarding the input database
    for(QString user : users_list)
    {
        // Get the current user and its roles
        mongodb_manager::getUserRolesList(user, &user_roles_list);
        role = user_roles_list.at(index);

        // In case that the users has a valid role in the database, remove it
        if(mongodb_manager::verifyRole(role))
        {
            mongodb_manager::revokeRoleFromUser(user.remove(ADMIN_DB_EXTEND), database, role);
        }
    }
}

/**
 * Get a list with all the roles for a specific users in MongoDB.
 *
 * @param user Name of the user.
 * @param user_roles_list Container to save the roles list of the selected user.
 *
 */

void mongodb_manager::getUserRolesList(QString user, QStringList *user_roles_list)
{
    QStringList database_list;
    mongodb_roles roles;

    // Connect to the admin database and the users collection.
    mongodb_manager::connectToCollection(ADMIN_DB, USERS_COLLECTION);

    // Clean and innitialize the user roles list.
    user_roles_list->clear();

    // Update the database list.
    mongodb_manager::getDatabaseList(&database_list);

    // Get the user data.
    mongodb_document user_data= mongodb_manager::getDocument(user);

    // A user can have multiple roles. We have to save the roles of the user in an array.
    QJsonValue roles_Value = (user_data.getDoc()).value("roles");
    QJsonArray roles_Array = roles_Value.toArray();

    // Iterate over all the databases and check the current user rights.
    for(QString database : database_list)
    {
        // Add ROLE_NULL to the list as a sign that the user has no roles found yet.
        user_roles_list->push_back(roles.ROLE_NULL);

        // Iterate over the roles of the user and check if it has any regarding the current database.
        for(QJsonValueRef v : roles_Array)
        {
            QJsonObject obj = v.toObject();
            // In case that a role for the current database is found, update the list.
            if(obj.value("db").toString() == database)
            {
                int index = database_list.indexOf(database);
                user_roles_list->replace(index,(obj.value("role").toString()));

                break;
            }
        }
    }
}

/**
 * Get the role of a user in an specific database.
 *
 * @param user Name of the user.
 * @param database Name of the database.
 * @return Role of the user in the requested database.
 *
 */

QString mongodb_manager::getUserRole(QString user, QString database)
{
    QStringList database_list;
    QStringList user_roles_list;

    // Formate the user properly
    user.prepend(ADMIN_DB_EXTEND);

    // Update user roles list and database list
    mongodb_manager::getUserRolesList(user, &user_roles_list);
    mongodb_manager::getDatabaseList(&database_list);

    // Get role
    int database_index = database_list.indexOf(database);
    QString role = user_roles_list.at(database_index);

    return role;
}

/**
 * Get the number of users in MongoDB.
 *
 * @return Number of users.
 *
 */

int mongodb_manager::getNumberOfUsers()
{
    QStringList users_list;

    // Update information
    mongodb_manager::getUsersList(&users_list);
    return users_list.size();
}

/**
 * Get the number of databases in MongoDB.
 *
 * @return: Number of databases.
 *
 */

int mongodb_manager::getNumberOfDatabases()
{
    QStringList database_list;

    // Update information
    mongodb_manager::getDatabaseList(&database_list);
    return database_list.size();
}

/**
 * Load a document file from disk and add it to the collection.
 *
 * @param file_path Path to the file.
 *
 */

void mongodb_manager::importDocument(QString file_path)
{
    int MAX_FILE_SIZE = 16000000 - 1;

    // Load size from disk
    mongodb_document document;
    document.loadFromDisk(file_path);

    // Get the size of the file
    QByteArray bytes = (document.toQString()).toUtf8();
    int file_size = bytes.size();

    if(file_size > MAX_FILE_SIZE)
    {
        _logger->add(_m_type.INFO, " Uploading a file bigger than 16 Mb, size is: ", QString(file_size));

        // initialize connection to GridFS
        mongodb_manager::connectGridFS(_current_database_name);

        // Write file using GridFS
        QString id = mongodb_manager::addDocumentGridFS((document.toQString()), file_path.toStdString());
    }
    else
    {
        _logger->add(_m_type.INFO, " Uploading a file smaller than 16 Mb");
        mongodb_document json_obj(document);
        mongodb_manager::addDocument(json_obj);
    }
}

/**
 * Load a document file from disk and add it to the collection (with parser).
 *
 * @param id Id of the document to be removed.
 * @return True if the document was deleted, false otherwise.
 *
 */

bool mongodb_manager::deleteDocument(QString id)
{
    QStringList id_list;
    std::vector<mongodb_document> document_list;

    // Update document list
    mongodb_manager::getDocumentList(&id_list, &document_list);

    // Depending on which collection is selected, the method for deleting the file is different
    if(_current_collection_name == "fs.files")
    {
        // Initialize connection to GridFS
        connectGridFS(_current_database_name);
        mongodb_document doc;
        doc.updateDocumentId(id);

        bsoncxx::types::value id_GridFS = doc.getIdGridfsFormat();
        _gridfs_bucket.delete_file(id_GridFS);
        return true;
    }
    else if(_current_collection_name == "fs.chunks")
    {
        _logger->add(_m_type.INFO, " Can't delete elements from this database");
        return false;
    }
    else
    {
        // Iterate over all the elements in the collection
        for(QString id_it : id_list)
        {
            if(id_it == id)
            {
                _logger->add(_m_type.INFO, " Json file with id ", id, " found in the database");
                bsoncxx::builder::stream::document doc{};
                bsoncxx::document::value filt  = doc << "_id" << bsoncxx::oid(id.toStdString()) << bsoncxx::builder::stream::finalize;

                _collection_MDB.delete_one(filt.view());
                return true;
            }
        }
    }

    _logger->add(_m_type.ERROR, " Json file with id ", id, " NOT found in the database. ");
    return false;
}

/**
 * Get a table with the roles for each user regarding all the databases.
 *
 * @param roles_table Container for the table info.
 *
 */

void mongodb_manager::getRolesTable(std::vector<QStringList> *roles_table)
{
    QStringList database_list;
    QStringList users_list;
    QStringList user_roles_list;

    // Empty the container for the table
    roles_table->clear();

    // Get a vector with a list of the database names
    mongodb_manager::getDatabaseList(&database_list);
    mongodb_manager::getUsersList(&users_list);

    // Connect to the admin database, use the collection that contains the list of the users
    mongodb_manager::connectToCollection(ADMIN_DB, USERS_COLLECTION);

    // Iterate over all the users
    for(QString user : users_list)
    {
        // Get user roles
        mongodb_manager::getUserRolesList(user, &user_roles_list);

        // Don't show the admin user that is running the program
        if(user.remove(ADMIN_DB_EXTEND) != _user)
        {
            // Add row with the user information
            roles_table->push_back(user_roles_list);
        }
    }
}

/**
 * Set a custom mongodb_logger different than the one created by default, this is used
 * when the manager is created by other classes and we want both to use the same logger.
 *
 * @param custom_logger
 *
 */

void mongodb_manager::setCustomLogger(mongodb_logger *custom_logger)
{
    if(_logger_custom == false)
    {
        delete _logger;
        _logger_custom = true;
    }

    if(_logger != custom_logger)
    {
        _logger = custom_logger;
    }
}

/**
 * Print the log with all the messages generated by the manager functions.
 */

void mongodb_manager::printlog()
{
    _logger->printMessagelog(_m_type.ALL);
}

/**
 * Create the command required for using functio: runcomand(). Depending on the type of command, the input parameters will change.
 *
 * @param option
 * @param field1
 * @param field2
 * @param field3
 *
 */

bsoncxx::document::value mongodb_manager::createTemplate(QString option,QString field1 ,QString field2,QString field3)
{
    mongodb_actions actions;

    bsoncxx::builder::stream::document doc{};

    if(option ==  actions.ADD_USER)
    {
        bsoncxx::document::value val = doc << "createUser" << field1.toStdString() <<
                                              "pwd" << field2.toStdString() <<
                                              "roles" << bsoncxx::builder::stream::open_array <<
                                              bsoncxx::builder::stream::close_array <<
                                              bsoncxx::builder::stream::finalize;
        return val;
    }
    else if(option ==  actions.REVOKE_ROLE)
    {
        bsoncxx::document::value val = doc << "revokeRolesFromUser" << field1.toStdString()  <<
                                              "roles" << bsoncxx::builder::stream::open_array <<
                                              bsoncxx::builder::stream::open_document << "role" << field3.toStdString()  << "db" << field2.toStdString()  << bsoncxx::builder::stream::close_document <<
                                              bsoncxx::builder::stream::close_array <<
                                              bsoncxx::builder::stream::finalize;
        return val;
    }
    else if(option ==  actions.GRANT_ROLE)
    {
        bsoncxx::document::value val = doc << "grantRolesToUser" << field1.toStdString() <<
                                              "roles" << bsoncxx::builder::stream::open_array <<
                                              bsoncxx::builder::stream::open_document << "role" << field3.toStdString() << "db" << field2.toStdString()  << bsoncxx::builder::stream::close_document <<
                                              bsoncxx::builder::stream::close_array <<
                                              bsoncxx::builder::stream::finalize;
        return val;
    }
    else if(option ==  actions.DELETE_USER)
    {
        bsoncxx::document::value val = doc << "dropUser" << field1.toStdString() << bsoncxx::builder::stream::finalize;
        return val;
    }
    else
    {
        _logger->add(_m_type.ERROR,"In function createTemplate, the option introduced is not valid");
    }
}

/**
 * Load a .json file containing the credentials required to log into MongoDB. If the credentials file doesn't have all the required fields, the returned map will be empty.
 *
 * @param  file_path Path to the credentials file
 * @return Map with the credentials information.
 *
 */

QVariantMap mongodb_manager::loadCredentialsFile(QString file_path)
{
    mongodb_document file;

    /// ToDo: Why is it neccessary to declare the temp variable? if it is not declared, the program fails.
    QString temp = file_path;

    file.loadFromDisk(file_path);
    QVariantMap output = file.toQVariantMap();

    if(!output["user"].isNull() && !output["password"].isNull() && !output["database"].isNull() && !output["port"].isNull() && !output["host"].isNull())
    {
        return output;
    }
    else
    {
        _logger->add(_m_type.ERROR, "The selected credentials file doesn't have the neccessary fields");
        output.clear();
        return output;
    }
}

/**
 * Check that the user exists in MongoDB.
 *
 * @param  user User to be checked
 * @return True if the user exists, false otherwise.
 *
 */

bool mongodb_manager::verifyUser(QString user)
{
    QStringList users_list;

    user.prepend(ADMIN_DB_EXTEND);
    mongodb_manager::getUsersList(&users_list);

    for(QString element : users_list)
    {
        if(element == user)
        {
            return true;
        }
    }
    return false;
}

/**
 * Check that the database exists in MongoDB.
 *
 * @param  database Database to be checked
 * @return True if the database exists, false otherwise.
 *
 */

bool mongodb_manager::verifyDatabase(QString database)
{
    QStringList database_list;

    mongodb_manager::getDatabaseList(&database_list);

    for(QString element : database_list)
    {
        if(element == database)
        {
            return true;
        }
    }
    return false;
}

/**
 * Check that the collection exists in a database in MongoDB.
 *
 * @param  collection Collection to be checked
 * @return True if the collection exists, false otherwise.
 *
 */

bool mongodb_manager::verifyCollection(QString database, QString collection)
{
    QStringList collection_list;
    mongodb_manager::getCollectionList(database, &collection_list);

    for(QString element : collection_list)
    {
        if(element == collection)
        {
            return true;
        }
    }
    return false;
}

/**
 * Check that the role is a valid role (read/readWrite).
 *
 * @param  role Role to be checked
 * @return True if the role is valid, false otherwise.
 *
 */

bool mongodb_manager::verifyRole(QString role)
{
    mongodb_roles roles;

    if(role == roles.ROLE_R || role == roles.ROLE_RW)
    {
        return true;
    }

    return false;
}


/// ToDo: Revise the following functions






/**
 * For files bigger than 16 Mb, use the GridFS library to upload them to the database. GridFS by default uses two collections fs.files
 * and fs.chunks to store the file's metadata and the chunks. Each chunk is identified by its unique _id ObjectId field. The fs.files
 * serves as a parent document. The files_id field in the fs.chunks document links the chunk to its parent.
 *
 * @param  document File to be uploaded to the database.
 * @param  file_name Name given to the file in the fs.file collection.
 * @return Id of the added document.
 *
 */

QString mongodb_manager::addDocumentGridFS(QString document, std::string file_name)
{
    // Store the content of the QString in a std::vector (format needed by GridFS methods)
    std::string document_stdstring = document.toStdString();
    std::string::iterator begining = document_stdstring.begin();
    std::string::iterator ending = document_stdstring.end();
    std::vector<uint8_t> json_vector(begining, ending);
    uint8_t *json_vector_ptr = &json_vector[0];

    // Iniitalize the GridFS uploader method
    mongocxx::gridfs::uploader uploader = _gridfs_bucket.open_upload_stream(file_name); //Using the same name of the file

    // Upload the json_vector to the database and close uploader once it's done
    uploader.write(json_vector_ptr, json_vector.size() );
    mongocxx::result::gridfs::upload result = uploader.close();

    // Get the id of the written file
    bsoncxx::types::value bson_id = result.id();
    bsoncxx::types::b_oid bson_oid = bson_id.get_oid();
    bsoncxx::oid bson_oid_value = bson_oid.value;
    QString id = QString::fromStdString(bson_oid_value.to_string());

    return id;
}

/**
 * For files bigger than 16 Mb, use the GridFS library to download a file from the database.
 *
 * @param id Id of the file (This file should belong to the fs.files collection!!!)
 * @return Content of the file.
 *
 */

mongodb_document mongodb_manager::getDocumentGridFS(QString id)
{
    // Initialize connection to GridFS
    connectGridFS(_current_database_name);

    // Build a valid mongodb id structure
    bsoncxx::builder::stream::document id_stream{};
    bsoncxx::document::value id_doc_value = id_stream << "_id" << bsoncxx::oid(id.toStdString()) << bsoncxx::builder::stream::finalize;
    bsoncxx::document::view id_doc_view = id_doc_value.view();

    // The open_download_stream method requires bsoncxx::types::value, therefore a conversion from bsoncxx::document::value is needed
    bsoncxx::document::element doc_element = id_doc_view["_id"];
    bsoncxx::types::value id_types_value{doc_element.get_value()};

    // Strat the downloader with the given id
    mongocxx::gridfs::downloader downloader = _gridfs_bucket.open_download_stream(id_types_value);
    // Get the lenght of the file to be downloaded in a std::vector
    std::int64_t file_length = downloader.files_document()["length"].get_int64().value;
    // Initialize the vector where the downloaded file will be saved
    std::vector<std::uint8_t> file_download_vector((int(file_length)));
    std::uint8_t *big_file_download = &file_download_vector[0];
    // Read the file and save it into the vector
    downloader.read(big_file_download, file_length);

    // Convert the content of the std::vector to a QByte array
    QByteArray* json_QByte = new QByteArray(reinterpret_cast<const char*>(file_download_vector.data()), file_download_vector.size());

    return *json_QByte;
}


/**
 * Save the users roles table to ./UsersAndRoles.csv file.
 */

void mongodb_manager::saveUsersAndRolesTable()
{
    QStringList database_list;
    QStringList users_list;
    std::vector<QStringList> roles_table;

    // Update
    mongodb_manager::getUsersList(&users_list);
    mongodb_manager::getDatabaseList(&database_list);
    mongodb_manager::getRolesTable(&roles_table);

    // Open file
    std::ofstream myfile;
    QString filePath = SOURCE_PATH;
    filePath.append("/shared/UsersAndRoles.csv");
    myfile.open(filePath.toStdString());

    // Add first row with databeses
    myfile << " , ";
    for(QString database : database_list)
    {
        myfile << database.toStdString() << ", ";
    }

    myfile << "\n";

    // Remove the admin from the user list
    std::vector<QString> users_no_admin;
    for(int i = 1; i < users_list.size(); i++)
    {
        QString user_i =users_list.at(i);
        user_i.remove(ADMIN_DB_EXTEND);

        if(user_i != _user)
        {
            users_no_admin.push_back(user_i);
        }
    }

    // Add a row for each user with name and roles
    for(uint i = 0; i < users_no_admin.size(); i++)
    {
        myfile << users_no_admin.at(i).toStdString() << ", ";
        for(int j = 0; j < roles_table.at(0).size(); j++)
        {
            myfile << roles_table.at(i).at(j).toStdString();
            if(j < roles_table.at(0).size() - 1)
            {
                myfile<< ", ";
            }
        }
        myfile << "\n";
    }
    myfile << "\n";

    myfile.close();

    //_logger->add(_m_type.INFO, "Updated csv file");
}
