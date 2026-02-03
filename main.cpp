#include <QApplication>
#include "ui/notepad.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    Notepad window;
    window.show();
    
    return app.exec();
}
