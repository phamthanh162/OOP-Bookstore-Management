#pragma once
#include "InventoryManager.h"
#include "ReportManager.h"

#include <QWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QtCharts>
#include <QChart>
#include <QBarCategoryAxis>
#include <QValueAxis>

class MainWindow : public QWidget {
    Q_OBJECT 

public:
    // Cho phép truyền tham số "role" (Quyền) vào khi khởi tạo app
    explicit MainWindow(QString role = "Admin", QWidget *parent = nullptr);

signals:
    void logoutRequested(); // <--- CHÈN VÀO ĐÂY: Tín hiệu báo hệ thống cần đăng xuất

// Thêm vào phần private slots:
private slots:
    void on_btnMenuReport_clicked(); // Xử lý khi bấm nút "Báo cáo" ở sidebar
    void refreshReportData();        // Cập nhật lại số liệu

protected:
    // Hàm hệ thống của Qt, tự động kích hoạt khi có biến động từ Windows
    void changeEvent(QEvent *event) override;

private:
    // --- BIẾN CHO TRANG DASHBOARD & BÁO CÁO ---
    QLabel *lblDashTotalBooks; // <--- THÊM DÒNG NÀY
    QLabel *lblDashTotalValue; // <--- THÊM DÒNG NÀY
    QLabel *lblDashTotalInvoices;  // <--- THÊM BIẾN NÀY (Tổng hóa đơn)
    QLabel *lblDashTotalCustomers; // <--- THÊM BIẾN NÀY (Tổng khách hàng)

    // THÊM 5 BIẾN NÀY ĐỂ ĐIỀU KHIỂN BIỂU ĐỒ VÀ BẢNG TOP 10:
    QChart *inventoryChart = nullptr;
    QBarCategoryAxis *inventoryAxisX = nullptr;
    QValueAxis *inventoryAxisY = nullptr;
    QTableWidget *tableTopBooks = nullptr;
    
    ReportManager* reportManager;
    
    // UI elements cho trang Báo cáo
    QWidget* reportPage;
    QTableWidget* tableLowStock;

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

    // Thêm 3 dòng này vào dưới lblTotalValue
    QLabel *lblTotalSold;
    QLabel *lblTotalRevenue;
    QLabel *lblTotalCost;    // <--- THÊM BIẾN NÀY (Tiền vốn sách)
    QLabel *lblTotalProfit;

    // --- BIẾN MỚI CHO TRANG BÁN HÀNG (POS) ---
    // Gắn nullptr để an toàn tuyệt đối khi app vừa khởi động
    QTableWidget *salesBookTable = nullptr; 
    QTableWidget *cartTable = nullptr;      
    QLabel *lblCartTotal = nullptr;   
    QLineEdit *searchSalesInput;    // <--- THÊM DÒNG NÀY (Thanh tìm kiếm quầy bán)      

    QLineEdit *txtVoucher;
    double voucherPercent = 0.0; // Lưu % giảm (VD: 0.15)
    double voucherFlat = 0.0;    // Lưu số tiền giảm thẳng (VD: 50000)

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

    QWidget* createReportPage();    // <--- THÊM DÒNG NÀY VÀO ĐÂY
    
    void refreshInventoryTable();
    void updateDashboardStats();
    
    // --- HÀM MỚI CHO TRANG BÁN HÀNG ---
    void refreshSalesBookTable();
    void updateCartTotal();

    // --- BIẾN CHO DASHBOARD ---
    QTableWidget *recentSalesTable = nullptr; 

    QTableWidget *recentImportsTable = nullptr; // <--- THÊM BIẾN NÀY

    void loadSalesHistory(); // Hàm quét ổ cứng tìm hóa đơn
    void loadImportHistory(); // <--- THÊM HÀM NÀY (Quét tìm phiếu nhập)
    void updateTheme(); // Hàm do ta tự viết để tính toán lại toàn bộ màu sắc
};

