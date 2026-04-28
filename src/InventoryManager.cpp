#include "InventoryManager.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>

InventoryManager::InventoryManager() {}

void InventoryManager::addBook(const Book& newBook) {
    // Tạm thời cứ thêm thẳng vào cuối danh sách
    bookList.append(newBook);
}

bool InventoryManager::removeBook(const QString& id) {
    for (int i = 0; i < bookList.size(); ++i) {
        if (bookList[i].getId() == id) {
            bookList.removeAt(i);
            return true; // Xóa thành công
        }
    }
    return false; // Không tìm thấy sách để xóa
}

Book* InventoryManager::findBookById(const QString& id) {
    for (int i = 0; i < bookList.size(); ++i) {
        if (bookList[i].getId() == id) {
            return &bookList[i];
        }
    }
    return nullptr; // Không tìm thấy
}

QVector<Book> InventoryManager::getAllBooks() const {
    return bookList;
}

// --- GHI DỮ LIỆU RA FILE ---
void InventoryManager::saveToFile(const QString& fileName) {
    QFile file(fileName);
    // Mở file ở chế độ Ghi đè (WriteOnly) và dạng Text
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (int i = 0; i < bookList.size(); ++i) {
            // Lưu mỗi cuốn sách trên 1 dòng, cách nhau bởi dấu gạch đứng "|"
            out << bookList[i].getId() << "|" 
                << bookList[i].getTitle() << "|" 
                << bookList[i].getAuthor() << "|" 
                << bookList[i].getQuantity() << "|" 
                << bookList[i].getPrice() << "\n";
        }
        file.close();
    }
}

// --- ĐỌC DỮ LIỆU TỪ FILE ---
void InventoryManager::loadFromFile(const QString& fileName) {
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        bookList.clear(); // Xóa sạch kho cũ trước khi nạp dữ liệu mới
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split("|"); // Cắt dòng text ra thành các phần
            if (parts.size() == 5) { // Phải đủ 5 thông tin mới nạp vào
                bookList.append(Book(parts[0], parts[1], parts[2], parts[3].toInt(), parts[4].toDouble()));
            }
        }
        file.close();
    }
}

bool InventoryManager::updateBook(const QString& id, const Book& newBook) {
    for (int i = 0; i < bookList.size(); ++i) {
        if (bookList[i].getId() == id) {
            bookList[i] = newBook; // Ghi đè dữ liệu mới
            return true;
        }
    }
    return false;
}