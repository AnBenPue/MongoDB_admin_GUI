#ifndef MONGODB_GUI_CREDENTIALS_DIALOG_H
#define MONGODB_GUI_CREDENTIALS_DIALOG_H

/// \cond
#include <QJsonObject>
#include <QDialog>
#include <QString>
#include <QWidget>
#include <ui_mongodb_credentials_dialog.h>
/// \endcond

#include <mongodb_document.h>

/**
 * @brief DialogBox for inputing the credentials.
 */

class mongodb_gui_credentials_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit mongodb_gui_credentials_dialog(QWidget *parent = 0);
    ~mongodb_gui_credentials_dialog();
    void saveCredentials();
    void clearInputFields();

private:
    Ui::mongodb_credentials_dialog *_ui;
    mongodb_document _credentials;
    QString _default_file_path = QString(SOURCE_PATH) + "/shared/credentials.json";;

};

#endif // MONGODB_GUI_CREDENTIALS_DIALOG_H
