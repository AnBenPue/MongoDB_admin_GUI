/// \cond
#include <QDebug>
/// \endcond

#include <mongodb_logger.h>

mongodb_logger::mongodb_logger(QObject *parent) : QObject(parent)
{
}

/**
 * Add a new line to the logger. All the fields are empty by default to allow variation in the number of input parameters)
 *
 * @param type Selected logger, INFO,ERROR or ACTION
 * @param field1 Data to be added to the new line
 * @param field2 Data to be added to the new line
 * @param field3 Data to be added to the new line
 * @param field4 Data to be added to the new line
 * @param field5 Data to be added to the new line
 * @param field6 Data to be added to the new line
 * @param field7 Data to be added to the new line
 *
 */

void mongodb_logger::add(QString type,QString field1,QString field2,QString field3,QString field4,QString field5,QString field6,QString field7)
{
    if(type == _m_type.ACTION)
    {
        QStringList action;
        action.push_back(field1);
        action.push_back(field2);
        action.push_back(field3);
        action.push_back(field4);
        action.push_back(field5);
        action.push_back(field6);
        action.push_back(field7);

        _log_actions.push_back(action);
        emit this->ActionslogChanged();
    }
    else
    {
        QString message;
        message.append(type);
        message.append(field1);
        message.append(field2);
        message.append(field3);
        message.append(field4);
        message.append(field5);
        message.append(field6);
        message.append(field7);

        if(type == _m_type.INFO )
        {
            _log_message_info.push_back(message);
        }
        else if(type == _m_type.ERROR)
        {
            _log_message_error.push_back(message);
            emit this->logError();
        }

        _log_message_all.push_back(message);
        emit this->logChanged();
    }
}

/**
 * Get the list of actions stored in the mongodb_logger.
 *
 * @param  container Container for the mongodb_logger actions information
 *
 */

void mongodb_logger::getActionLog(std::vector<QStringList> *container)
{
    container->clear();
    for(QStringList item : _log_actions)
    {
        container->push_back(item);
    }
}

/**
 * Get the list of messages stored in the mongodb_logger.
 *
 * @param type Logger to be selected ERRO, INFO or ALL
 * @param container Container for the mongodb_logger messages information
 *
 */

void mongodb_logger::getMessageLog(QString type, QStringList *container)
{
    container->clear();
    if(type == _m_type.ALL)
    {
        for(QString message : _log_message_all)
        {
            container->push_back(message);
        }
    }
    else if(type == _m_type.ERROR)
    {
        for(QString message : _log_message_error)
        {
            container->push_back(message);
        }
    }
    else if(type == _m_type.INFO)
    {
        for(QString message : _log_message_info)
        {
            container->push_back(message);
        }
    }
}

/**
 * Print the list of messages stored in the mongodb_logger.
 *
 * @param type Logger to be selected ERRO, INFO or ALL
 *
 */

void mongodb_logger::printMessagelog(QString type)
{
    if(type == _m_type.ALL)
    {
        for(QString message : _log_message_all)
        {
            qDebug() << message;
        }
    }
    else if(type == _m_type.ERROR)
    {
        for(QString message : _log_message_error)
        {
            qDebug() << message;
        }
    }
    else if(type == _m_type.INFO)
    {
        for(QString message : _log_message_info)
        {
            qDebug() << message;
        }
    }
    else
    {
        qDebug() << "ERROR: Invalid log type.";
    }
}

/**
 * Delete all the actions stored in the actions log.
 */

void mongodb_logger::clearActionsList()
{
    _log_actions.clear();
    emit this->ActionslogCleared();
}
