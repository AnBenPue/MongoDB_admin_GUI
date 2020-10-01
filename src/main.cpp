#include "mongodb_gui_admin.h"

/// \cond
#include <QApplication>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>

#include <mongocxx/exception/query_exception.hpp>
#include <mongocxx/v_noabi/mongocxx/exception/query_exception.hpp>
/// \endcond

#include <mongodb_table_model.h>
#include <mongodb_manager.h>
#include <mongodb_structures.h>
#include <mongodb_gui_document.h>

/*! \mainpage MongoDB admin management
 *
 * \section installation_requirements Installation requirements:
 *
 * In order to run this software the following driver is required:
 * <a href="http://mongocxx.org/">mongocxx.org</a>
 *
 * In MongoDB, acces control must be enabled:
 * <a href="https://docs.mongodb.com/manual/tutorial/enable-authentication/">https://docs.mongodb.com/manual/tutorial/enable-authentication/</a>
 *
 */


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("Mongodb Manager");

    QCommandLineParser parser;
    parser.setApplicationDescription("Mongodb Manager");
    parser.addHelpOption();
    QCommandLineOption guiOption(QStringList() << "g" << "gui",QCoreApplication::translate("main", "Enable the graphical interface. (GUI requires admin privileges)"));
    parser.addOption(guiOption);
    QCommandLineOption credentialsOption(QStringList() << "c" << "credentials",QCoreApplication::translate("main", "Path to the json file containing the necccessary information to connect to MongoDB.\n The expected format is the following: \n {\n \t\"user\":\"UserAdmin\",\n \t\"password\":\"supersafepassword\",\n \t\"database\":\"admin\",\n \t\"port\":\"27017\",\n \t\"host\":\"localhost\"\n}\n"),QCoreApplication::translate("main", "path/to/file.json"));
    parser.addOption(credentialsOption);
    QCommandLineOption userOption(QStringList() << "u" << "user",QCoreApplication::translate("main", "User to be modified / New user"),QCoreApplication::translate("main", "user"));
    parser.addOption(userOption);
    QCommandLineOption databaseOption(QStringList() << "d" << "database",QCoreApplication::translate("main", "Database to be modified / New database"),QCoreApplication::translate("main", "database"));
    parser.addOption(databaseOption);
    QCommandLineOption passwordOption(QStringList() << "p" << "password",QCoreApplication::translate("main", "Password for the New user"),QCoreApplication::translate("main", "password"));
    parser.addOption(passwordOption);
    QCommandLineOption roleOption(QStringList() << "r" << "role",QCoreApplication::translate("main", "Role to be modified / New role \n Valid roles: \n - read \n - readWrite"),QCoreApplication::translate("main", "role"));
    parser.addOption(roleOption);
    QCommandLineOption actionOption(QStringList() << "a" << "action",QCoreApplication::translate("main", "Action to be performed:\n - addUser: Add a new user to MongoDB,  requires: <user,password> \n - deleteUser: Delete a user from MongoDB, requires arguments: <user> \n - addDatabase: Add a new database to MongoDB, requires: <database> \n - deleteDatabase: Delete a database from MongoDB, requires <database> \n - grantRole: Grant a role to a user in a database, requires: <user,database,role> \n - revokeRole: Revoke a role from a user in a database, requires: <user,database,role> \n"),QCoreApplication::translate("main", "action"));
    parser.addOption(actionOption);

    // Process the actual command line arguments given by the user
    parser.process(a);

    // Insatance of mongoDB
    mongocxx::instance inst{};

    mongodb_actions actions;

    QString user = parser.value(userOption);
    QString database = parser.value(databaseOption);
    QString action = parser.value(actionOption);
    QString role = parser.value(roleOption);
    QString password =parser.value(passwordOption);
    QString path_to_credentials = parser.value(credentialsOption);
    bool use_gui =  parser.isSet(guiOption);

    if(use_gui)
    {
        mongodb_gui_admin window;
        if (window.initializeWindow())
        {
            window.show();
            return a.exec();
        }
    }
    else
    {
        if(!path_to_credentials.isEmpty())
        {
            mongodb_manager manager;
            QVariantMap file;
            try
            {
                file = manager.loadCredentialsFile(path_to_credentials);
            }
            catch(int e)
            {
                // ToDo: Create custom exception instead of using int values.
                if(e==1)
                {
                    qDebug() << "ERROR: Provided credentials path is not a valid path.";
                    return 0;
                }
            }

            if(!file.isEmpty())
            {
                manager.configureConnection(file["user"].toString(),file["password"].toString(),file["database"].toString(),file["port"].toString(),file["host"].toString());

                try
                {
                    if(action == actions.ADD_DATABASE && !database.isEmpty())
                    {
                        manager.addDatabase(database);
                    }
                    else if(action == actions.DELETE_DATABASE && !database.isEmpty())
                    {
                        manager.deleteDatabase(database);
                    }
                    else if(action == actions.ADD_USER && !user.isEmpty() && !password.isEmpty())
                    {
                        manager.addUser(user,password);
                    }
                    else if(action == actions.DELETE_USER && !user.isEmpty())
                    {
                        manager.deleteUser(user);
                    }
                    else if(action == actions.GRANT_ROLE && !user.isEmpty() && !database.isEmpty() && !role.isEmpty())
                    {
                        manager.grantRoleToUser(user,database,role);
                    }
                    else if(action == actions.REVOKE_ROLE && !user.isEmpty() && !database.isEmpty() && !role.isEmpty())
                    {
                        manager.revokeRoleFromUser(user,database,role);
                    }
                    else if(action.isEmpty())
                    {
                        // Do nothing, just show the MongoDB infomation table.
                    }
                    else if((action != actions.REVOKE_ROLE) && (action != actions.GRANT_ROLE) && (action != actions.DELETE_USER) && (action != actions.ADD_USER) && (action != actions.DELETE_DATABASE) && (action != actions.ADD_DATABASE))
                    {
                        qDebug() << "ERROR: Invalid acction. Please check the help manual for valid actions.";
                        return 0;
                    }

                    manager.saveUsersAndRolesTable();
                    qDebug() << "INFO: MongoDB management table:";

                    // Command to run the MongoDB initialization script
                    QString filePath = SOURCE_PATH;
                    std::string table_path = "column -t -s, " + filePath.toStdString() + "/shared/UsersAndRoles.csv";
                    qDebug() << QString::fromStdString(table_path);
                    const char *command = table_path.c_str();

                    int system_Ret = system(command);
                    if(system_Ret != 0)
                    {
                        qDebug() << "ERROR: Failed to access the file " + filePath + "/UsersAndRoles.csv";
                    }
                    else
                    {
                        manager.printlog();
                        return 0;
                    }

                }
                catch(const mongocxx::v_noabi::query_exception& e)
                {
                    if (e.code().category() == mongocxx::server_error_category())
                    {
                        qDebug() << "ERROR: MongoDB service seems not to be running, please run: ";
                        qDebug() << "\t sudo mongod --auth --port 27017 --dbpath /var/lib/mongodb \n";
                        qDebug() << "INFO: If the error persists, please run the GUI version for more infromation about possible causes of this error. (GUI requires admin privileges) ";
                        return 0;
                    }
                }
            }
            else
            {
                qDebug() << "ERROR: Provided credentials file is empty.";
                return 0;
            }
        }
        else
        {
            qDebug() << "ERROR: credentials path argument not found. \n";
            parser.showHelp();
            return 0;
        }
    }
}
