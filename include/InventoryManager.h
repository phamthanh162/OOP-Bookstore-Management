#pragma once
#include <QVector>
#include "Book.h"

class InventoryManager {
private:
    QVector<Book> bookList; // Danh sách sách trong kho

public:
    InventoryManager();

    // Các hàm nghiệp vụ cốt lõi
    void addBook(const Book& newBook);
    bool removeBook(const QString& id);
    bool updateBook(const QString& id, const Book& newBook);
    Book* findBookById(const QString& id); // Trả về con trỏ để dễ kiểm tra Null
    
    // Lấy toàn bộ danh sách để in ra bảng giao diện
    QVector<Book> getAllBooks() const;

    void saveToFile(const QString& fileName);
    void loadFromFile(const QString& fileName);
};