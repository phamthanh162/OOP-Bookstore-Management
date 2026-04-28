/*
#pragma once
#include <QString>

class Customer {
private:
    QString phone;
    int points;
    QString rank; // Đồng, Bạc, Vàng

public:
    Customer(QString p, int pts = 0) : phone(p), points(pts) {
        updateRank();
    }

    void addPoints(int p) { points += p; updateRank(); }

    void updateRank() {
            if (points >= 3000) rank = "Kim Cương";
            else if (points >= 1000) rank = "Vàng";
            else if (points >= 500) rank = "Bạc";
            else rank = "Đồng";
        }

    QString getPhone() const { return phone; }
    QString getRank() const { return rank; }
    int getPoints() const { return points; }
};
*/

// 28/04  
#pragma once
#include <QString>

class Customer {
private:
    QString phone;
    QString name;
    int points;

public:
    // Khởi tạo mặc định
    Customer();
    // Khởi tạo có tham số (Hỗ trợ code cũ chỉ truyền SĐT và Điểm)
    Customer(QString phone, int points, QString name = "Khách hàng VIP");

    // Getters
    QString getPhone() const;
    QString getName() const;
    int getPoints() const;
    QString getRank() const; // Tự động tính hạng dựa trên điểm

    // Setters
    void setPhone(QString phone);
    void setName(QString name);
    void setPoints(int points);
};
// 28/04