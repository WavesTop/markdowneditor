#ifndef NOTEPAD_H
#define NOTEPAD_H

#include <QMainWindow>
#include <QLabel>
#include <QPlainTextEdit>
#include <QTabWidget>

class Notepad : public QMainWindow
{
    Q_OBJECT

public:
    explicit Notepad(QWidget *parent = nullptr);
    ~Notepad();

private:
    QTabWidget* m_tabWidget;
    QPlainTextEdit* m_textEdit;

    void initToolbar();

    void initUI();
    void initTabWidget();
    void initTextEdit();

    QString m_currentFilePath;
private slots:
    void onNewFile();
    void onOpenFile();
    void onSaveFile();
    void onSaveAsFile();
};

#endif // NOTEPAD_H
