#pragma once
#include <QVector>
#include "Book.h"

class InventoryManager {
private:
    QVector<Book> bookList; // Vẫn giữ làm Cache bộ nhớ tạm để UI chạy nhanh

public:
    InventoryManager();

    bool addBook(const Book& newBook); // Đổi thành bool để bắt lỗi trùng ID
    bool removeBook(const QString& id);
    bool updateBook(const QString& id, const Book& newBook);
    Book* findBookById(const QString& id);
    
    QVector<Book> getAllBooks() const;

    // KHAI TỬ saveToFile và loadFromFile, thay bằng:
    void loadFromDatabase(); 
};