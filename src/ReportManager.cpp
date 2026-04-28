#include "ReportManager.h"
#include "Book.h"

ReportManager::ReportManager(InventoryManager* inv) : inventory(inv) {}

int ReportManager::getTotalBooksInStock() {
    int total = 0;
    // Giả sử InventoryManager có hàm getBooks() trả về vector/list các Book
    // for (const auto& book : inventory->getBooks()) {
    //     total += book.getQuantity(); 
    // }
    return total; 
}

double ReportManager::getTotalInventoryValue() {
    double totalValue = 0.0;
    // for (const auto& book : inventory->getBooks()) {
    //     totalValue += (book.getQuantity() * book.getPrice());
    // }
    return totalValue;
}