#include <mongodb_table_view.h>

mongodb_table_view::mongodb_table_view(QWidget *parent)
    : QTableView(parent)
{
    _logger = new mongodb_logger;
    _model = new mongodb_table_model;
}

/**
 * Initializes the table_view by adding the roles combobox to all the cells.
 */

void mongodb_table_view::initializeTableView()
{
    connect(&_rolesCB, &mongodb_table_roles_delegate::rolesComboBoxSelected,[=]()
    {
        emit this->delegateSelected();

    });

    // Add the roles combobox to all the cells
    for(int i = 0; i < _model->getNumberOfDatabases();i++)
    {
        this->setItemDelegateForColumn(i, &_rolesCB);
    }
}

/**
 * Set a different mongodb_logger to be used for all the messages and actions logging by the mongodb_table_view and the mongodb_table_roles_delegate.
 *
 * @param custom_logger Pointer to the mongodb_logger to be used.
 *
 */

void mongodb_table_view::setCustomLogger(mongodb_logger *custom_logger)
{
    if(_logger_custom == false)
    {
        delete _logger;
        _logger_custom = true;
    }

    if(_logger != custom_logger)
    {
        _logger = custom_logger;
        _rolesCB.setCustomLogger(_logger);
    }
}

/**
 * Set a different mongodb_table_model to be used by the table_view and the roles combobox.
 *
 * @param custom_model Pointer to the mongodb_table_model to be used.
 *
 */

void mongodb_table_view::setCustomModel(mongodb_table_model *custom_model)
{
    if(_model_custom == false)
    {
        delete _model;
        _model_custom = true;
    }

    if(_model != custom_model)
    {        
        _model = custom_model;
        _rolesCB.setCustomModel(_model);
    }
}
