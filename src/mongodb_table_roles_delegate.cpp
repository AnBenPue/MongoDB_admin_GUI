/// \cond
#include <QComboBox>
/// \endcond

#include <mongodb_table_roles_delegate.h>
#include <mongodb_structures.h>

mongodb_table_roles_delegate::mongodb_table_roles_delegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    _model = new mongodb_table_model;
    _logger = new mongodb_logger;
}

mongodb_table_roles_delegate::~mongodb_table_roles_delegate()
{
}

QWidget *mongodb_table_roles_delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Create the combobox and populate it
    QComboBox *cb = new QComboBox(parent);
    cb->addItem(QString(_roles.ROLE_NULL));
    cb->addItem(QString(_roles.ROLE_R));
    cb->addItem(QString(_roles.ROLE_RW));
    return cb;
}

void mongodb_table_roles_delegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(cb);
    // Get the index of the text in the combobox that matches the current value of the item
    const QString currentText = index.data(Qt::EditRole).toString();
    const int cbIndex = cb->findText(currentText);
    // If it is valid, adjust the combobox
    if (cbIndex >= 0){
        cb->setCurrentIndex(cbIndex);
    }
    emit this->rolesComboBoxSelected();
}

void mongodb_table_roles_delegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
    Q_ASSERT(cb);
    model->setData(index, cb->currentText(), Qt::EditRole);

    // Get role to update
    QString role = cb->currentText();

    // Get database name
    QVariant temp = _model->headerData(index.column(), Qt::Horizontal);
    QString database = temp.toString();

    // Get user mane
    temp = _model->headerData(index.row(), Qt::Vertical);
    QString username = temp.toString();

    // Update the users's role
    if(role == _roles.ROLE_NULL)
    {
        _model->revokeRoleFromUser(username, database, role);
    }
    else
    {
        _model->grantRoleToUser(username, database, role);
    }
}

/**
 * Set a different mongodb_table_model to be used by the mongodb_table_roles_delegate.
 *
 * @param custom_model Pointer to the mongodb_table_model to be used.
 *
 */

void mongodb_table_roles_delegate::setCustomModel(mongodb_table_model *custom_model)
{
    if(_logger_custom == false)
    {
        delete _logger;
        _logger_custom = true;
    }

    if(_model != custom_model)
    {
        _model = custom_model;
    }
}

/**
 * Set a different mongodb_logger to be used for all the messages and actions logging by the mongodb_table_roles_delegate.
 *
 * @param custom_logger Pointer to the mongodb_logger to be used.
 *
 */

void mongodb_table_roles_delegate::setCustomLogger(mongodb_logger *custom_logger)
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
