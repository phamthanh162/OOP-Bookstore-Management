#include <QApplication>
#include "MainWindow.h"
#include "LoginDialog.h" // Kéo cái cổng bảo vệ vào

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 1. Dựng cổng bảo vệ lên
    LoginDialog login;
    
    // 2. Nếu đăng nhập thành công (hàm accept() được gọi)
    if (login.exec() == QDialog::Accepted) {
        // 3. Mở phần mềm chính
        MainWindow w(login.getRole()); // <--- SỬA DÒNG NÀY LÀ XONG
        w.show();
        return a.exec();
    }

    // Nếu tắt khung đăng nhập hoặc sai pass -> Dừng app
    return 0; 
}