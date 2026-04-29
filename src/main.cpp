#include <QApplication>
#include "AppController.h"
#include "DatabaseManager.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // Kích hoạt Database ngay lập tức
    DatabaseManager::initializeDatabase();

    AppController app;
    app.start();

    return a.exec();
}