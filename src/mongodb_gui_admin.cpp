/// \cond
#include <QTextBrowser>
#include <QFileDialog>
#include <QDebug>
#include <qsplashscreen.h>
#include <QInputDialog>
#include <QMessageBox>
#include <QStringList>

#include <thread>

#include <mongocxx/exception/query_exception.hpp>
#include <mongocxx/exception/logic_error.hpp>
#include <mongocxx/exception/error_code.hpp>

#include <fstream>
/// \endcond

#include <mongodb_gui_credentials_dialog.h>
#include <mongodb_gui_admin.h>
#include <mongodb_gui_document.h>

mongodb_gui_admin::mongodb_gui_admin(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::main_window)
{
    _ui->setupUi(this);
}

mongodb_gui_admin::~mongodb_gui_admin()
{
    delete _ui;
}

/**
 * Initialize window.
 *
 **/
bool mongodb_gui_admin::initializeWindow()
{    
    int system_Ret;
    bool connection_succed = false;
    bool _mongodb_service_ready = false;

    // Command to run the MongoDB initialization script
    std::string script_path = _filePath.toStdString();
    script_path.append("/shared/start_server");
    const char *command = script_path.c_str();

    while(!connection_succed)
    {
        try
        {
            mongodb_gui_admin::loadCredentials();
            _model.setCustomLogger(&_logger);
            _model.configureConnection(_credentials["user"].toString(),_credentials["password"].toString(),_credentials["database"].toString(),_credentials["port"].toString(),_credentials["host"].toString());
            _documents_widget.configureConnection(_credentials["user"].toString(),_credentials["password"].toString(),_credentials["database"].toString(),_credentials["port"].toString(),_credentials["host"].toString());
            _model.initializeModel();
            connection_succed = true;
        }
        catch(const mongocxx::v_noabi::query_exception& e)
        {
            QGuiApplication::restoreOverrideCursor();
            if (e.code().category() == mongocxx::server_error_category())
            {
                if(_mongodb_service_ready)
                {
                    if (e.code().value() == 11) // generic server error, Authentication failed
                    {
                        mongodb_document empty_file;
                        empty_file.saveToDisk(_default_file_path);

                        // Show error box:
                        QMessageBox msgBox;
                        msgBox.setText("Can't connect to MongoDB, please make sure that the credentials are correct.");
                        msgBox.setInformativeText("ERROR: Invalid user, password or database");
                        msgBox.setStandardButtons(QMessageBox::Ok);
                        msgBox.exec();
                    }
                    else if(e.code().value() == 13053) // generic server error, may be thrown by: wrong host, wrong port or no Mongod service running
                    {
                        QString error_message = e.what();
                        QString error_message_substring = error_message.mid(error_message.indexOf('[')+1 ,error_message.indexOf('\'')-error_message.indexOf('[')-1);

                        if(error_message_substring == "Failed to resolve ") // host error
                        {
                            mongodb_document empty_file;
                            empty_file.saveToDisk(_default_file_path);

                            // Show error box:
                            QMessageBox msgBox;
                            msgBox.setText("Can't connect to MongoDB, please make sure that the credentials are correct.");
                            msgBox.setInformativeText("ERROR: Invalid host");
                            msgBox.setStandardButtons(QMessageBox::Ok);
                            msgBox.exec();

                        }
                        else if(error_message_substring == "connection refused calling ismaster on ") // port error
                        {
                            mongodb_document empty_file;
                            empty_file.saveToDisk(_default_file_path);

                            // Show error box:
                            QMessageBox msgBox;
                            msgBox.setText("Can't connect to MongoDB, please make sure that the credentials are correct.");
                            msgBox.setInformativeText("ERROR: Invalid port");
                            msgBox.setStandardButtons(QMessageBox::Ok);
                            msgBox.exec();
                        }
                    }
                }
                else // service missing
                {
                    QPixmap pixmap(_filePath + "/icons/mongodb_splash.svg");
                    QSplashScreen splash(pixmap);
                    splash.show();

                    // Show error box:
                    QMessageBox msgBox;
                    msgBox.setText("Can't connect to MongoDB, please make sure that the service is running.");
                    msgBox.setInformativeText("Would you like to try to connect again?");
                    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
                    msgBox.setDefaultButton(QMessageBox::Ok);

                    int ret = msgBox.exec();
                    switch (ret)
                    {
                    case QMessageBox::Ok:

                        // Try to initalize the MongoDB service
                        qDebug() << "INFO: Initializing the MongoDB service.";
                        splash.showMessage(QSplashScreen::tr("LOADING MONGODB SERVER..."),Qt::AlignBottom | Qt::AlignHCenter, Qt::white);

                        // Start the server:
                        system_Ret = system(command);

                        if(system_Ret != 0)
                        {
                            // Show error box:
                            QMessageBox msgBox;
                            msgBox.setText("Can't connect to MongoDB, failed to launch the script:" + _filePath + "/start_server");
                            msgBox.setInformativeText("ERROR: Failed to connect");
                            msgBox.setStandardButtons(QMessageBox::Ok);
                            msgBox.exec();
                        }

                        std::this_thread::sleep_for(std::chrono::seconds(5));
                        splash.finish(this);
                        _log_list_widget.clear();
                        break;

                    case QMessageBox::Cancel:;
                        return false;
                        break;
                    default:
                        break;
                    }

                    _mongodb_service_ready = true;
                }

            }
        }
        catch(const mongocxx::logic_error& e) // Wrong uri format
        {
            if (e.code() == mongocxx::error_code::k_invalid_uri)
            {
                mongodb_document empty_file;
                empty_file.saveToDisk(_default_file_path);

                // Show error box:
                QMessageBox msgBox;
                msgBox.setText("Can't connect to MongoDB, please make sure that the credentials are correct.");
                msgBox.setInformativeText("ERROR: Invalid connection uri");
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.exec();
            }
        }
    }

    this->addDockWidget(Qt::BottomDockWidgetArea, &_logger_widget);
    this->addDockWidget(Qt::RightDockWidgetArea, &_help_widget);
    _logger_widget.hide();
    _help_widget.hide();

    _ui->users_table->setModel(&_model);
    _ui->users_table->setCustomModel(&_model);
    _ui->users_table->setCustomLogger(&_logger);
    _ui->users_table->initializeTableView();

    mongodb_gui_admin::setToolbar();
    mongodb_gui_admin::setIcons();
    mongodb_gui_admin::setConnections();

    return true;
}

/**
 * Load the required credentials to log in MongoDB.
 *
 **/
void mongodb_gui_admin::loadCredentials()
{
    mongodb_gui_credentials_dialog credentials_request;
    bool file_found = false;
    bool file_valid = false;
    _credentials.clear();

    int MAX_ATTEMPT = 5;
    for(int i=0; i < MAX_ATTEMPT; i++)
    {
        if(file_found == false || file_valid == false)
        {
            // Get credentials from the default path
            std::ifstream infile(_default_file_path.toStdString());
            if(infile.good())
            {
                _logger.add(_m_type.INFO,"Credentials file found in the default path: ", _default_file_path);
                file_found = true;

                // Load the file
                _credentials = _model.loadCredentialsFile(_default_file_path);

                // Check if the file is valid
                if(!_credentials.isEmpty())
                {
                    _logger.add(_m_type.INFO,"Credentials file is a valid file");
                    file_valid = true;
                }
                else
                {
                    _logger.add(_m_type.ERROR,"Credentials file is not a valid file");
                    file_valid = false;
                    credentials_request.exec();
                    credentials_request.clearInputFields();
                }
            }
            else
            {
                _logger.add(_m_type.INFO,"Credentials file not found, please introduce the required fields.");
                credentials_request.exec();
                credentials_request.clearInputFields();
            }
        }
        else
        {
            break;
        }

        credentials_request.close();
    }
}

/**
 * Set toolbar for the window.
 *
 **/
void mongodb_gui_admin::setToolbar()
{
    // Available buttons
    QToolButton *_toolButton_add_db = new QToolButton();
    QToolButton *_toolButton_delete_db = new QToolButton();
    QToolButton *_toolButton_add_user = new QToolButton();
    QToolButton *_toolButton_delete_user = new QToolButton();
    QToolButton *_toolButton_save = new QToolButton();
    QToolButton *_toolButton_reload = new QToolButton();

    // Move toolbar to the left
    removeToolBar(_ui->toolBar);
    addToolBar(_ui->toolBar);
    _ui->toolBar->show();

    // Set the icons and the text
    _toolButton_add_db->setIcon(QIcon(_filePath + "/icons/database_add.svg"));
    _toolButton_delete_db->setIcon(QIcon(_filePath + "/icons/database_del.svg"));
    _toolButton_add_user->setIcon(QIcon(_filePath + "/icons/user_add.svg"));
    _toolButton_delete_user->setIcon(QIcon(_filePath + "/icons/user_del.svg"));
    _toolButton_save->setIcon(QIcon(_filePath + "/icons/save.svg"));
    _toolButton_reload->setIcon(QIcon(_filePath + "/icons/reload.svg"));

    _toolButton_add_db->setToolTip(QString("Add database"));
    _toolButton_delete_db->setToolTip(QString("Delete database"));
    _toolButton_add_user->setToolTip(QString("Add user"));
    _toolButton_delete_user->setToolTip(QString("Delete user"));
    _toolButton_save->setToolTip(QString("Save"));
    _toolButton_reload->setToolTip(QString("Reload"));

    // Add widget
    _ui->toolBar->addWidget(_toolButton_add_db);
    _ui->toolBar->addWidget(_toolButton_delete_db);
    _ui->toolBar->addWidget(_toolButton_add_user);
    _ui->toolBar->addWidget(_toolButton_delete_user);
    _ui->toolBar->addWidget(_toolButton_save);
    _ui->toolBar->addWidget(_toolButton_reload);

    _toolButton_save->setEnabled(false);
    _toolButton_reload->setEnabled(false);

    // Connect toolbuttons with actions
    connect(_toolButton_add_db,&QToolButton::clicked,_ui->actionAddDb,&QAction::triggered);
    connect(_toolButton_delete_db,&QToolButton::clicked,_ui->actionDeleteDb,&QAction::triggered);
    connect(_toolButton_add_user,&QToolButton::clicked,_ui->actionAddUser,&QAction::triggered);
    connect(_toolButton_delete_user,&QToolButton::clicked,_ui->actionDeleteUser,&QAction::triggered);
    connect(_toolButton_save,&QToolButton::clicked,_ui->actionSave,&QAction::triggered);
    connect(_toolButton_reload,&QToolButton::clicked,_ui->actionReload,&QAction::triggered);

    connect(&_logger,&mongodb_logger::ActionslogChanged,[=]()
    {
        this->setWindowModified(true);
        _toolButton_save->setEnabled(true);
        _toolButton_reload->setEnabled(true);
    });

    connect(_ui->users_table,&mongodb_table_view::delegateSelected,[=]()
    {
        _toolButton_save->setEnabled(true);
        this->setWindowModified(true);
        _toolButton_reload->setEnabled(true);
    });

    connect(&_logger,&mongodb_logger::ActionslogCleared,[=]()
    {
        this->setWindowModified(false);
        _toolButton_save->setEnabled(false);
        _toolButton_reload->setEnabled(false);
    });

    mongodb_gui_admin::deselectTable();
}

/**
 * Set icons for the window.
 *
 **/
void mongodb_gui_admin::setIcons()
{
    QIcon saveicon = QIcon(_filePath + "/icons/save.svg");
    QIcon reloadicon = QIcon(_filePath + "/icons/reload.svg");
    QIcon fileicon = QIcon(_filePath + "/icons/file.svg");
    QIcon credentialsicon = QIcon(_filePath + "/icons/key.svg");
    QIcon usericon = QIcon(_filePath + "/icons/user.svg");
    QIcon dbicon = QIcon(_filePath + "/icons/database.svg");
    QIcon plusicon = QIcon(_filePath + "/icons/plus.svg");
    QIcon minusicon = QIcon(_filePath + "/icons/minus.svg");
    QIcon helpicon = QIcon(_filePath + "/icons/help.svg");
    QIcon loggericon = QIcon(_filePath + "/icons/logger.svg");
    QIcon windowicon = QIcon(_filePath + "/icons/mongodb_logo.svg");
    QIcon documenticon = QIcon(_filePath + "/icons/documents.svg");

    this->setWindowIcon(windowicon);

    _ui->actionSave->setIcon(saveicon);
    _ui->actionReload->setIcon(reloadicon);
    _ui->actionCredentials->setIcon(credentialsicon);
    _ui->menuDatabase->setIcon(dbicon);
    _ui->menuUser->setIcon(usericon);
    _ui->actionAddUser->setIcon(plusicon);
    _ui->actionAddDb->setIcon(plusicon);
    _ui->actionDeleteUser->setIcon(minusicon);
    _ui->actionDeleteDb->setIcon(minusicon);
    _ui->actionShow_help->setIcon(helpicon);
    _ui->actionShow_logger->setIcon(loggericon);
    _ui->actionDocuments->setIcon(documenticon);
}

/**
 * Deselect the roles table.
 *
 **/
void mongodb_gui_admin::deselectTable()
{
    _ui->users_table->clearSelection(); // Deselect the roles table (not the combo box)
    _ui->users_table->setEditTriggers(QAbstractItemView::NoEditTriggers); // In this way a new selection doesn't show the combo box
    _ui->users_table->selectColumn(0); // Select column 0. This moves to index 0,0
    QModelIndex in = _ui->users_table->currentIndex(); // Save the index as a QModelIndex
    _ui->users_table->setCurrentIndex(in.sibling(in.row() - 1,in.column() - 1)); // Move to index -1,-1
    _ui->users_table->clearSelection(); // Deselect the roles table (not the combo box)
    _ui->users_table->setEditTriggers(QAbstractItemView::AllEditTriggers); // Enable again the selection that show tha combo box
}

/**
 * Initialize the connections for the window.
 *
 **/
void mongodb_gui_admin::setConnections()
{
    QCheckBox *check_info = new QCheckBox(QString("Show INFO"));

    connect(check_info,&QCheckBox::clicked,[=]()
    {
        _logger.logChanged();
    });

    connect(&_logger,&mongodb_logger::logError,[=]()
    {
        // Show logger if there is an error
        _ui->actionShow_logger->triggered(true);
    });

    connect(&_logger,&mongodb_logger::logChanged,[=]()
    {
        QStringList log_messages;

        if( check_info->isChecked() ) // If check info, add both info and errors in the logger
        {
            _logger.getMessageLog(_m_type.ALL,&log_messages);
        }
        else
        {
            _logger.getMessageLog(_m_type.ERROR,&log_messages);
        }

        _log_list_widget.clear();
        for(QString message : log_messages)
        {
            _log_list_widget.addItem(message);
            // Get the error type to print the line in red color in case it is an error.
            QString message_type = message.left(message.indexOf(':')+2);

            if(message_type == _m_type.ERROR)
            {
                int line =_log_list_widget.count() - 1; // Minus one because the index is one less than the size
                _log_list_widget.item(line)->setForeground(Qt::red);
            }
        }

        _log_list_widget.scrollToBottom();
    });

    connect(_ui->actionSave,&QAction::triggered,[=]()
    {
        deselectTable();
        _model.runActions();
    });

    connect(_ui->actionReload,&QAction::triggered,[=]()
    {
        deselectTable();

        if(!(_model.isSaved()))
        {
            QMessageBox msgBox;
            msgBox.setText("Are you sure you want to reload?");
            msgBox.setInformativeText("This will delete all the unsaved changes.");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            int ret = msgBox.exec();

            switch (ret)
            {
            case QMessageBox::Yes:
                _model.cancelActions();
                break;
            case QMessageBox::Cancel:
                break;
            default:
                break;
            }
        }

    });

    connect(_ui->actionDocuments,&QAction::triggered,[=]()
    {
        _documents_widget.show();
        hide();
    });


    connect(_ui->actionCredentials,&QAction::triggered,[=]()
    {
        QString file_path = QFileDialog::getOpenFileName(this,"Select:");
        if (file_path.isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText("Invalid file.");
            msgBox.setInformativeText("The path of the file is empty.");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();
        }
        else if(!(file_path.right(file_path.size() - file_path.indexOf('.')) == ".json"))
        {
            QMessageBox msgBox;
            msgBox.setText("Invalid file.");
            msgBox.setInformativeText("The selected file is not a .json file.");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();
        }
        else
        {
            mongodb_document doc;
            doc.loadFromDisk(file_path);
            doc.saveToDisk(_default_file_path);
        }

    });

    connect(_ui->actionAddDb, &QAction::triggered,[=]()
    {
        bool ok_1;
        QString database = QInputDialog::getText(this, "Input new database name", "Name:", QLineEdit::Normal, "New_database", &ok_1);
        if(ok_1 && !database.isEmpty() && !(database.indexOf(' ') >= 0))
        {
            _model.addDatabase(database);
            _ui->users_table->initializeTableView();
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText("Please insert a valid database name.");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            int ret = msgBox.exec();
            switch (ret)
            {
            case QMessageBox::Ok:
                break;
            default:
                break;
            }
        }

    });

    connect(_ui->actionDeleteDb, &QAction::triggered,[=]()
    {
        QStringList database_list;

        _model.getDatabaseList(&database_list);

        if(_model.getNumberOfDatabases() != 0)
        {
            bool ok;
            QString database = QInputDialog::getItem(this, tr("Select database:"),tr("database:"), database_list, 0, false, &ok);
            if (ok && !database.isEmpty())
            {
                _model.deleteDatabase(database);
                _ui->users_table->initializeTableView();
            }
        }
    });

    connect(_ui->actionAddUser, &QAction::triggered,[=]()
    {
        bool ok_1;
        bool ok_2;
        QString username = QInputDialog::getText(this, "Input new user name", "Name:", QLineEdit::Normal, "New_user", &ok_1);
        if(ok_1 && !username.isEmpty() && !(username.indexOf(' ') >= 0))
        {
            QString password = QInputDialog::getText(this, "Input new password", "Password:", QLineEdit::Normal, "New_password", &ok_2);
            if(ok_2 && !password.isEmpty() && !(password.indexOf(' ') >= 0))
            {
                _model.addUser(username,password);
            }
            else
            {
                QMessageBox msgBox;
                msgBox.setText("Please insert a valid password.");
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setDefaultButton(QMessageBox::Ok);
                int ret = msgBox.exec();
                switch (ret)
                {
                case QMessageBox::Ok:
                    break;
                default:
                    break;
                }
            }
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText("Please insert a valid Username.");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            int ret = msgBox.exec();
            switch (ret)
            {
            case QMessageBox::Ok:
                break;
            default:
                break;
            }
        }
    });

    connect(_ui->actionDeleteUser, &QAction::triggered,[=]()
    {
        QStringList users_list;
        _model.getUsersList(&users_list);

        if(_model.getNumberOfUsers() != 0)
        {
            bool ok;
            QString user = QInputDialog::getItem(this, tr("Select user:"), tr("user:"), users_list, 0, false, &ok);
            if (ok && !user.isEmpty())
            {
                _model.deleteUser(user);
            }
        }
    });

    connect(_ui->actionShow_help, &QAction::triggered,[=]()
    {
        QString filePath = SOURCE_PATH;
        QTextBrowser *tb = new QTextBrowser(&_help_widget);
        QString html_path = filePath + "/shared/help.html";

        //load the file and convert its contents into a QbyteArray
        QFile file(html_path);
        if(!file.open(QIODevice::ReadOnly))
        {
            _logger.add(_m_type.INFO,"Failed to open help file: ",html_path);
            throw(1);
        }

        QTextStream file_text(&file);
        QString output = file_text.readAll();
        file.close();

        tb->setHtml(output);
        _help_widget.setWidget(tb);
        _help_widget.setAllowedAreas(Qt::RightDockWidgetArea);
        _help_widget.setFeatures(QDockWidget::NoDockWidgetFeatures);
        _help_widget.setFeatures(QDockWidget::DockWidgetClosable);
        _help_widget.show();

    });

    connect(_ui->actionShow_logger, &QAction::triggered,[=]()
    {
        QWidget *multiWidget = new QWidget(); // This will contain the list Widget and c_boxes widget in an horizontal layout

        // Initialize layouts
        QVBoxLayout *v_layout = new QVBoxLayout();

        // Set initial state of the checkbox as selected
        check_info->setCheckState(Qt::CheckState::Unchecked);

        // Add widgets and set layouts
        v_layout->addWidget(&_log_list_widget);
        v_layout->addWidget(check_info);

        multiWidget->setLayout(v_layout);
        _logger_widget.setWidget(multiWidget);

        _logger_widget.setAllowedAreas(Qt::BottomDockWidgetArea);
        _logger_widget.setFeatures(QDockWidget::NoDockWidgetFeatures);
        _logger_widget.setFeatures(QDockWidget::DockWidgetClosable);

        _logger_widget.show();
        _logger.logChanged(); // To show all the messages of the logger

    });

    connect(&_documents_widget,&mongodb_gui_documents::widgetClosed, [=]()
    {
        _model.initializeModel();
        show();
    });
}

/**
 * Close event handler.
 *
 **/
void mongodb_gui_admin::closeEvent(QCloseEvent *pressX)
{
    deselectTable();
    if(!(_model.isSaved()))
    {
        QMessageBox msgBox;
        msgBox.setText("Are you sure you want to close without saving?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Close | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();

        switch (ret)
        {
        case QMessageBox::Save:
            _logger.printMessagelog(_m_type.ACTION);
            _model.runActions();
            pressX->accept();
            break;
        case QMessageBox::Close:
            pressX->accept();
            break;
        case QMessageBox::Cancel:
            pressX->ignore();
            break;
        default:
            break;
        }
    }
    else
    {
        pressX->accept();
    }
}
