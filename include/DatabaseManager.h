#pragma once
#include <QString>
#include <QSqlDatabase>

class DatabaseManager {
public:
    // Hàm tĩnh để khởi tạo DB 1 lần lúc bật app
    static bool initializeDatabase();
};
