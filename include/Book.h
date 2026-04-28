#pragma once
#include <QString>

class Book {
protected: // Dùng protected để lớp con có thể kế thừa
    QString id;
    QString title;
    QString author;
    int quantity;
    double price;

public:
    Book(); // Constructor mặc định
    Book(QString id, QString title, QString author, int quantity, double price);
    virtual ~Book() {} // Hàm hủy ảo để an toàn cho đa hình

    // Các hàm Getter (Lấy dữ liệu)
    QString getId() const;
    QString getTitle() const;
    QString getAuthor() const;
    int getQuantity() const;
    double getPrice() const;

    // Các hàm Setter (Sửa dữ liệu)
    void setQuantity(int qty);
    void setPrice(double pr);

    // Hàm Đa hình (Polymorphism)
    virtual QString getType() const { return "Sách thường"; }
};

// --- LỚP CON KẾ THỪA ---
class TextBook : public Book {
    int grade;
public:
    TextBook(QString id, QString title, QString author, int qty, double pr, int g)
        : Book(id, title, author, qty, pr), grade(g) {}
    
    QString getType() const override { 
        return "Sách Giáo Khoa (Lớp " + QString::number(grade) + ")"; 
    }
};