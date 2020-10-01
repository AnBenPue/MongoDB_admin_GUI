#ifndef MONGODB_STRUCTURES_H
#define MONGODB_STRUCTURES_H

/// \cond
#include <QString>
/// \endcond
/**
 * @brief Possible acctions to perform in MongoDB.
 */

struct mongodb_actions
{
    QString ADD_USER = "addUser";
    QString DELETE_USER = "deleteUser";
    QString REVOKE_ROLE ="revokeRole";
    QString GRANT_ROLE = "grantRole";
    QString ADD_DATABASE = "addDatabase";
    QString DELETE_DATABASE = "deleteDatabase";
};

/**
 * @brief Valid roles
 */

struct mongodb_roles
{
    QString ROLE_R = "read";  /**< enum value role_r */
    QString ROLE_RW = "readWrite";
    QString ROLE_NULL = " - ";
};

/**
 * @brief Message types
*/

struct mongodb_message_types
{
    QString ERROR = "ERROR: ";
    QString INFO = "INFO: ";
    QString ACTION = "ACTION: ";
    QString ALL = "ALL";
};

#endif // MONGODB_STRUCTURES_H
