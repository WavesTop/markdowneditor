#include "notepad.h"
#include <QVBoxLayout>
#include <QToolBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>

Notepad::Notepad(QWidget *parent)
    : QMainWindow(parent)
    , m_untitledCount(0)
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
    initToolbar();

    setCentralWidget(m_tabWidget);

    // 创建第一个空白Tab
    onNewFile();
}

void Notepad::initTabWidget()
{
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);

    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &Notepad::onCloseTab);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &Notepad::onTabChanged);
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

QPlainTextEdit* Notepad::currentEditor()
{
    return qobject_cast<QPlainTextEdit*>(m_tabWidget->currentWidget());
}

QPlainTextEdit* Notepad::editorAt(int index)
{
    return qobject_cast<QPlainTextEdit*>(m_tabWidget->widget(index));
}

QPlainTextEdit* Notepad::createEditorTab(const QString& title, const QString& filePath)
{
    QPlainTextEdit* editor = new QPlainTextEdit();
    editor->setPlaceholderText("Enter your markdown text here...");

    // 将文件路径存储在widget的属性中
    editor->setProperty("filePath", filePath);

    int index = m_tabWidget->addTab(editor, title);
    m_tabWidget->setCurrentIndex(index);

    return editor;
}

QString Notepad::getFilePath(int index)
{
    QPlainTextEdit* editor = editorAt(index);
    if (editor)
        return editor->property("filePath").toString();
    return QString();
}

void Notepad::setFilePath(int index, const QString& path)
{
    QPlainTextEdit* editor = editorAt(index);
    if (editor)
        editor->setProperty("filePath", path);
}

void Notepad::updateTabTitle(int index, const QString& filePath)
{
    if (filePath.isEmpty())
        return;

    QFileInfo fileInfo(filePath);
    m_tabWidget->setTabText(index, fileInfo.fileName());
    m_tabWidget->setTabToolTip(index, filePath);
}

void Notepad::onNewFile()
{
    m_untitledCount++;
    QString title = QString("Untitled %1").arg(m_untitledCount);
    createEditorTab(title);
}

void Notepad::onOpenFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open File",
        QDir::homePath(),
        "Markdown Files (*.md);;Text Files (*.txt);;All Files (*)",
        nullptr,
        QFileDialog::DontUseNativeDialog
    );

    if (fileName.isEmpty())
        return;

    // 检查文件是否已经打开
    for (int i = 0; i < m_tabWidget->count(); i++)
    {
        if (getFilePath(i) == fileName)
        {
            m_tabWidget->setCurrentIndex(i);
            return;
        }
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", "Cannot open file");
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // 创建新Tab并加载内容
    QFileInfo fileInfo(fileName);
    QPlainTextEdit* editor = createEditorTab(fileInfo.fileName(), fileName);
    editor->setPlainText(content);

    int currentIndex = m_tabWidget->currentIndex();
    m_tabWidget->setTabToolTip(currentIndex, fileName);
}

void Notepad::onSaveFile()
{
    int currentIndex = m_tabWidget->currentIndex();
    QString filePath = getFilePath(currentIndex);

    // 如果没有文件路径，调用另存为
    if (filePath.isEmpty())
    {
        onSaveAsFile();
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", "Cannot save file");
        return;
    }

    QTextStream out(&file);
    QPlainTextEdit* editor = currentEditor();
    if (editor)
        out << editor->toPlainText();

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
    QPlainTextEdit* editor = currentEditor();
    if (editor)
        out << editor->toPlainText();

    file.close();

    // 更新文件路径和Tab标题
    int currentIndex = m_tabWidget->currentIndex();
    setFilePath(currentIndex, fileName);
    updateTabTitle(currentIndex, fileName);
}

void Notepad::onCloseTab(int index)
{
    if (m_tabWidget->count() == 1)
    {
        // 最后一个Tab，清空内容而不是关闭
        QPlainTextEdit* editor = editorAt(index);
        if (editor)
        {
            editor->clear();
            editor->setProperty("filePath", QString());
            m_tabWidget->setTabText(index, "Untitled 1");
            m_tabWidget->setTabToolTip(index, "");
        }
        return;
    }

    QWidget* widget = m_tabWidget->widget(index);
    m_tabWidget->removeTab(index);
    delete widget;
}

void Notepad::onTabChanged(int index)
{
    if (index < 0)
        return;

    QString filePath = getFilePath(index);
    if (!filePath.isEmpty())
    {
        QFileInfo fileInfo(filePath);
        setWindowTitle(QString("%1 - Markdown Editor").arg(fileInfo.fileName()));
    }
    else
    {
        setWindowTitle(QString("%1 - Markdown Editor").arg(m_tabWidget->tabText(index)));
    }
}
