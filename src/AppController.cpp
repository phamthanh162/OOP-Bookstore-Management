#include "AppController.h"
#include "LoginDialog.h"
#include "MainWindow.h"

#include <QApplication>
#include <cstdlib>

AppController::AppController(QObject *parent) : QObject(parent) {}

void AppController::start() {
    showLogin();
}

void AppController::showLogin() {
    LoginDialog* login = new LoginDialog();

    // Chờ người dùng nhập xong và bấm OK
    if (login->exec() == QDialog::Accepted) {
        // Lấy quyền và mở trang chính
        MainWindow* w = new MainWindow(login->getRole());

        // KẾT NỐI: Nếu trang chính phát tín hiệu Đăng xuất -> Đóng nó lại và mở Login
        connect(w, &MainWindow::logoutRequested, [this, w]() {
            w->close();
            w->deleteLater();
            this->showLogin(); // Gọi lại màn hình login
        });

        w->show();
    } else {
        // Nếu bấm dấu X tắt cửa sổ Login -> Thoát hoàn toàn
        std::exit(0);
    }
    login->deleteLater();
}