#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>

bool DatabaseManager::initializeDatabase() {
    // 1. Lấy đường dẫn nơi file .exe đang chạy (thường là thư mục build)
    QString exePath = QCoreApplication::applicationDirPath();
    
    // 2. Lùi ra ngoài 1 cấp (thư mục gốc dự án) và ghép với datafiles
    QString dataFolder = exePath + "/../datafiles"; 

    // 3. Tự động tạo thư mục nếu chưa tồn tại
    QDir dir;
    if (!dir.exists(dataFolder)) {
        dir.mkpath(dataFolder);
    }

    // 4. Kết nối Database với đường dẫn tuyệt đối
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = dataFolder + "/fetel_database.db";
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qDebug() << "Không thể kết nối Database:" << db.lastError().text();
        return false;
    }

    QSqlQuery query;
    
    query.exec("CREATE TABLE IF NOT EXISTS Books ("
               "id TEXT PRIMARY KEY, "
               "title TEXT, "
               "author TEXT, "
               "quantity INTEGER, "
               "price REAL)");

    query.exec("CREATE TABLE IF NOT EXISTS Customers ("
               "phone TEXT PRIMARY KEY, "
               "name TEXT, "
               "points INTEGER)");

    query.exec("CREATE TABLE IF NOT EXISTS Invoices ("
               "id TEXT PRIMARY KEY, "
               "phone TEXT, "
               "date TEXT, "
               "total REAL, "
               "staff TEXT)");

    query.exec("CREATE TABLE IF NOT EXISTS InvoiceDetails ("
               "invoice_id TEXT, "
               "book_id TEXT, "
               "quantity INTEGER, "
               "price REAL, "
               "FOREIGN KEY(invoice_id) REFERENCES Invoices(id))");

    query.exec("CREATE TABLE IF NOT EXISTS Imports ("
               "id TEXT PRIMARY KEY, "
               "date TEXT, "
               "total REAL)");

    query.exec("CREATE TABLE IF NOT EXISTS Vouchers ("
               "code TEXT PRIMARY KEY, "
               "discount_percent REAL, "
               "discount_flat REAL, "
               "usage_limit INTEGER, "
               "used_count INTEGER DEFAULT 0, "
               "expiry_date TEXT)");
    
    // --- MIGRATION KHÁCH HÀNG ---
    QSqlQuery checkCus("SELECT COUNT(*) FROM Customers");
    if (checkCus.next() && checkCus.value(0).toInt() == 0) {
        QFile file(dataFolder + "/db_customers.txt");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QSqlQuery q;
            q.prepare("INSERT INTO Customers (phone, points, name) VALUES (:phone, :points, :name)");
            
            QSqlDatabase::database().transaction(); // Bật chế độ siêu tốc
            while (!in.atEnd()) {
                QStringList p = in.readLine().split("|");
                if (p.size() >= 2) {
                    q.bindValue(":phone", p[0]);
                    q.bindValue(":points", p[1].toInt());
                    q.bindValue(":name", p.size() >= 3 ? p[2] : "Khách hàng");
                    q.exec();
                }
            }
            QSqlDatabase::database().commit(); // Chốt hạ
            file.close();
            qDebug() << "✅ Đã migrate Customers!";
        }
    }

    // --- MIGRATION VOUCHER ---
    QSqlQuery checkVou("SELECT COUNT(*) FROM Vouchers");
    if (checkVou.next() && checkVou.value(0).toInt() == 0) {
        QFile file(dataFolder + "/db_vouchers.txt");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QSqlQuery q;
            q.prepare("INSERT INTO Vouchers (code, discount_percent, discount_flat, usage_limit, used_count, expiry_date) "
                      "VALUES (:code, :pct, :flat, 100, 0, '2026-12-31')"); // Set sẵn hạn mức
            
            QSqlDatabase::database().transaction();
            while (!in.atEnd()) {
                QStringList p = in.readLine().split("|");
                if (p.size() >= 2) {
                    double val = p[1].toDouble();
                    q.bindValue(":code", p[0]);
                    q.bindValue(":pct", val < 1.0 ? val : 0.0);
                    q.bindValue(":flat", val >= 1.0 ? val : 0.0);
                    q.exec();
                }
            }
            QSqlDatabase::database().commit();
            file.close();
            qDebug() << "✅ Đã migrate Vouchers!";
        }
    }
    return true;
}