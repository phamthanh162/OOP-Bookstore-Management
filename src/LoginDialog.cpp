#include "LoginDialog.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QScreen>
#include <QGuiApplication>
#include <QGraphicsDropShadowEffect>
#include <QEvent>
#include <QPalette>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setupUI();
}

void LoginDialog::setupUI() {
    setWindowTitle("Hệ thống FETEL - Đăng nhập");
    setFixedSize(350, 260); 

    QRect screenGeom = QGuiApplication::primaryScreen()->geometry();
    move((screenGeom.width() - width()) / 2, (screenGeom.height() - height()) / 2);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(15);

    // --- ĐÃ HÒA GIẢI XUNG ĐỘT (MERGED) ---
    QLabel *logo = new QLabel("📚 ĐĂNG NHẬP FETEL PRO");
    logo->setStyleSheet("background-color: transparent; font-size: 20px; font-weight: bold; color: #0088CC;"); 
    
    logo->setAlignment(Qt::AlignCenter);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(6); 
    shadow->setColor(QColor(0, 0, 0, 90)); 
    shadow->setOffset(1, 2);
    logo->setGraphicsEffect(shadow);

    txtUser = new QLineEdit();
    txtUser->setPlaceholderText("Tên đăng nhập (Gợi ý: admin)");

    txtPass = new QLineEdit();
    txtPass->setPlaceholderText("Mật khẩu (Gợi ý: 123)");
    txtPass->setEchoMode(QLineEdit::Password);

    btnLogin = new QPushButton("VÀO HỆ THỐNG");
    btnLogin->setStyleSheet("background-color: #0088CC; color: white; padding: 12px; font-weight: bold; border-radius: 5px;");

    layout->addWidget(logo);
    layout->addSpacing(10);
    layout->addWidget(txtUser);
    layout->addWidget(txtPass);
    layout->addSpacing(10);
    layout->addWidget(btnLogin);

    // KÍCH HOẠT THEME NGAY KHI VỪA TẠO XONG GIAO DIỆN
    updateTheme();

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

void LoginDialog::changeEvent(QEvent *event) {
    if (event->type() == QEvent::PaletteChange || 
        event->type() == QEvent::ApplicationPaletteChange || 
        event->type() == QEvent::ThemeChange) {
        updateTheme();
    }
    QDialog::changeEvent(event);
}

void LoginDialog::updateTheme() {
    if (!txtUser || !txtPass) return;

    // 🛡️ KHIÊN CHỐNG ĐỆ QUY VÔ HẠN (Phải có dòng này!)
    static bool isUpdating = false;
    if (isUpdating) return; 
    isUpdating = true;      

    bool isDark = QGuiApplication::palette().color(QPalette::Window).lightness() < 128;
    QString inputBg = isDark ? "#161B22" : "#FFFFFF";
    QString textColor = isDark ? "#C9D1D9" : "#2D3748";

    QString lineEditQss = QString("padding: 10px; border: 1px solid #0088CC; border-radius: 5px; font-size: 14px; background-color: %1; color: %2;")
                          .arg(inputBg, textColor);
    
    txtUser->setStyleSheet(lineEditQss);
    txtPass->setStyleSheet(lineEditQss);
    this->setStyleSheet(isDark ? "background-color: #0D1117;" : "background-color: #F8F9FA;");

    isUpdating = false; // 🔓 Xong việc thì mở khóa ra
}