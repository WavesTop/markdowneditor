#ifndef NOTEPAD_H
#define NOTEPAD_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QMap>

class Notepad : public QMainWindow
{
    Q_OBJECT

public:
    explicit Notepad(QWidget *parent = nullptr);
    ~Notepad();

private:
    QTabWidget* m_tabWidget;
    int m_untitledCount;  // 未命名文件计数器

    void initToolbar();
    void initUI();
    void initTabWidget();

    // 获取当前活动的编辑器
    QPlainTextEdit* currentEditor();
    // 获取指定索引的编辑器
    QPlainTextEdit* editorAt(int index);
    // 创建新的编辑器Tab
    QPlainTextEdit* createEditorTab(const QString& title, const QString& filePath = QString());
    // 获取Tab对应的文件路径
    QString getFilePath(int index);
    // 设置Tab对应的文件路径
    void setFilePath(int index, const QString& path);
    // 更新Tab标题
    void updateTabTitle(int index, const QString& filePath);

private slots:
    void onNewFile();
    void onOpenFile();
    void onSaveFile();
    void onSaveAsFile();
    void onCloseTab(int index);
    void onTabChanged(int index);
};

#endif // NOTEPAD_H
