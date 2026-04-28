#include <QApplication>
#include "AppController.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // Ủy quyền toàn bộ luồng chạy cho AppController
    AppController app;
    app.start();

    return a.exec();
}