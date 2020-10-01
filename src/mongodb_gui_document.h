#ifndef MONGODB_GUI_H
#define MONGODB_GUI_H

/// \cond
#include <QWidget>
/// \endcond

#include <mongodb_manager.h>
#include <ui_mongodb_gui_document.h>

namespace Ui
{
class mongodb_gui_documents;
}

/**
 * @brief Widget to manage the documents.
 */

class mongodb_gui_documents : public QWidget
{
    Q_OBJECT

public:
    explicit mongodb_gui_documents(QWidget *parent = 0);
    ~mongodb_gui_documents();
    void configureConnection(QString user, QString password, QString database, QString port, QString host);
    void closeEvent(QCloseEvent *event) override;

private:
    void disableButtons();
    void showFileContent();
    void updateCollections();
    void updateDatabases();
    void updateDocumentsLists();
    void initializeGUI();

    void errorMessage(QString message);
    QString requestTextInput(QString title, QString body, QString default_value);

private:
    Ui::mongodb_gui_documents *ui;
    mongodb_manager manager;

    QStringList _collection_list;
    QStringList _database_list;

    QString _selected_database;
    QString _selected_collection;
    QString _error_message;

signals:
    void widgetClosed();
};

#endif // MONGODB_GUI_H
