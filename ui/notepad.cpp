#include "notepad.h"
#include <QVBoxLayout>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QPalette>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>

// ============ 颜色定义 ============
namespace Theme {
    const QColor background(39, 40, 34);       // #272822
    const QColor backgroundDark(30, 31, 28);   // #1e1f1c
    const QColor backgroundLight(62, 61, 50);  // #3e3d32
    const QColor foreground(248, 248, 242);    // #f8f8f2
    const QColor foregroundDim(117, 113, 94);  // #75715e
    const QColor selection(73, 72, 62);        // #49483e
    const QColor accent(249, 38, 114);         // #f92672 (pink)
    const QColor accentGreen(166, 226, 46);    // #a6e22e
    const QColor accentYellow(230, 219, 116);  // #e6db74
}

// ============ CustomTabBar 实现 ============
CustomTabBar::CustomTabBar(QWidget* parent)
    : QTabBar(parent)
    , m_hoverIndex(-1)
    , m_closeButtonHovered(false)
{
    setDrawBase(false);
    setExpanding(false);
    setElideMode(Qt::ElideRight);
    setMouseTracking(true);  // 启用鼠标追踪
}

QSize CustomTabBar::tabSizeHint(int index) const
{
    QSize size = QTabBar::tabSizeHint(index);
    // 紧凑的 Tab 尺寸
    size.setHeight(28);
    size.setWidth(qMin(size.width() + 24, 150));  // 为关闭按钮留空间
    size.setWidth(qMax(size.width(), 80));
    return size;
}

QRect CustomTabBar::closeButtonRect(int index) const
{
    QRect tabRect = this->tabRect(index);
    return QRect(tabRect.right() - 20, tabRect.center().y() - 6, 12, 12);
}

void CustomTabBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        for (int i = 0; i < count(); i++)
        {
            if (closeButtonRect(i).contains(event->pos()))
            {
                emit tabCloseClicked(i);
                return;
            }
        }
    }
    QTabBar::mousePressEvent(event);
}

void CustomTabBar::mouseMoveEvent(QMouseEvent* event)
{
    int oldHoverIndex = m_hoverIndex;
    bool oldCloseHovered = m_closeButtonHovered;
    
    m_hoverIndex = -1;
    m_closeButtonHovered = false;
    
    for (int i = 0; i < count(); i++)
    {
        if (tabRect(i).contains(event->pos()))
        {
            m_hoverIndex = i;
            m_closeButtonHovered = closeButtonRect(i).contains(event->pos());
            break;
        }
    }
    
    if (m_hoverIndex != oldHoverIndex || m_closeButtonHovered != oldCloseHovered)
    {
        update();
    }
    
    QTabBar::mouseMoveEvent(event);
}

void CustomTabBar::leaveEvent(QEvent* event)
{
    m_hoverIndex = -1;
    m_closeButtonHovered = false;
    update();
    QTabBar::leaveEvent(event);
}

void CustomTabBar::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景
    painter.fillRect(rect(), Theme::backgroundDark);
    
    for (int i = 0; i < count(); i++)
    {
        QRect tabRect = this->tabRect(i);
        bool isSelected = (i == currentIndex());
        bool isHovered = (i == m_hoverIndex);
        
        // Tab 背景
        QColor bgColor = Theme::backgroundDark;
        if (isSelected)
            bgColor = Theme::background;
        else if (isHovered)
            bgColor = Theme::backgroundLight;
        
        // 绘制圆角矩形背景（顶部圆角）
        QPainterPath path;
        int radius = 4;
        path.moveTo(tabRect.left(), tabRect.bottom() + 1);
        path.lineTo(tabRect.left(), tabRect.top() + radius);
        path.quadTo(tabRect.left(), tabRect.top(), tabRect.left() + radius, tabRect.top());
        path.lineTo(tabRect.right() - radius, tabRect.top());
        path.quadTo(tabRect.right(), tabRect.top(), tabRect.right(), tabRect.top() + radius);
        path.lineTo(tabRect.right(), tabRect.bottom() + 1);
        path.closeSubpath();
        
        painter.fillPath(path, bgColor);
        
        // Tab 文字
        QColor textColor = isSelected ? Theme::foreground : Theme::foregroundDim;
        painter.setPen(textColor);
        
        QFont font = this->font();
        font.setPointSize(11);
        painter.setFont(font);
        
        // 为关闭按钮留出空间
        QRect textRect = tabRect.adjusted(10, 0, -24, 0);
        QString text = tabText(i);
        text = painter.fontMetrics().elidedText(text, Qt::ElideRight, textRect.width());
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);
        
        // 绘制关闭按钮（只在悬停时显示，或者选中时显示）
        if (isSelected || isHovered)
        {
            QRect closeRect = closeButtonRect(i);
            bool closeHovered = (i == m_hoverIndex && m_closeButtonHovered);
            
            if (closeHovered)
            {
                painter.setBrush(Theme::accent);
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(closeRect);
                painter.setPen(QPen(Theme::foreground, 1.5));
            }
            else
            {
                painter.setPen(QPen(Theme::foregroundDim, 1.2));
            }
            
            // 绘制 X
            int margin = 3;
            painter.drawLine(closeRect.left() + margin, closeRect.top() + margin,
                           closeRect.right() - margin, closeRect.bottom() - margin);
            painter.drawLine(closeRect.right() - margin, closeRect.top() + margin,
                           closeRect.left() + margin, closeRect.bottom() - margin);
        }
        
        // 选中的 Tab 底部高亮线
        if (isSelected)
        {
            painter.setPen(Qt::NoPen);
            painter.setBrush(Theme::accentGreen);
            painter.drawRect(tabRect.left() + 2, tabRect.bottom() - 1, tabRect.width() - 4, 2);
        }
    }
}

// ============ CustomTabWidget 实现 ============
CustomTabWidget::CustomTabWidget(QWidget* parent)
    : QTabWidget(parent)
{
    CustomTabBar* tabBar = new CustomTabBar(this);
    setTabBar(tabBar);
    setDocumentMode(true);
    
    // 连接自定义关闭信号
    connect(tabBar, &CustomTabBar::tabCloseClicked, this, &QTabWidget::tabCloseRequested);
}

// ============ Notepad 实现 ============
Notepad::Notepad(QWidget *parent)
    : QMainWindow(parent)
    , m_untitledCount(0)
{
    setWindowTitle("Markdown Editor");
    resize(1200, 800);
    
    applyTheme();
    initUI();
}

Notepad::~Notepad()
{
}

void Notepad::applyTheme()
{
    // 使用 QPalette 设置全局颜色
    QPalette palette;
    palette.setColor(QPalette::Window, Theme::background);
    palette.setColor(QPalette::WindowText, Theme::foreground);
    palette.setColor(QPalette::Base, Theme::background);
    palette.setColor(QPalette::AlternateBase, Theme::backgroundDark);
    palette.setColor(QPalette::Text, Theme::foreground);
    palette.setColor(QPalette::Button, Theme::backgroundDark);
    palette.setColor(QPalette::ButtonText, Theme::foreground);
    palette.setColor(QPalette::Highlight, Theme::selection);
    palette.setColor(QPalette::HighlightedText, Theme::foreground);
    palette.setColor(QPalette::ToolTipBase, Theme::backgroundDark);
    palette.setColor(QPalette::ToolTipText, Theme::foreground);
    palette.setColor(QPalette::PlaceholderText, Theme::foregroundDim);
    
    QApplication::setPalette(palette);
    setPalette(palette);
    
    // 设置全局字体
    QFont appFont("SF Pro Text", 12);
    appFont.setStyleHint(QFont::SansSerif);
    QApplication::setFont(appFont);
}

void Notepad::initUI()
{
    initMenuBar();
    initTabWidget();
    initStatusBar();

    setCentralWidget(m_tabWidget);
    onNewFile();
}

void Notepad::initMenuBar()
{
    QMenuBar* menu = this->menuBar();
    menu->setFont(QFont("SF Pro Text", 13));
    
    // File 菜单
    QMenu* fileMenu = menu->addMenu("File");
    
    QAction* newAction = fileMenu->addAction("New File");
    newAction->setShortcut(QKeySequence::New);
    
    QAction* openAction = fileMenu->addAction("Open File...");
    openAction->setShortcut(QKeySequence::Open);
    
    fileMenu->addSeparator();
    
    QAction* saveAction = fileMenu->addAction("Save");
    saveAction->setShortcut(QKeySequence::Save);
    
    QAction* saveAsAction = fileMenu->addAction("Save As...");
    saveAsAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
    
    fileMenu->addSeparator();
    
    QAction* closeTabAction = fileMenu->addAction("Close Tab");
    closeTabAction->setShortcut(QKeySequence("Ctrl+W"));
    
    fileMenu->addSeparator();
    
    QAction* exitAction = fileMenu->addAction("Exit");
    exitAction->setShortcut(QKeySequence::Quit);
    
    connect(newAction, &QAction::triggered, this, &Notepad::onNewFile);
    connect(openAction, &QAction::triggered, this, &Notepad::onOpenFile);
    connect(saveAction, &QAction::triggered, this, &Notepad::onSaveFile);
    connect(saveAsAction, &QAction::triggered, this, &Notepad::onSaveAsFile);
    connect(closeTabAction, &QAction::triggered, this, [this]() {
        onCloseTab(m_tabWidget->currentIndex());
    });
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    
    // Edit 菜单
    QMenu* editMenu = menu->addMenu("Edit");
    
    QAction* undoAction = editMenu->addAction("Undo");
    undoAction->setShortcut(QKeySequence::Undo);
    
    QAction* redoAction = editMenu->addAction("Redo");
    redoAction->setShortcut(QKeySequence::Redo);
    
    editMenu->addSeparator();
    
    QAction* cutAction = editMenu->addAction("Cut");
    cutAction->setShortcut(QKeySequence::Cut);
    
    QAction* copyAction = editMenu->addAction("Copy");
    copyAction->setShortcut(QKeySequence::Copy);
    
    QAction* pasteAction = editMenu->addAction("Paste");
    pasteAction->setShortcut(QKeySequence::Paste);
    
    editMenu->addSeparator();
    
    QAction* selectAllAction = editMenu->addAction("Select All");
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    
    connect(undoAction, &QAction::triggered, this, [this]() {
        if (currentEditor()) currentEditor()->undo();
    });
    connect(redoAction, &QAction::triggered, this, [this]() {
        if (currentEditor()) currentEditor()->redo();
    });
    connect(cutAction, &QAction::triggered, this, [this]() {
        if (currentEditor()) currentEditor()->cut();
    });
    connect(copyAction, &QAction::triggered, this, [this]() {
        if (currentEditor()) currentEditor()->copy();
    });
    connect(pasteAction, &QAction::triggered, this, [this]() {
        if (currentEditor()) currentEditor()->paste();
    });
    connect(selectAllAction, &QAction::triggered, this, [this]() {
        if (currentEditor()) currentEditor()->selectAll();
    });
    
    // View 菜单
    QMenu* viewMenu = menu->addMenu("View");
    
    QAction* zoomInAction = viewMenu->addAction("Zoom In");
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    
    QAction* zoomOutAction = viewMenu->addAction("Zoom Out");
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    
    connect(zoomInAction, &QAction::triggered, this, [this]() {
        if (currentEditor()) currentEditor()->zoomIn(2);
    });
    connect(zoomOutAction, &QAction::triggered, this, [this]() {
        if (currentEditor()) currentEditor()->zoomOut(2);
    });
}

void Notepad::initTabWidget()
{
    m_tabWidget = new CustomTabWidget(this);
    // 不调用 setTabsClosable(true)，我们使用自定义的关闭按钮
    m_tabWidget->setMovable(true);
    m_tabWidget->setUsesScrollButtons(true);

    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &Notepad::onCloseTab);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &Notepad::onTabChanged);
}

void Notepad::initStatusBar()
{
    QStatusBar* status = statusBar();
    status->setFixedHeight(24);
    
    // 设置状态栏颜色
    QPalette pal = status->palette();
    pal.setColor(QPalette::Window, Theme::backgroundDark);
    pal.setColor(QPalette::WindowText, Theme::foregroundDim);
    status->setPalette(pal);
    status->setAutoFillBackground(true);
    
    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setFont(QFont("SF Pro Text", 11));
    
    m_cursorPosLabel = new QLabel("Ln 1, Col 1");
    m_cursorPosLabel->setFont(QFont("SF Pro Text", 11));
    m_cursorPosLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_cursorPosLabel->setMinimumWidth(100);
    
    status->addWidget(m_statusLabel, 1);
    status->addPermanentWidget(m_cursorPosLabel);
}

CodeEditor* Notepad::currentEditor()
{
    return qobject_cast<CodeEditor*>(m_tabWidget->currentWidget());
}

CodeEditor* Notepad::editorAt(int index)
{
    return qobject_cast<CodeEditor*>(m_tabWidget->widget(index));
}

CodeEditor* Notepad::createEditorTab(const QString& title, const QString& filePath)
{
    CodeEditor* editor = new CodeEditor();
    editor->setProperty("filePath", filePath);
    
    // 设置编辑器颜色
    QPalette editorPal = editor->palette();
    editorPal.setColor(QPalette::Base, Theme::background);
    editorPal.setColor(QPalette::Text, Theme::foreground);
    editorPal.setColor(QPalette::Highlight, Theme::selection);
    editorPal.setColor(QPalette::HighlightedText, Theme::foreground);
    editor->setPalette(editorPal);
    
    // 设置等宽编程字体
    QFont font;
#ifdef Q_OS_MAC
    font = QFont("Menlo", 13);
#else
    font = QFont("Consolas", 11);
#endif
    font.setStyleHint(QFont::Monospace);
    editor->setFont(font);
    
    // Tab 宽度设置为 4 个空格
    QFontMetrics metrics(font);
    editor->setTabStopDistance(4 * metrics.horizontalAdvance(' '));
    
    // 移除边框
    editor->setFrameShape(QFrame::NoFrame);
    
    // 连接光标位置变化信号
    connect(editor, &CodeEditor::cursorPositionChanged, this, &Notepad::updateCursorPosition);

    int index = m_tabWidget->addTab(editor, title);
    m_tabWidget->setCurrentIndex(index);

    return editor;
}

QString Notepad::getFilePath(int index)
{
    CodeEditor* editor = editorAt(index);
    if (editor)
        return editor->property("filePath").toString();
    return QString();
}

void Notepad::setFilePath(int index, const QString& path)
{
    CodeEditor* editor = editorAt(index);
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

void Notepad::updateCursorPosition()
{
    CodeEditor* editor = currentEditor();
    if (editor)
    {
        QTextCursor cursor = editor->textCursor();
        int line = cursor.blockNumber() + 1;
        int col = cursor.columnNumber() + 1;
        m_cursorPosLabel->setText(QString("Ln %1, Col %2").arg(line).arg(col));
    }
}

void Notepad::onNewFile()
{
    m_untitledCount++;
    QString title = QString("untitled-%1").arg(m_untitledCount);
    createEditorTab(title);
    m_statusLabel->setText("New file created");
}

void Notepad::onOpenFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open File",
        QDir::homePath(),
        "All Files (*);;Markdown Files (*.md);;Text Files (*.txt)",
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
        QMessageBox::warning(this, "Error", "Cannot open file: " + fileName);
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    QFileInfo fileInfo(fileName);
    CodeEditor* editor = createEditorTab(fileInfo.fileName(), fileName);
    editor->setPlainText(content);

    int currentIndex = m_tabWidget->currentIndex();
    m_tabWidget->setTabToolTip(currentIndex, fileName);
    m_statusLabel->setText("Opened: " + fileName);
}

void Notepad::onSaveFile()
{
    int currentIndex = m_tabWidget->currentIndex();
    QString filePath = getFilePath(currentIndex);

    if (filePath.isEmpty())
    {
        onSaveAsFile();
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", "Cannot save file: " + filePath);
        return;
    }

    QTextStream out(&file);
    CodeEditor* editor = currentEditor();
    if (editor)
        out << editor->toPlainText();

    file.close();
    m_statusLabel->setText("Saved: " + filePath);
}

void Notepad::onSaveAsFile()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save File As",
        QDir::homePath(),
        "All Files (*);;Markdown Files (*.md);;Text Files (*.txt)",
        nullptr,
        QFileDialog::DontUseNativeDialog
    );

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", "Cannot save file: " + fileName);
        return;
    }

    QTextStream out(&file);
    CodeEditor* editor = currentEditor();
    if (editor)
        out << editor->toPlainText();

    file.close();

    int currentIndex = m_tabWidget->currentIndex();
    setFilePath(currentIndex, fileName);
    updateTabTitle(currentIndex, fileName);
    m_statusLabel->setText("Saved: " + fileName);
}

void Notepad::onCloseTab(int index)
{
    if (m_tabWidget->count() == 1)
    {
        CodeEditor* editor = editorAt(index);
        if (editor)
        {
            editor->clear();
            editor->setProperty("filePath", QString());
            m_tabWidget->setTabText(index, "untitled-1");
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
        setWindowTitle(QString("%1 — Markdown Editor").arg(fileInfo.fileName()));
    }
    else
    {
        setWindowTitle(QString("%1 — Markdown Editor").arg(m_tabWidget->tabText(index)));
    }
    
    updateCursorPosition();
}
