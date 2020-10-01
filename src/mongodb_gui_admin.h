#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <mongodb_structures.h>
#include <mongodb_table_model.h>
#include <mongodb_logger.h>
#include <mongodb_gui_document.h>

/// \cond
#include <QMainWindow>
#include <QWidget>
#include <QListWidget>
#include <QDockWidget>
#include <QString>

#include <qcheckbox.h>
#include <qtoolbutton.h>
#include <QCloseEvent>

#include <vector>

#include <ui_mainwindow.h>
/// \endcond

#include <QDir>

/**
 * @brief Windom to manage the roles, databases and users.
 */

class mongodb_gui_admin : public QMainWindow
{
    Q_OBJECT

public:
    explicit mongodb_gui_admin(QWidget *parent = 0);
    void loadCredentials();
    void setConnections();
    void setIcons();
    void setToolbar();
    void deselectTable();
    bool initializeWindow();
    void closeEvent(QCloseEvent *pressX);

    ~mongodb_gui_admin();

private:
    QVariantMap _credentials;

    QString _filePath = SOURCE_PATH;
    QString _default_file_path = _filePath + "/shared/credentials.json";

    mongodb_logger _logger;
    mongodb_table_model _model;

    Ui::main_window *_ui;
    mongodb_message_types _m_type;

    QListWidget _log_list_widget;
    QDockWidget _logger_widget;
    QDockWidget _help_widget;
    mongodb_gui_documents _documents_widget;

};

#endif // MAINWINDOW_H
