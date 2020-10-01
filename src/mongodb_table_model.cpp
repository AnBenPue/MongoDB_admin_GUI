/// \cond
#include <QDebug>
/// \endcond

#include "mongodb_table_model.h"

mongodb_table_model::mongodb_table_model(QObject *parent)
    : QStandardItemModel(parent)
{
    _logger = new mongodb_logger;
}

/**
 * The mongodb_table_model first withdraws all the information from MongoDB (users,databases & roles).
 * Once this information is available, a table is created in order to show said information. After this initalization step,
 * the table modifications are not reflected in MongoDB, this means that if a new user is selected, a new row will be added
 * to the table and the acction addUser will be logged for later processing. The idea is to show how MongoDB would look once
 * the actions are applied without actually performing them.
 */

void mongodb_table_model::initializeModel()
{
    QStringList user_roles_list;
    QString user;

    // Clear the information containers
    _roles_table.clear();
    _users_list.clear();
    _database_list.clear();

    // Save the user name used to log in MongoDB
    user = mongodb_table_model::getAdminUser();
    user.prepend(ADMIN_DB_EXTEND);

    // Get the list of the databases
    mongodb_manager::getDatabaseList(&_database_list);
    _num_of_databases = mongodb_table_model::getNumberOfDatabases();

    // Get the list of the users
    mongodb_manager::getUsersList(&_users_list);
    // Remove the admin user
    int index = _users_list.indexOf(user);
    _users_list.erase(_users_list.begin() + index, _users_list.begin() + index+1);
    _num_of_users = _users_list.size();

    for(int i=0; i<_num_of_users; i++)
    {
        QString user_i = _users_list.at(i);

        mongodb_table_model::getUserRolesList(user_i, &user_roles_list);
        _roles_table.push_back(user_roles_list);

        _users_list[i].replace(QString(ADMIN_DB_EXTEND),QString(""));
    }

    // Update the GUI table
    mongodb_table_model::updateTable();
}

/**
 * Build the table with the proper size regarding number of users and databases.
 */

void mongodb_table_model::initializeTable()
{
    // Convert the std::vector<QString> containers to QStringList
    QStringList horizontal_header;
    QStringList vertical_header;

    // Give names to the table headers
    for(QString database : _database_list)
    {
        horizontal_header.append(database);
    }

    for(QString user : _users_list)
    {
        vertical_header.append(user);
    }

    // Define size of the table
    this->setRowCount(_num_of_users);
    this->setColumnCount(_num_of_databases);

    // Define name of the headers
    this->setHorizontalHeaderLabels(horizontal_header);
    this->setVerticalHeaderLabels(vertical_header);
}

/**
 * Initializes the table and populates the cell with the role values.
 */

void mongodb_table_model::updateTable()
{
    mongodb_table_model::initializeTable();

    for(int row_num = 0; row_num < _num_of_users ; row_num++)
    {
        for(int col_num = 0; col_num < _num_of_databases; col_num++)
        {
            QString role = _roles_table.at(row_num).at(col_num);
            QModelIndex index = this->index(row_num, col_num, QModelIndex());
            this->setData(index, role);
        }
    }

    this->saveUsersAndRolesTable();
}

/**
 * Check if the changes in the mongodb_table_model have been applied to MongoDB
 */

bool mongodb_table_model::isSaved()
{
    _logger->getActionLog(&_log_actions);
    return _log_actions.empty();
}

/**
 * Run all the actions registered in the actions logger. When .This function is called, all the actions will
 * take effect in MongoDB.
 */

void mongodb_table_model::runActions()
{
    _logger->getActionLog(&_log_actions);
    for(QStringList item : _log_actions)
    {
        if(item.at(0) == _actions.ADD_DATABASE)
        {
            mongodb_manager::addDatabase(item.at(1));
        }
        else if(item.at(0) == _actions.DELETE_DATABASE)
        {
            mongodb_manager::deleteDatabase(item.at(1));
        }
        else if(item.at(0) == _actions.ADD_USER)
        {
            mongodb_manager::addUser(item.at(1), item.at(2));
        }
        else if(item.at(0) == _actions.DELETE_USER)
        {
            mongodb_manager::deleteUser(item.at(1));
        }
        else if(item.at(0) == _actions.GRANT_ROLE)
        {
            mongodb_manager::grantRoleToUser(item.at(1), item.at(2), item.at(3));
        }
        else if(item.at(0) == _actions.REVOKE_ROLE)
        {
            mongodb_manager::revokeRoleFromUser(item.at(1), item.at(2), item.at(3));
        }
    }

    _logger->clearActionsList();
    mongodb_table_model::initializeModel();
}

/**
 * Clear the actions list from the logger to make sure that the past actions are discarded.
 */

void mongodb_table_model::cancelActions()
{
    _logger->clearActionsList();
    mongodb_table_model::initializeModel();
}

/// ----- Overrided funtions: ------ ///

/**
 * Get a list with all the users in the mongodb_table_model.
 *
 * @param users_list Container to save the list of users.
 *
 */

void mongodb_table_model::getUsersList(QStringList *users_list)
{
    users_list->clear();

    for(QString user : _users_list)
    {
        users_list->push_back(user);
    }
}

/**
 * Get a list with all the databases in the mongodb_table_model.
 *
 * @param database_list Container to save the list of databases.
 *
 */

void mongodb_table_model::getDatabaseList(QStringList *database_list)
{
    database_list->clear();

    for(QString database : _database_list)
    {
        database_list->push_back(database);
    }
}

/**
 * Check that the database exists in the mongodb_table_model.
 *
 * @param  database Database to be checked
 * @return True if the database exists, false otherwise.
 *
 */

bool mongodb_table_model::verifyDatabase(QString database)
{
    for(QString element : _database_list)
    {
        if(element == database)
        {
            return true;
        }
    }
    return false;
}

/**
 * Check that the user exists in the mongodb_table_model.
 *
 * @param  user User to be checked
 * @return True if the user exists, false otherwise.
 *
 */

bool mongodb_table_model::verifyUser(QString user)
{
    for(QString element : _users_list)
    {
        if(element == user)
        {
            return true;
        }
    }
    return false;
}

/**
 * Get the number of databases in the mongodb_table_model.
 *
 * @return: Number of databases.
 *
 */

int mongodb_table_model::getNumberOfDatabases()
{
    return _database_list.size();
}

/**
 * Get the number of users in the mongodb_table_model.
 *
 * @return: Number of users.
 *
 */

int mongodb_table_model::getNumberOfUsers()
{
    return _users_list.size();
}

/**
 * Add a database to the mongodb_table_model.
 *
 * @param database Name to the database to be added.
 * @return True if the document was deleted, false otherwise.
 *
 */

bool mongodb_table_model::addDatabase(QString database)
{
    mongodb_roles roles;

    // Check that the database doesn't exist in the database list
    if(!(mongodb_table_model::verifyDatabase(database)))
    {
        _logger->add(_m_type.ACTION, _actions.ADD_DATABASE, database);

        // Add new database to the database list and database count
        _database_list.push_back(database);
        _num_of_databases++;

        // Initialize the roles at the new databse to ROLE_NULL for all the users
        std::vector<QString> new_column;
        for(int i=0; i<_num_of_users; i++)
        {
            _roles_table.at(i).push_back(roles.ROLE_NULL);
        }
    }
    else
    {
        _logger->add(_m_type.ERROR, "The selected database: ", database, " , already exist");
        return false;
    }

    mongodb_table_model::updateTable();
    return true;
}

/**
 * Delete a database from the mongodb_table_model.
 *
 * @param database Name to the database to be deleted.
 * @return True if the document was deleted, false otherwise.
 *
 */

bool mongodb_table_model::deleteDatabase(QString database)
{
    // Check that the database exists in the database list
    if(mongodb_table_model::verifyDatabase(database))
    {
        _logger->add(_m_type.ACTION, _actions.DELETE_DATABASE, database);

        // Remove the selected database from the database list and database count
        int index = _database_list.indexOf(database);
        _database_list.erase(_database_list.begin() + index, _database_list.begin() + index + 1);
        _num_of_databases--;

        // For each row, delete the cell related to the deleted database.
        for(int i=0; i<_num_of_users; i++)
        {
            _roles_table.at(i).erase(_roles_table.at(i).begin() + index, _roles_table.at(i).begin() + index + 1);
        }
    }
    else
    {
        _logger->add(_m_type.ERROR, "The selected database: ", database, " , doesn't exist");
        return false;
    }

    mongodb_table_model::updateTable();
    return true;
}

/**
 * Add new user to the mongodb_table_model.
 *
 * @param  user Name of the user (remember to remove the .admin from the user).
 * @return True if opertation was succesfull, false otherwise.
 *
 */

bool mongodb_table_model::addUser(QString user, QString password)
{
    mongodb_roles roles;

    // Check that the user doesn't exist in the users list
    if(!(mongodb_table_model::verifyUser(user)))
    {
        _logger->add(_m_type.ACTION, _actions.ADD_USER, user, password);

        // Add new user to the users list and user count
        _users_list.push_back(user);
        _num_of_users++;

        // Initialize the roles for the new user to ROLE_NULL for all the databases
        QStringList new_row;
        for(int i=0; i<_num_of_databases; i++)
        {
            new_row.push_back(roles.ROLE_NULL);
        }

        // Add the roles of the new user to the _roles_table
        _roles_table.push_back(new_row);

    }
    else
    {
        _logger->add(_m_type.ERROR, "The selected user: ", user, " , already exists");
        return false;
    }
    mongodb_table_model::updateTable();
    return true;
}

/**
 * Delete selected user from the mongodb_table_model.
 *
 * @param  user Name of the user (remember to remove the .admin from the user).
 * @return True if opertation was succesfull, false otherwise.
 *
 */

bool mongodb_table_model::deleteUser(QString user)
{
    // Check that the user exists in the users list
    if(mongodb_table_model::verifyUser(user))
    {
        _logger->add(_m_type.ACTION, _actions.DELETE_USER, user);

        // Remove the selected user from the users list and user count
        int index = _users_list.indexOf(user);

        _users_list.erase(_users_list.begin() + index, _users_list.begin() + index + 1);
        _num_of_users--;

        // Delete the row related to the deleted user.
        _roles_table.erase(_roles_table.begin() + index, _roles_table.begin() + index + 1);
    }
    else
    {
        _logger->add(_m_type.ERROR, "The selected user: ", user, " , doesn't exist");
        return false;
    }

    mongodb_table_model::updateTable();
    return true;
}

/**
 * Delete selected role off a user from the mongodb_table_model.
 *
 * @param  user Name of the user (remember to remove the .admin from the user).
 * @return True if opertation was succesfull, false otherwise.
 *
 */

bool mongodb_table_model::revokeRoleFromUser(QString user, QString database, QString role)
{
    mongodb_roles roles;
    // Get old role from the table
    int col_num = _database_list.indexOf(database);
    int row_num = _database_list.indexOf(user);
    QString previous_role = _roles_table.at(row_num).at(col_num);

    // If the new_role is ROLE_NULL and the previous_role was not ROLE_NULL, revoke the previous_role
    if(!(previous_role == roles.ROLE_NULL))
    {
        _logger->add(_m_type.ACTION, _actions.REVOKE_ROLE, user, database, previous_role);
    }
    else
    {
        return false;
    }

    // Update the cell in the roles table with the new role:
    _roles_table.at(row_num).replace(col_num,role);

    mongodb_table_model::updateTable();
    return true;
}

/**
 * Grant role to a user from the mongodb_table_model.
 *
 * @param  user User to be granted the role (remember to remove the .admin from the user).
 * @param  database Database in which the role will be granted.
 * @param  role Role to be granted.
 * @return True if opertation was succesfull, false otherwise.
 *
 */

bool mongodb_table_model::grantRoleToUser(QString user, QString database, QString role)
{
    mongodb_roles roles;

    // Get old role from the table
    int col_num = _database_list.indexOf(database);
    int row_num = _users_list.indexOf(user);
    QString previous_role = _roles_table.at(row_num).at(col_num);

    // If the previous_role was not ROLE_NULL, revoke the previous_role and grant the new_role
    if(!(previous_role == roles.ROLE_NULL))
    {
        _logger->add(_m_type.ACTION, _actions.REVOKE_ROLE, user, database, previous_role);
        _logger->add(_m_type.ACTION, _actions.GRANT_ROLE, user, database, role);
    }
    // If the previous_role was ROLE_NULL, grant the new_role
    else
    {
        _logger->add(_m_type.ACTION, _actions.GRANT_ROLE, user, database, role);
    }

    // Update the cell in the roles table with the new role:
    _roles_table.at(row_num).replace(col_num,role);

    mongodb_table_model::updateTable();
    return true;
}

/**
 * Set a custom mongodb_logger different than the one created by default, this is used
 * when the manager is created by other classes and we want both to use the same logger.
 *
 * @param custom_logger
 *
 */

void mongodb_table_model::setCustomLogger(mongodb_logger *custom_logger)
{
    if(_logger_custom == false)
    {
        delete _logger;
        _logger_custom = true;
    }

    if(_logger != custom_logger)
    {
        _logger = custom_logger;

        // Set the same logger for the _utilities class and the manager
        mongodb_manager::setCustomLogger(_logger);
    }
}
