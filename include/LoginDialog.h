#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QString> // Thêm thư viện này

class LoginDialog : public QDialog {
    Q_OBJECT // Bắt buộc phải có để dùng Signal/Slot

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    QString getRole() const { return userRole; } // <--- Thêm hàm này

protected:
    void changeEvent(QEvent *event) override;

private:
    // Các biến giao diện
    QLineEdit *txtUser = nullptr;
    QLineEdit *txtPass = nullptr;
    QPushButton *btnLogin = nullptr;

    QString userRole; // <--- Thêm biến lưu Quyền (Role)

    // Các hàm xử lý
    void setupUI();
    void handleLogin();
    void updateTheme();
};