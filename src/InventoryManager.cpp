#include "InventoryManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>

// 30/04
#include <algorithm> // <--- THÊM DÒNG NÀY VÀO ĐỂ DÙNG HÀM SORT
// 30/04

InventoryManager::InventoryManager() {}

void InventoryManager::loadFromDatabase() {
    bookList.clear(); // Xóa sạch cache cũ
    // QSqlQuery query("SELECT id, title, author, quantity, price FROM Books");
    
// 30/04
    QSqlQuery query("SELECT id, title, author, quantity, price FROM Books ORDER BY id ASC");
// 30/04

    while (query.next()) {
        bookList.append(Book(
            query.value(0).toString(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toInt(),
            query.value(4).toDouble()
        ));
    }
}

bool InventoryManager::addBook(const Book& newBook) {
    // 🛡️ VÁ LỖ HỔNG CHÍ MẠNG: Kiểm tra trùng ID trước khi thêm
    if (findBookById(newBook.getId()) != nullptr) {
        return false; // Trả về false nếu ID đã tồn tại
    }

    QSqlQuery query;
    query.prepare("INSERT INTO Books (id, title, author, quantity, price) VALUES (:id, :title, :author, :qty, :price)");
    query.bindValue(":id", newBook.getId());
    query.bindValue(":title", newBook.getTitle());
    query.bindValue(":author", newBook.getAuthor());
    query.bindValue(":qty", newBook.getQuantity());
    query.bindValue(":price", newBook.getPrice());

    if (query.exec()) {
        bookList.append(newBook); // Cập nhật ngay vào cache để UI không bị lag

    // 30/04
    // --- CHÈN THÊM: Sắp xếp lại danh sách theo Mã sách (ID) từ A-Z ---
    std::sort(bookList.begin(), bookList.end(), [](const Book& a, const Book& b) {
        return a.getId() < b.getId();
    });
    // 30/04

        return true;
    }
    qDebug() << "Lỗi thêm sách DB:" << query.lastError().text();
    return false;
}

bool InventoryManager::updateBook(const QString& id, const Book& newBook) {
    QSqlQuery query;
    query.prepare("UPDATE Books SET title=:title, author=:author, quantity=:qty, price=:price WHERE id=:id");
    query.bindValue(":title", newBook.getTitle());
    query.bindValue(":author", newBook.getAuthor());
    query.bindValue(":qty", newBook.getQuantity());
    query.bindValue(":price", newBook.getPrice());
    query.bindValue(":id", id);

    if (query.exec()) {
        // Cập nhật lại cache
        for (int i = 0; i < bookList.size(); ++i) {
            if (bookList[i].getId() == id) {
                bookList[i] = newBook;
                return true;
            }
        }
    }
    return false;
}

bool InventoryManager::removeBook(const QString& id) {
    QSqlQuery query;
    query.prepare("DELETE FROM Books WHERE id=:id");
    query.bindValue(":id", id);

    if (query.exec()) {
        for (int i = 0; i < bookList.size(); ++i) {
            if (bookList[i].getId() == id) {
                bookList.removeAt(i);
                return true;
            }
        }
    }
    return false;
}

Book* InventoryManager::findBookById(const QString& id) {
    for (int i = 0; i < bookList.size(); ++i) {
        if (bookList[i].getId() == id) return &bookList[i];
    }
    return nullptr;
}

QVector<Book> InventoryManager::getAllBooks() const {
    return bookList;
}