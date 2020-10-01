#ifndef QTABLEMONGODB_H
#define QTABLEMONGODB_H

/// \cond
#include <QStandardItemModel>
#include <QObject>
#include <QString>
#include <QStringList>
/// \endcond

#include <mongodb_logger.h>
#include <mongodb_manager.h>
#include <mongodb_structures.h>

/**
 * @brief Table model based on QStandardItemModel which also inherits from mongodb_manager to have acces to the MongoDB functionalities.
 */

class mongodb_table_model : public QStandardItemModel, public mongodb_manager
{
    Q_OBJECT

public:
    explicit mongodb_table_model(QObject *parent = nullptr);
    mongodb_table_model(int rows, int columns, QObject *parent = nullptr)
        : QStandardItemModel(rows, columns, parent)
    {
    };
    // Table management:
    void initializeModel();
    void initializeTable();
    void updateTable();
    bool isSaved();

    // Actions management:
    void runActions();
    void cancelActions();

    // Database management:
    bool verifyDatabase(QString database) override;
    bool addDatabase(QString database) override;
    bool deleteDatabase(QString database) override;
    int getNumberOfDatabases() override;
    void getDatabaseList(QStringList *database_list);

    // Users management:
    bool verifyUser(QString user) override;
    bool addUser(QString user, QString password) override;
    bool deleteUser(QString user) override;
    int getNumberOfUsers() override;
    void getUsersList(QStringList *users_list);

    // User's roles management:
    bool revokeRoleFromUser(QString user, QString database, QString role) override;
    bool grantRoleToUser(QString user, QString database, QString role) override;

    // Logger:
    void setCustomLogger(mongodb_logger *custom_logger) override;

private:
    // Utilities:
    mongodb_message_types _m_type;
    mongodb_actions _actions;
    mongodb_logger *_logger;

    // Information containers:
    QStringList _database_list;
    QStringList _users_list;
    std::vector<QStringList> _roles_table;

    // Log conainers:
    std::vector<QStringList> _log_actions;

    // General information:
    int _num_of_users;
    int _num_of_databases;
    bool _logger_custom = false;
    QString ADMIN_DB_EXTEND = "admin.";
};

#endif // QTABLEMONGODB_H
