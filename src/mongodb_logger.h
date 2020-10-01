#ifndef MONGODB_LOGGER_H
#define MONGODB_LOGGER_H

/// \cond
#include <QString>
#include <QStringList>
#include <QObject>

#include <vector>
/// \endcond

#include <mongodb_structures.h>

/**
 * @brief Logger for both the messages and actions.
 */

class mongodb_logger: public QObject
{
    Q_OBJECT

private:
    QStringList _log_message_info;
    QStringList _log_message_error;
    QStringList _log_message_all;
    std::vector<QStringList> _log_actions;
    mongodb_message_types _m_type;

public:
    explicit mongodb_logger(QObject *parent = nullptr);

    void add(QString type, QString field1=QString(""), QString field2=QString(""), QString field3=QString(""), QString field4=QString(""), QString field5=QString(""), QString field6=QString(""), QString field7=QString(""));
    void getMessageLog(QString type, QStringList *container);
    void getActionLog(std::vector<QStringList> *container);
    void printMessagelog(QString type);
    void clearActionsList();

signals:
    void logChanged();
    void logError();
    void ActionslogChanged();
    void ActionslogCleared();
};

#endif // MONGODB_LOGGER_H
