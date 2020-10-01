/// \cond
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>
#include <QPushButton>

#include <mongocxx/v_noabi/mongocxx/exception/query_exception.hpp>
/// \endcond

#include <mongodb_gui_document.h>
#include <mongodb_document.h>

mongodb_gui_documents::mongodb_gui_documents(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::mongodb_gui_documents)
{

}

mongodb_gui_documents::~mongodb_gui_documents()
{
    delete ui;
}

void mongodb_gui_documents::updateCollections()
{

    // Set cursor appearance to wait
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    // Update GUI appearance
    ui->collection_comboBox->clear();

    try
    {
        manager.getCollectionList(_selected_database,&_collection_list);
    }
    catch (const mongocxx::v_noabi::query_exception& e)
    {
        if(e.code().category() == mongocxx::server_error_category())
        {
            QGuiApplication::restoreOverrideCursor();
            _error_message.append("SERVER ERROR: ");
            _error_message.append(e.what());
            errorMessage(_error_message);
        }
        QGuiApplication::restoreOverrideCursor();
        return;
    }

    for(QString & element : _collection_list)
    {
        ui->collection_comboBox->addItem(element);
    }

    // restore cursor appearance
    QGuiApplication::restoreOverrideCursor();
}

QString mongodb_gui_documents::requestTextInput(QString title, QString body, QString default_value)
{
    bool ok;
    QString input = QInputDialog::getText(this, title,body, QLineEdit::Normal,default_value, &ok);

    if(ok && !input.isEmpty())
    {
        return input;
    }
    else
    {
        _error_message.append("SERVER ERROR: Please introduce a valid name for the collection.");
        errorMessage(_error_message);
        return "invalid_input";
    }
}

void mongodb_gui_documents::showFileContent()
{
    mongodb_document selectedJsonDocument;

    // Set cursor appearance to wait
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // Depending on which collection is selected, the method for reading the file is different
    if(_selected_collection == "fs.files")
    {
        // Update GUI values:
        // Get the json file containing the metadata:
        mongodb_document selectedJsonDocument(manager.getDocument(ui->documentList->currentItem()->text()));
        // Update GUI appearance:
        ui->fileContentTextBox->setPlainText(selectedJsonDocument.toQString());

        // Get the real content of the file using GridFS
        selectedJsonDocument = manager.getDocumentGridFS(ui->documentList->currentItem()->text());
    }
    else
    {
        selectedJsonDocument = manager.getDocument(ui->documentList->currentItem()->text());

        // Update GUI appearance:
        ui->fileContentTextBox->setPlainText(selectedJsonDocument.toQString());
    }

    // restore cursor appearance
    QGuiApplication::restoreOverrideCursor();
}

void mongodb_gui_documents::updateDocumentsLists()
{
    std::vector<mongodb_document> document_list;
    QStringList id_list;

    // set cursor appearance to wait
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // Update GUI values:
    manager.getDocumentList(&id_list,&document_list);

    // Update GUI appearance:
    ui->documentList->clear();
    ui->fileContentTextBox->clear();

    for(QString & element : id_list)
    {
        ui->documentList->addItem(element);
    }

    // restore cursor appearance
    QGuiApplication::restoreOverrideCursor();
}

void mongodb_gui_documents::closeEvent(QCloseEvent *event)
{
    emit widgetClosed();
    event->accept();
}

void mongodb_gui_documents::initializeGUI()
{
    ui->setupUi(this);
    ui->collection_comboBox->setEnabled(false);
    ui->selectButton->setEnabled(false);
    disableButtons();

    connect(ui->database_comboBox, &QComboBox::currentTextChanged, [=]()
    {
        // Update GUI appearance:
        disableButtons();

        ui->collection_comboBox->setEnabled(true);

        // Update GUI values:
        this -> _selected_database = ui->database_comboBox->currentText();
        if(_selected_database != ""){
            updateCollections();
        }
    });

    connect(ui->collection_comboBox, &QComboBox::currentTextChanged, [=]()
    {
        // Update GUI appearance:
        disableButtons();
        ui->selectButton->setEnabled(true);

        // Update GUI values:
        _selected_collection = ui->collection_comboBox->currentText();

    });

    connect(ui->selectButton, &QPushButton::clicked, [=]()
    {
        // Establish connection to selected database/collection
        manager.connectToCollection(_selected_database, _selected_collection);

        // Update GUI appearance:
        ui->uploadButton->setEnabled(true);
        ui->exportButton->setEnabled(true);
        ui->addCollectionButton->setEnabled(true);
        ui->deleteCollectionButton->setEnabled(true);
        ui->addDatabaseButton->setEnabled(true);
        ui->deleteDatabaseButton->setEnabled(true);
        ui->deleteButton->setEnabled(false);
        ui->downloadButton->setEnabled(false);
        updateDocumentsLists();
    });

    connect(ui->documentList,&QListWidget::doubleClicked,[=]()
    {
        // Ensure that the connection is correct:
        manager.connectToCollection(_selected_database, _selected_collection);

        // Update GUI appearance
        ui->deleteButton->setEnabled(true);
        ui->downloadButton->setEnabled(true);
        showFileContent();

    });

    connect(ui->downloadButton, &QPushButton::clicked, [=]()
    {
        // Ensure that the connection is correct:
        manager.connectToCollection(_selected_database,_selected_collection);

        QString filename = QFileDialog::getSaveFileName(this,"Save as");

        // Save Json into a file:
        manager.exportDocument(ui->documentList->currentItem()->text(), filename);

        // Update GUI appearance:
        ui->deleteButton->setEnabled(false);
        ui->downloadButton->setEnabled(false);
        mongodb_gui_documents::updateDocumentsLists();

    });

    connect(ui->deleteButton, &QPushButton::clicked, [=]()
    {
        if(ui->documentList->currentItem()!= NULL)
        {
            // Update GUI values:
            manager.deleteDocument(ui->documentList->currentItem()->text());

            // Update GUI appearance
            mongodb_gui_documents::updateDocumentsLists();
            ui->deleteButton->setEnabled(false);
            ui->downloadButton->setEnabled(false);
        }
        else
        {
            _error_message.append("ERROR: Selection not valid");
            errorMessage(_error_message);
        }
    });

    connect(ui->uploadButton, &QPushButton::clicked, [=]()
    {
        // Ensure that the connection is correct:
        manager.connectToCollection(_selected_database, _selected_collection);

        // Update GUI values:
        QString file_path = QFileDialog::getOpenFileName(this, "Open File");

        // set cursor appearance to wait
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        manager.importDocument(file_path);

        // restore cursor appearance
        QGuiApplication::restoreOverrideCursor();

        // Update GUI appearance:
        updateDocumentsLists();
        ui->deleteButton->setEnabled(false);
        ui->downloadButton->setEnabled(false);

    });

    connect(ui->exportButton, &QPushButton::clicked, [=]()
    {
        QString filename = QFileDialog::getSaveFileName(this,"Save collection as");
        manager.downloadCollection(filename);

    });

    connect(ui->addDatabaseButton, &QPushButton::clicked, [=]()
    {
        bool ok;
        QString new_database_name = QInputDialog::getText(this, tr("Input new database name"),tr("Name:"), QLineEdit::Normal,"New_database", &ok);
        if (ok && !new_database_name.isEmpty())
        {
            try
            {
                manager.addDatabase(new_database_name);

            }
            catch (const mongocxx::v_noabi::operation_exception& e)
            {
                if (e.code().category() == mongocxx::server_error_category())
                {
                    _error_message.append("SERVER ERROR: ");
                    _error_message.append(e.what());
                    _error_message.append(" Inserted name: ");
                    _error_message.append(new_database_name);
                    errorMessage(_error_message);
                }
            }

        }
        else
        {
            _error_message.append("SERVER ERROR: Please introduce a valid name for the database.");
            errorMessage(_error_message);
        }
        // Update GUI appearance:
        mongodb_gui_documents::updateDatabases();
    });

    connect(ui->deleteDatabaseButton, &QPushButton::clicked, [=](){

        try
        {
            manager.deleteDatabase(_selected_database);
        }
        catch (const mongocxx::v_noabi::operation_exception& e)
        {
            if(e.code().category() == mongocxx::server_error_category())
            {
                _error_message.append("SERVER ERROR: ");
                _error_message.append(e.what());
                errorMessage(_error_message);
            }
        }

        // Update GUI appearance:
        mongodb_gui_documents::updateDatabases();
        mongodb_gui_documents::updateCollections();
        mongodb_gui_documents::disableButtons();
    });

    connect(ui->addCollectionButton, &QPushButton::clicked, [=]()
    {
        QString new_collection_name = mongodb_gui_documents::requestTextInput("Input new collection name","Name:","New_collection");

        if (new_collection_name != "invalid_input")
        {
            try
            {
                manager.addCollection(_selected_database, new_collection_name);
            }
            catch (const mongocxx::v_noabi::operation_exception& e)
            {
                if (e.code().category() == mongocxx::server_error_category())
                {
                    _error_message.append("SERVER ERROR: ");
                    _error_message.append(e.what());
                    errorMessage(_error_message);
                }
            }
        }

        // Update GUI:
        mongodb_gui_documents::updateDatabases();
        mongodb_gui_documents::updateCollections();
        mongodb_gui_documents::disableButtons();

    });

    connect(ui->deleteCollectionButton, &QPushButton::clicked, [=]()
    {
        manager.deleteCollection(_selected_database, _selected_collection);

        // Update GUI:
        mongodb_gui_documents::updateDatabases();
        mongodb_gui_documents::updateCollections();
        mongodb_gui_documents::disableButtons();
    });
}

void mongodb_gui_documents::errorMessage(QString message)
{
    QMessageBox messageBox;
    messageBox.critical(0,"Error",message);
    messageBox.setFixedSize(500,200);
    _error_message.clear();
}

void mongodb_gui_documents::disableButtons()
{
    ui->DocumentManagementControlers->setEnabled(false);
    ui->CollectionManagementControlers->setEnabled(false);
    ui->databaseManagementControlers->setEnabled(false);

    ui->exportButton->setEnabled(false);

    ui->documentList->clear();
    ui->fileContentTextBox->clear();
}

void mongodb_gui_documents::configureConnection(QString user, QString password, QString database, QString port, QString host)
{
    // Pass the neccessary information for the connexion to the manager.
    manager.configureConnection(user,password,database,port,host);

    // Initialize the GUI.
    initializeGUI();

    // Update the databases
    updateDatabases();
}

void mongodb_gui_documents::updateDatabases()
{
    // Set cursor appearance to wait
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // Update GUI values:
    ui->database_comboBox->clear();
    manager.getDatabaseList(&_database_list);

    // Update GUI appearance:
    for (QString & element : _database_list)
    {
        ui->database_comboBox->addItem(element);
    }

    // restore cursor appearance
    QGuiApplication::restoreOverrideCursor();
}




