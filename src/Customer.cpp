#include "Customer.h"

Customer::Customer() : phone(""), points(0), name("Khách hàng VIP") {}

Customer::Customer(QString phone, int points, QString name)
    : phone(phone), points(points), name(name) {}

QString Customer::getPhone() const { return phone; }
QString Customer::getName() const { return name; }
int Customer::getPoints() const { return points; }

void Customer::setPhone(QString p) { phone = p; }
void Customer::setName(QString n) { name = n; }
void Customer::setPoints(int p) { points = p; }

// LOGIC XẾP HẠNG THẺ TỰ ĐỘNG
QString Customer::getRank() const {
    if (points >= 3000) return "Kim Cương";
    if (points >= 2000) return "Vàng";
    if (points >= 1000) return "Bạc";
    return "Đồng";
}