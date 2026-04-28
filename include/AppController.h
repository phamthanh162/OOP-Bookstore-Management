#pragma once
#include <QObject>

class AppController : public QObject {
    Q_OBJECT
public:
    explicit AppController(QObject *parent = nullptr);
    void start(); // Hàm mồi để chạy app

private slots:
    void showLogin(); // Hàm hiển thị luồng đăng nhập
};