#pragma once
#include <QWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include "InventoryManager.h"

// mới thêm 27/04

// Thêm include ở đầu file
#include "ReportManager.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QtCharts>
#include <QChart>
#include <QBarCategoryAxis>
#include <QValueAxis>
// mới thêm 27/04



class MainWindow : public QWidget {
    Q_OBJECT 

public:
    // Cho phép truyền tham số "role" (Quyền) vào khi khởi tạo app
    explicit MainWindow(QString role = "Admin", QWidget *parent = nullptr);


// mới thêm 27/04
// Thêm vào phần private slots:
private slots:
    void on_btnMenuReport_clicked(); // Xử lý khi bấm nút "Báo cáo" ở sidebar
    void refreshReportData();        // Cập nhật lại số liệu
// mới thêm 27/04


private:

// mới thêm 27/04
// --- BIẾN CHO TRANG DASHBOARD & BÁO CÁO ---
    QLabel *lblDashTotalBooks; // <--- THÊM DÒNG NÀY
    QLabel *lblDashTotalValue; // <--- THÊM DÒNG NÀY
    QLabel *lblDashTotalInvoices;  // <--- THÊM BIẾN NÀY (Tổng hóa đơn)
    QLabel *lblDashTotalCustomers; // <--- THÊM BIẾN NÀY (Tổng khách hàng)
// mới thêm 27/04

// mới thêm 28/04
// THÊM 5 BIẾN NÀY ĐỂ ĐIỀU KHIỂN BIỂU ĐỒ VÀ BẢNG TOP 10:
    QChart *inventoryChart;
    QBarCategoryAxis *inventoryAxisX;
    QValueAxis *inventoryAxisY;
    QTableWidget *tableTopBooks;
// mới thêm 28/04


// mới thêm 27/04
    ReportManager* reportManager;
    
    // UI elements cho trang Báo cáo
    QWidget* reportPage;
    QTableWidget* tableLowStock;
// mới thêm 27/04


    QListWidget *sideMenu;        
    QStackedWidget *stackedPages; 
    QString userRole; // <--- Thêm biến lưu Quyền (Role)
    QString currentUserRole;

    // --- BIẾN CHO TRANG QUẢN LÝ KHO ---
    InventoryManager manager; 
    QTableWidget *inventoryTable; 
    QLineEdit *idInput, *titleInput, *authorInput, *quantityInput, *priceInput;
    QLineEdit *searchInventoryInput; // <--- THÊM DÒNG NÀY (Thanh tìm kiếm kho)

    // --- BIẾN CHO TRANG DASHBOARD ---
    QLabel *lblTotalBooks;
    QLabel *lblTotalValue;

// mới thêm 27/04
// Thêm 3 dòng này vào dưới lblTotalValue
    QLabel *lblTotalSold;
    QLabel *lblTotalRevenue;
    QLabel *lblTotalCost;    // <--- THÊM BIẾN NÀY (Tiền vốn sách)
    QLabel *lblTotalProfit;
// mới thêm 27/04

    // --- BIẾN MỚI CHO TRANG BÁN HÀNG (POS) ---
    // Gắn nullptr để an toàn tuyệt đối khi app vừa khởi động
    QTableWidget *salesBookTable = nullptr; 
    QTableWidget *cartTable = nullptr;      
    QLabel *lblCartTotal = nullptr;   
    QLineEdit *searchSalesInput;    // <--- THÊM DÒNG NÀY (Thanh tìm kiếm quầy bán)      

// 28/04
    QLineEdit *txtVoucher;
    double voucherPercent = 0.0; // Lưu % giảm (VD: 0.15)
    double voucherFlat = 0.0;    // Lưu số tiền giảm thẳng (VD: 50000)
// 28/04

    // THÊM 3 DÒNG NÀY VÀO:
    QLineEdit *txtCustomerPhone; 
    QLabel *lblCustomerInfo;
    double currentDiscount = 0.0; // Lưu % giảm giá (VD: 0.05 là 5%)
    
    // --- KHAI BÁO CÁC HÀM ---
    void setupUI();
    QWidget* createDashboardPage();
    QWidget* createInventoryPage(); 
    QWidget* createSalesPage();     
    QWidget* createSettingsPage();  

    // mới thêm 27/04
    QWidget* createReportPage();    // <--- THÊM DÒNG NÀY VÀO ĐÂY
    // mới thêm 27/04
    
    void refreshInventoryTable();
    void updateDashboardStats();
    
    // --- HÀM MỚI CHO TRANG BÁN HÀNG ---
    void refreshSalesBookTable();
    void updateCartTotal();

    // --- BIẾN CHO DASHBOARD ---
    QTableWidget *recentSalesTable = nullptr; 

// mới thêm 28/04
QTableWidget *recentImportsTable = nullptr; // <--- THÊM BIẾN NÀY
// mới thêm 28/04

    void loadSalesHistory(); // Hàm quét ổ cứng tìm hóa đơn

// mới thêm 28/04
    void loadImportHistory(); // <--- THÊM HÀM NÀY (Quét tìm phiếu nhập)
// mới thêm 28/04

};

