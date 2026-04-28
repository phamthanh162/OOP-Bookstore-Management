#ifndef REPORTMANAGER_H
#define REPORTMANAGER_H

#include "InventoryManager.h"
#include <vector>
#include <QString>

class ReportManager {
private:
    InventoryManager* inventory;

public:
    // Khởi tạo với con trỏ trỏ đến kho hàng hiện tại
    ReportManager(InventoryManager* inv);

    // Các hàm thống kê cơ bản
    int getTotalBooksInStock();       // Tổng số lượng sách trong kho
    double getTotalInventoryValue();  // Tổng giá trị kho hàng
    
    // Bạn có thể thêm hàm lấy danh sách sách sắp hết hàng (ví dụ: số lượng < 10)
    // std::vector<Book> getLowStockBooks(int threshold = 10); 
};

#endif // REPORTMANAGER_H