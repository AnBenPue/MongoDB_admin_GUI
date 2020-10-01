#ifndef MONGODB_TABLE_ROLES_DELEGATE_H
#define MONGODB_TABLE_ROLES_DELEGATE_H

/// \cond
#include <QStyledItemDelegate>
#include <QObject>
#include <QWidget>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QAbstractItemModel>
/// \endcond

#include <mongodb_table_model.h>
#include <mongodb_logger.h>

/**
 * @brief Item delegate based on QStyledItemDelegate. The delegate shows a combobox with the valid roles.
 */

class mongodb_table_roles_delegate : public QStyledItemDelegate
{
    Q_OBJECT

private:
    mongodb_table_model *_model;
    mongodb_logger *_logger;
    mongodb_roles _roles;

    bool _logger_custom = false;
    bool _model_custom = false;

public:
    mongodb_table_roles_delegate(QObject *parent = nullptr);
    ~mongodb_table_roles_delegate();
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;    
    void setCustomModel(mongodb_table_model *custom_model);
    void setCustomLogger(mongodb_logger *custom_logger);

signals:
    void rolesComboBoxSelected() const;
};

#endif // MONGODB_TABLE_ROLES_DELEGATE_H
