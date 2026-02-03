#ifndef NOTEPAD_H
#define NOTEPAD_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTabBar>
#include <QStatusBar>
#include <QLabel>
#include "codeeditor.h"

// 自定义 TabBar，实现更精细的样式控制
class CustomTabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit CustomTabBar(QWidget* parent = nullptr);
    
protected:
    QSize tabSizeHint(int index) const override;
    void paintEvent(QPaintEvent* event) override;
};

// 自定义 TabWidget
class CustomTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit CustomTabWidget(QWidget* parent = nullptr);
};

class Notepad : public QMainWindow
{
    Q_OBJECT

public:
    explicit Notepad(QWidget *parent = nullptr);
    ~Notepad();

private:
    CustomTabWidget* m_tabWidget;
    QLabel* m_statusLabel;
    QLabel* m_cursorPosLabel;
    int m_untitledCount;

    void initUI();
    void initMenuBar();
    void initTabWidget();
    void initStatusBar();
    void applyTheme();

    CodeEditor* currentEditor();
    CodeEditor* editorAt(int index);
    CodeEditor* createEditorTab(const QString& title, const QString& filePath = QString());
    QString getFilePath(int index);
    void setFilePath(int index, const QString& path);
    void updateTabTitle(int index, const QString& filePath);

private slots:
    void onNewFile();
    void onOpenFile();
    void onSaveFile();
    void onSaveAsFile();
    void onCloseTab(int index);
    void onTabChanged(int index);
    void updateCursorPosition();
};

#endif // NOTEPAD_H
