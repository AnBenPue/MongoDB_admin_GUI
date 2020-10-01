#ifndef MONGODB_TABLE_VIEW
#define MONGODB_TABLE_VIEW

/// \cond
#include <QObject>
#include <QTableView>
#include <QWidget>
/// \endcond

#include <mongodb_table_model.h>
#include <mongodb_table_roles_delegate.h>
#include <mongodb_logger.h>

/**
 * @brief Table model based on QTableView. The cells of the table in this view are modified to show a combobox containing the valid roles.
 */

class mongodb_table_view : public QTableView
{
    Q_OBJECT

public:
    explicit mongodb_table_view(QWidget *parent = nullptr);
    void initializeTableView();
    void setCustomLogger(mongodb_logger *custom_logger);
    void setCustomModel(mongodb_table_model *custom_model);

private:
    mongodb_logger *_logger;
    mongodb_table_model *_model;
    mongodb_table_roles_delegate _rolesCB;
    bool _logger_custom = false;
    bool _model_custom = false;

signals:
    void delegateSelected();
};

#endif // MONGODB_TABLE_VIEW
