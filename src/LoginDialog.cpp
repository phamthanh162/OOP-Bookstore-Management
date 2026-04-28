#include "LoginDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setupUI();
}

void LoginDialog::setupUI() {
    setWindowTitle("Bảo mật Hệ thống");
    setFixedSize(350, 260); 
    setStyleSheet("background-color: #F8F9FA;"); // Nền xám nhạt cho toàn bộ khung

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(15);

    QLabel *logo = new QLabel("📚 ĐĂNG NHẬP FAHASA");
    logo->setStyleSheet("font-size: 20px; font-weight: bold; color: #C92127;");
    logo->setAlignment(Qt::AlignCenter);

    txtUser = new QLineEdit();
    txtUser->setPlaceholderText("Tên đăng nhập (Gợi ý: admin)");
    // CHỈ CẦN THÊM DÒNG: color: #2D3748; (Ép chữ màu tối)
    txtUser->setStyleSheet("padding: 10px; border: 1px solid #CED4DA; border-radius: 5px; font-size: 14px; color: #2D3748;");

    txtPass = new QLineEdit();
    txtPass->setPlaceholderText("Mật khẩu (Gợi ý: 123)");
    txtPass->setEchoMode(QLineEdit::Password);
    // TƯƠNG TỰ, THÊM: color: #2D3748; VÀO STYLE SHEET CỦA Ô MẬT KHẨU
    txtPass->setStyleSheet("padding: 10px; border: 1px solid #CED4DA; border-radius: 5px; font-size: 14px; color: #2D3748;");

    btnLogin = new QPushButton("🔓 ĐĂNG NHẬP");
    btnLogin->setStyleSheet("background-color: #C92127; color: white; padding: 12px; font-weight: bold; border-radius: 5px; font-size: 14px;");

    layout->addWidget(logo);
    layout->addSpacing(10);
    layout->addWidget(txtUser);
    layout->addWidget(txtPass);
    layout->addSpacing(10);
    layout->addWidget(btnLogin);

    connect(btnLogin, &QPushButton::clicked, this, &LoginDialog::handleLogin);
}

void LoginDialog::handleLogin() {
    QString u = txtUser->text();
    QString p = txtPass->text();

    if (u == "admin" && p == "123") {
        userRole = "Admin"; // Gắn mác Giám đốc
        accept();
    } 
    else if (u == "staff" && p == "123") {
        userRole = "Staff"; // Gắn mác Nhân viên
        accept();
    } 
    else {
        QMessageBox::critical(this, "Từ chối truy cập", "Sai tên đăng nhập hoặc mật khẩu!\nVui lòng thử lại.");
    }
}