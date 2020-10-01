/// \cond
#include <QIcon>
/// \endcond

#include <mongodb_gui_credentials_dialog.h>

/**
 * Constructor of the class.
 *
 */

mongodb_gui_credentials_dialog::mongodb_gui_credentials_dialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::mongodb_credentials_dialog)
{
    _ui->setupUi(this);

    QString filePath = SOURCE_PATH;
    filePath.append("/icons/key.svg");
    QIcon key_icon = QIcon(filePath);
    this->setWindowIcon(key_icon);

    connect(_ui->userLineEdit,&QLineEdit::textChanged,[=]()
    {
        _credentials.insertKeyValuePair(QString("user"),_ui->userLineEdit->text());
    });
    connect(_ui->passwordLineEdit,&QLineEdit::textChanged,[=]()
    {
        _credentials.insertKeyValuePair(QString("password"),_ui->passwordLineEdit->text());
    });
    connect(_ui->databaseLineEdit,&QLineEdit::textChanged,[=]()
    {
        _credentials.insertKeyValuePair(QString("database"),_ui->databaseLineEdit->text());
    });
    connect(_ui->portLineEdit,&QLineEdit::textChanged,[=]()
    {
        _credentials.insertKeyValuePair(QString("port"),_ui->portLineEdit->text());
    });
    connect(_ui->hostLineEdit,&QLineEdit::textChanged,[=]()
    {
        _credentials.insertKeyValuePair(QString("host"),_ui->hostLineEdit->text());
    });
    connect(_ui->cancelButton,&QPushButton::clicked,[=]()
    {
        exit(1);
    });
    connect(_ui->openButton,&QPushButton::clicked,[=]()
    {
        mongodb_gui_credentials_dialog::saveCredentials();
        this->hide();
    });

}

/**
 * Destructor of the class.
 *
 */

mongodb_gui_credentials_dialog::~mongodb_gui_credentials_dialog()
{
    delete _ui;
}

/**
 * Save credentials to disk.
 *
 */

void mongodb_gui_credentials_dialog::saveCredentials()
{    
    _credentials.saveToDisk(_default_file_path);
}

/**
 * Clear all the fields in the form.
 *
 */

void mongodb_gui_credentials_dialog::clearInputFields()
{
    _ui->databaseLineEdit->clear();
    _ui->userLineEdit->clear();
    _ui->passwordLineEdit->clear();
    _ui->hostLineEdit->clear();
    _ui->portLineEdit->clear();
}
