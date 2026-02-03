#include "notepad.h"
#include <QVBoxLayout>
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDir>

Notepad::Notepad(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Markdown Editor");
    resize(800, 600);
    initUI(); 
}

Notepad::~Notepad()
{

}

void Notepad::initUI()
{
    initTabWidget();
    initTextEdit();
    initToolbar();

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(centralWidget);
    contentLayout->addWidget(m_textEdit);
    setCentralWidget(centralWidget);
}

void Notepad::initTabWidget()
{

}

void Notepad::initTextEdit()
{
    m_textEdit = new QPlainTextEdit();
    m_textEdit->setPlaceholderText("Enter your markdown text here...");
}

void Notepad::initToolbar()
{
    QToolBar *toolbar = addToolBar("Main Toolbar");

    QAction* newAction = new QAction("New", this);
    QAction* openAction = new QAction("Open", this);
    QAction* saveAction = new QAction("Save", this);
    QAction* saveAsAction = new QAction("Save As", this);
    toolbar->addAction(newAction);
    toolbar->addAction(openAction);
    toolbar->addAction(saveAction);
    toolbar->addAction(saveAsAction);

    connect(newAction, &QAction::triggered, this, &Notepad::onNewFile);
    connect(openAction, &QAction::triggered, this, &Notepad::onOpenFile);
    connect(saveAction, &QAction::triggered, this, &Notepad::onSaveFile);
    connect(saveAsAction, &QAction::triggered, this, &Notepad::onSaveAsFile);
}

void Notepad::onNewFile()
{
    m_textEdit->clear();
    m_currentFilePath.clear();
}

void Notepad::onOpenFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open Text File",
        QDir::homePath(),
        "Text Files (*.txt);;Markdown Files (*.md);;All Files (*)",
        nullptr,
        QFileDialog::DontUseNativeDialog
    );

    if (fileName.isEmpty())
        return;

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", "Cannot open file");
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    if(m_textEdit != nullptr)
        m_textEdit->setPlainText(content);

    m_currentFilePath = fileName;
}

void Notepad::onSaveFile()
{
    QFile file(m_currentFilePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << m_textEdit->toPlainText();

    file.close();
}

void Notepad::onSaveAsFile()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save File As",
        QDir::homePath(),
        "Markdown Files (*.md);;Text Files (*.txt);;All Files (*)",
        nullptr,
        QFileDialog::DontUseNativeDialog
    );

    if (fileName.isEmpty())
        return;

    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", "Cannot save file");
        return;
    }

    QTextStream out(&file);
    out << m_textEdit->toPlainText();
    file.close();

    m_currentFilePath = fileName;
}
