#include "MainWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFont>
#include <QGridLayout>
#include <QFrame>
#include <QHeaderView>
#include <QTableWidget>
#include <QFormLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QLocale>
#include <QGroupBox>
#include <QComboBox>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QtCharts>
#include <QBarSeries>
#include <QBarSet>
#include <QChart>
#include <QChartView>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QDir>
#include <QPixmap>
#include <QPainter>
#include <QPen>
#include "Customer.h"
#include <QInputDialog>

QLocale locale(QLocale::Vietnamese, QLocale::Vietnam);

MainWindow::MainWindow(QString role, QWidget *parent) : QWidget(parent), currentUserRole(role) {
    setWindowTitle("Hệ thống Quản lý Nhà Sách Pro");
    resize(1200, 800); // Cho size to đùng luôn!

    // --- PHÉP THUẬT QSS ---
    this->setStyleSheet(R"(
        /* Màu nền tổng thể màu xám siêu nhạt */
        MainWindow {
            background-color: #F4F7FE;
        }
        
        /* Menu bên trái màu Xanh đen giống ảnh */
        QListWidget {
            background-color: #111C43;
            color: #A0ABC0;
            font-size: 15px;
            font-weight: bold;
            border: none;
            padding-top: 20px;
        }
        
        /* Khi click chọn 1 menu thì nó đổi sang màu xanh dương sáng */
        QListWidget::item:selected {
            background-color: #2B6CB0;
            color: white;
            border-left: 5px solid #63B3ED;
        }
        
        /* Khi hover chuột qua menu */
        QListWidget::item:hover {
            background-color: #1A2859;
            color: white;
        }
        
        /* Các khối Card màu trắng, bo góc */
        QFrame#StatCard, QFrame#ContentCard {
            background-color: white;
            border-radius: 10px;
            border: 1px solid #E2E8F0;
        }

        /* --- Làm đẹp Bảng (Table) --- */
        QTableWidget {
            background-color: white;
            color: #2D3748; /* <--- Thêm dòng này vào */
            border: 1px solid #E2E8F0;
            border-radius: 8px;
            gridline-color: #EDF2F7;
            font-size: 14px;
        }
        QHeaderView::section {
            background-color: #F8FAFC;
            color: #4A5568;
            font-weight: bold;
            border: none;
            border-right: 1px solid #E2E8F0;
            border-bottom: 1px solid #E2E8F0;
            padding: 8px;
        }

        /* --- Sửa lỗi chữ tàng hình & Làm đẹp Ô nhập liệu (QLineEdit) --- */
        QLineEdit {
            background-color: white;
            color: #2D3748; /* Ép chữ phải có màu xám đậm/đen */
            border: 1px solid #CBD5E0;
            border-radius: 5px;
            padding: 8px;
            font-size: 14px;
        }
        
        /* Hiệu ứng viền xanh khi click chuột vào ô nhập */
        QLineEdit:focus {
            border: 2px solid #3182CE; 
        }

        /* --- Sửa lỗi chữ tàng hình cho các Nhãn (QLabel) --- */
        QLabel {
            color: #2D3748; /* Chữ màu tối để hiện rõ trên nền trắng */
        }
    )");

    setupUI(); // Gọi hàm tạo giao diện

    // ========================================================
    // --- ÁP DỤNG PHÂN QUYỀN (ROLE-BASED ACCESS CONTROL) ---
    // ========================================================
    if (currentUserRole == "Staff") {
        // Nếu là Nhân viên -> Giấu các menu không cho phép đụng vào!
        sideMenu->item(0)->setHidden(true); // Ẩn Tổng quan
        sideMenu->item(1)->setHidden(true); // Ẩn Quản lý kho
        sideMenu->item(3)->setHidden(true); // Ẩn Cài đặt
        
        // Ép nó vào thẳng trang Bán hàng (nằm ở vị trí số 2)
        sideMenu->setCurrentRow(2);
        stackedPages->setCurrentIndex(2);
        
        // Đổi tên cửa sổ app cho nhân viên biết thân biết phận :))
        setWindowTitle("Fahasa POS - Quầy Thu Ngân (Nhân viên)");
    } else {
        // Admin thì mặc định vào trang Tổng quan (vị trí 0)
        sideMenu->setCurrentRow(0);
        stackedPages->setCurrentIndex(0);
        setWindowTitle("Fahasa Hệ thống Quản trị (Quyền Admin)");
    }

    // --- NẠP DỮ LIỆU KHI MỞ APP ---
    manager.loadFromFile("database_sach.txt");
    refreshInventoryTable(); 
}

void MainWindow::setupUI() {
    // 1. TẠO THANH MENU BÊN TRÁI
    sideMenu = new QListWidget();
    sideMenu->setFixedWidth(200); // Chốt cứng chiều rộng thanh menu
    
    // Thêm các mục vào menu (Icon + Chữ sau này có thể thêm sau, giờ dùng chữ trước)
    sideMenu->addItem("📊 Tổng quan (Dashboard)");
    sideMenu->addItem("📦 Quản lý Kho sách");
    sideMenu->addItem("🛒 Bán hàng (POS)");

    // mới thêm 27/04
    sideMenu->addItem("📈 Báo cáo & Thống kê"); // <--- THÊM DÒNG NÀY (Menu thứ 4)
    // mới thêm 27/04

    sideMenu->addItem("⚙️ Cài đặt hệ thống");

    // 2. TẠO KHU VỰC CHỨA CÁC TRANG (STACKED WIDGET)
    stackedPages = new QStackedWidget();
    
    // Nhét các trang (đang làm giả) vào trong hộp
    stackedPages->addWidget(createDashboardPage());
    stackedPages->addWidget(createInventoryPage());
    stackedPages->addWidget(createSalesPage());

// mới thêm 27/04
    stackedPages->addWidget(createReportPage()); // <--- THÊM DÒNG NÀY (Trang Báo cáo)
// mới thêm 27/04

    stackedPages->addWidget(createSettingsPage());


    // 3. KẾT NỐI MENU VỚI TRANG
    // Khi người dùng bấm vào 1 dòng trên menu, trang tương ứng bên phải sẽ hiện ra
    connect(sideMenu, &QListWidget::currentRowChanged, stackedPages, &QStackedWidget::setCurrentIndex);

    // 4. BỐ CỤC TỔNG THỂ CỬA SỔ (Chia đôi màn hình Trái - Phải)
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(sideMenu);
    mainLayout->addWidget(stackedPages);
    
    // Mặc định chọn dòng đầu tiên (Dashboard) khi vừa mở app
    sideMenu->setCurrentRow(0); 
}

// --- CÁC HÀM TẠO TRANG GIẢ (MOCKUP) ---
// Tạm thời mình dùng dòng chữ bự để nhận biết đang ở trang nào. 
// Sau này chúng ta sẽ đập mấy dòng chữ này đi và thay bằng Bảng, Nút bấm thật!

// Sửa lại hàm này một chút: Truyền cái biến QLabel vào thay vì truyền chữ
QFrame* createStatCard(QString title, QLabel* valueLabel, QString color) {
    QFrame *card = new QFrame();
    card->setObjectName("StatCard"); 
    QVBoxLayout *layout = new QVBoxLayout(card);
    
    QLabel *lblTitle = new QLabel(title);
    lblTitle->setStyleSheet("color: gray; font-size: 14px;");
    
    // Set màu cho Label được truyền vào
    valueLabel->setStyleSheet(QString("color: %1; font-size: 24px; font-weight: bold;").arg(color));
    
    layout->addWidget(lblTitle);
    layout->addWidget(valueLabel);
    return card;
}

QWidget* MainWindow::createDashboardPage() {
    QWidget *page = new QWidget();
    
    // Chỉ khai báo QGridLayout 1 LẦN DUY NHẤT ở đây
    QGridLayout *layout = new QGridLayout(page);
    layout->setSpacing(20);

    // --- DÒNG 1: 4 Thẻ Thống kê ---
    lblDashTotalBooks = new QLabel("0");  // Đổi tên biến thêm chữ Dash
    lblDashTotalValue = new QLabel("0 đ"); // Đổi tên biến thêm chữ Dash
    lblDashTotalInvoices = new QLabel("0");  // Thêm biến mới
    lblDashTotalCustomers = new QLabel("0"); // Thêm biến mới

    layout->addWidget(createStatCard("Tổng số lượng tồn", lblDashTotalBooks, "#007BFF"), 0, 0); 
    layout->addWidget(createStatCard("Tổng giá trị kho", lblDashTotalValue, "#28A745"), 0, 1); 
    layout->addWidget(createStatCard("Tổng hóa đơn", lblDashTotalInvoices, "#FD7E14"), 0, 2); 
    layout->addWidget(createStatCard("Tổng khách hàng", lblDashTotalCustomers, "#6F42C1"), 0, 3);

/*
    // --- DÒNG 2: Biểu đồ (Trái) & Top Sách (Phải) ---
    QFrame *chartFrame = new QFrame();
    chartFrame->setObjectName("ContentCard");
    QVBoxLayout *chartLayout = new QVBoxLayout(chartFrame);
    
    // 1. Tạo tập dữ liệu (BarSet)
    QBarSet *set = new QBarSet("Số lượng tồn kho");
    
    // Lấy dữ liệu thực từ manager để vẽ
    QVector<Book> books = manager.getAllBooks();
    QStringList categories;
    int count = 0;
    for (const auto& b : books) {
        if (count >= 7) break; // Chỉ vẽ tối đa 7 cuốn cho đẹp
        *set << b.getQuantity();
        categories << b.getTitle().left(10) + "..."; // Cắt ngắn tên sách
        count++;
    }

    // 2. Đưa vào Series
    QBarSeries *series = new QBarSeries();
    series->append(set);

    // 3. Cấu hình Biểu đồ (Chart)
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Thống kê tồn kho các đầu sách");
    chart->setAnimationOptions(QChart::SeriesAnimations); // Hiệu ứng nhảy cột mượt mà

    // 4. Thiết lập trục X (Tên sách)
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    // 5. Thiết lập trục Y (Tự động co giãn theo số lượng thực tế)
    int maxQty = 10; // Mặc định tối thiểu
    for (const auto& b : books) {
        if (b.getQuantity() > maxQty) maxQty = b.getQuantity();
    }
    
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, maxQty + (maxQty * 0.2)); // Cho trục Y cao hơn cột cao nhất 20% cho đẹp
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // 6. Hiển thị lên View
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing); // Khử răng cưa
    chartView->setMinimumHeight(300);

    chartLayout->addWidget(chartView);
    layout->addWidget(chartFrame, 1, 0, 1, 2);

    QFrame *topBooksFrame = new QFrame();
    topBooksFrame->setObjectName("ContentCard");
    QVBoxLayout *topLayout = new QVBoxLayout(topBooksFrame);
    topLayout->addWidget(new QLabel("🔥 Top 5 sách bán chạy"));
    layout->addWidget(topBooksFrame, 1, 2, 1, 2); 
*/

// mới thêm 28/04
// --- DÒNG 2: Biểu đồ (Trái) & Top Sách (Phải) ---
    QFrame *chartFrame = new QFrame();
    chartFrame->setObjectName("ContentCard");
    QVBoxLayout *chartLayout = new QVBoxLayout(chartFrame);
    
    // Khởi tạo Biểu đồ (Chưa có dữ liệu, sẽ bơm vào sau)
    inventoryChart = new QChart();
    inventoryChart->setTitle("Thống kê tồn kho 10 đầu sách đầu tiên");
    inventoryChart->setAnimationOptions(QChart::SeriesAnimations);
    
    inventoryAxisX = new QBarCategoryAxis();
    inventoryChart->addAxis(inventoryAxisX, Qt::AlignBottom);
    
    inventoryAxisY = new QValueAxis();
    inventoryChart->addAxis(inventoryAxisY, Qt::AlignLeft);
    
    QChartView *chartView = new QChartView(inventoryChart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(300);
    chartLayout->addWidget(chartView);
    layout->addWidget(chartFrame, 1, 0, 1, 2);

    // Khởi tạo Bảng Top 10 Sách Bán Chạy
    QFrame *topBooksFrame = new QFrame();
    topBooksFrame->setObjectName("ContentCard");
    QVBoxLayout *topLayout = new QVBoxLayout(topBooksFrame);
    
    QLabel* lblTop = new QLabel("🔥 Top 10 Sách Bán Chạy Nhất");
    lblTop->setStyleSheet("font-size: 16px; font-weight: bold; color: #DC3545; margin-bottom: 5px;");
    topLayout->addWidget(lblTop);
    
    tableTopBooks = new QTableWidget(0, 3);
    tableTopBooks->setHorizontalHeaderLabels({"Hạng", "Tên sách", "Đã bán"});
    tableTopBooks->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tableTopBooks->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tableTopBooks->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tableTopBooks->setEditTriggers(QAbstractItemView::NoEditTriggers); // Khóa sửa tay
    tableTopBooks->setSelectionMode(QAbstractItemView::NoSelection); // Bỏ highlight khi click
    topLayout->addWidget(tableTopBooks);
    
    layout->addWidget(topBooksFrame, 1, 2, 1, 2);
// mới thêm 28/04

    // --- DÒNG 3: 2 Bảng Dữ liệu dưới cùng ---
    // (Đoạn table1 bên trái cứ giữ nguyên)
// BẮT ĐẦU THAY THẾ TỪ ĐÂY (Bảng bên trái)
    recentImportsTable = new QTableWidget(0, 4); // Đổi tên biến table1 thành recentImportsTable
    recentImportsTable->setHorizontalHeaderLabels({"Mã Phiếu", "Nhà cung cấp", "Ngày nhập", "Tổng tiền"});
    recentImportsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    recentImportsTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Khóa không cho sửa tay
    layout->addWidget(recentImportsTable, 2, 0, 1, 2);
    
    recentImportsTable->setHorizontalHeaderLabels({"Mã Phiếu", "Nhà cung cấp", "Ngày nhập", "Tổng tiền"});
    recentImportsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(recentImportsTable, 2, 0, 1, 2);

    // BẮT ĐẦU THAY THẾ TỪ ĐÂY (Bảng bên phải)
    recentSalesTable = new QTableWidget(0, 4); // Khởi tạo 0 dòng
    recentSalesTable->setHorizontalHeaderLabels({"Mã HĐ", "Khách hàng", "Ngày bán", "Tổng tiền"});
    recentSalesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    recentSalesTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Khóa không cho sửa tay
    layout->addWidget(recentSalesTable, 2, 2, 1, 2);

    return page;
}

QWidget* MainWindow::createInventoryPage() {
    QWidget *page = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(page);

    // --- KHU VỰC 1: Form nhập liệu ---
    QFrame *formFrame = new QFrame();
    formFrame->setObjectName("ContentCard"); // Dùng lại CSS bo góc màu trắng
    QFormLayout *formLayout = new QFormLayout(formFrame);
    
    idInput = new QLineEdit();
    titleInput = new QLineEdit();
    authorInput = new QLineEdit();
    quantityInput = new QLineEdit();
    priceInput = new QLineEdit();

    formLayout->addRow("Mã sách:", idInput);
    formLayout->addRow("Tên sách:", titleInput);
    formLayout->addRow("Tác giả:", authorInput);
    formLayout->addRow("Số lượng tồn:", quantityInput);
    formLayout->addRow("Giá bán (VNĐ):", priceInput);

// --- KHU VỰC 2: Các nút chức năng ---
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *btnAdd = new QPushButton("➕ Thêm sách mới");
    QPushButton *btnEdit = new QPushButton("✏️ Sửa sách đang chọn"); // Nút Sửa mới
    QPushButton *btnDelete = new QPushButton("❌ Xóa sách đang chọn");
    
    btnAdd->setStyleSheet("background-color: #28A745; color: white; padding: 8px; border-radius: 5px; font-weight: bold;");
    btnEdit->setStyleSheet("background-color: #FFC107; color: black; padding: 8px; border-radius: 5px; font-weight: bold;"); // Nút sửa màu vàng
    btnDelete->setStyleSheet("background-color: #DC3545; color: white; padding: 8px; border-radius: 5px; font-weight: bold;");

    buttonLayout->addWidget(btnAdd);
    buttonLayout->addWidget(btnEdit);
    buttonLayout->addWidget(btnDelete);
    formLayout->addRow("", buttonLayout);

    mainLayout->addWidget(formFrame);

    // --- KHU VỰC 3: Bảng danh sách ---

    // BẮT ĐẦU CHÈN TỪ ĐÂY (Tạo thanh tìm kiếm)
    searchInventoryInput = new QLineEdit();
    searchInventoryInput->setPlaceholderText("🔍 Nhập Mã sách hoặc Tên sách để tìm kiếm nhanh...");
    searchInventoryInput->setStyleSheet("padding: 10px; border: 2px solid #3182CE; border-radius: 5px; font-size: 14px; margin-bottom: 10px;");
    mainLayout->addWidget(searchInventoryInput);
    // KẾT THÚC CHÈN

    inventoryTable = new QTableWidget(0, 5);
    inventoryTable->setSelectionBehavior(QAbstractItemView::SelectRows); // Ép click chọn nguyên dòng chứ ko chọn 1 ô
    inventoryTable->setHorizontalHeaderLabels({"Mã sách", "Tên sách", "Tác giả", "Số lượng", "Giá bán"});
    inventoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mainLayout->addWidget(inventoryTable);

    // --- HÀM TIỆN ÍCH DỌN DẸP FORM ---
    auto clearForm = [this]() {
        idInput->clear(); titleInput->clear(); authorInput->clear();
        quantityInput->clear(); priceInput->clear();
        idInput->setReadOnly(false); // Mở khóa ô ID
        idInput->setStyleSheet("background-color: white; color: #2D3748;");
        inventoryTable->clearSelection();
    };

    // --- KẾT NỐI SỰ KIỆN: CLICK VÀO BẢNG TỰ AUTO-FILL ---
    connect(inventoryTable, &QTableWidget::itemSelectionChanged, this, [this, clearForm]() {
        int row = inventoryTable->currentRow();
        if (row >= 0) {
            // Đẩy dữ liệu lên form
            idInput->setText(inventoryTable->item(row, 0)->text());
            titleInput->setText(inventoryTable->item(row, 1)->text());
            authorInput->setText(inventoryTable->item(row, 2)->text());
            quantityInput->setText(inventoryTable->item(row, 3)->text());
            
            // Tách bỏ dấu chấm và chữ VNĐ để nạp lại số thuần túy vào ô nhập giá
            QString priceStr = inventoryTable->item(row, 4)->text();
            priceStr.replace(".", "").replace(" VNĐ", "");
            priceInput->setText(priceStr);

            // Khóa ô Mã sách lại (vì quy tắc là ID không được phép đổi)
            idInput->setReadOnly(true);
            idInput->setStyleSheet("background-color: #E2E8F0; color: #718096;"); // Đổi màu xám báo hiệu bị khóa
        } else {
            clearForm();
        }
    });

    // --- KẾT NỐI SỰ KIỆN CÁC NÚT BẤM ---
    connect(btnAdd, &QPushButton::clicked, this, [this, clearForm]() {
        if (idInput->text().isEmpty() || titleInput->text().isEmpty()) {
            QMessageBox::warning(this, "Lỗi", "Vui lòng nhập Mã sách và Tên sách!"); return;
        }
        manager.addBook(Book(idInput->text(), titleInput->text(), authorInput->text(), 
                             quantityInput->text().toInt(), priceInput->text().toDouble()));

        
        // mới thêm 28/04
        // =========================================================
        // CHÈN THÊM ĐOẠN NÀY: TẠO FILE PHIẾU NHẬP KHI THÊM SÁCH
        // =========================================================
        QString pnId = "PN" + QDateTime::currentDateTime().toString("yyMMdd_HHmmss");
        QFile pnFile(pnId + ".txt");
        if (pnFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&pnFile);
            out << "Ma Phieu: " << pnId << "\n";
            out << "Nha cung cap: NXB Van Hoc\n"; // Tạm để mặc định là NXB Văn Học
            out << "Ngay nhap: " << QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss") << "\n";
            
            // Tiền vốn nhập kho (Tính bằng 70% giá bán niêm yết)
            double importPrice = priceInput->text().toDouble() * 0.7;
            double totalImportValue = importPrice * quantityInput->text().toInt();
            QString formattedTotal = QLocale(QLocale::English).toString(totalImportValue, 'f', 0).replace(",", ".") + " VNĐ";
            
            out << "TONG TIEN: " << formattedTotal << "\n";
            pnFile.close();
        }
        // =========================================================

        // mới thêm 28/04
        loadImportHistory();
        refreshInventoryTable();
        clearForm();
    });

    connect(btnEdit, &QPushButton::clicked, this, [this, clearForm]() {
        int row = inventoryTable->currentRow();
        if (row < 0) {
            QMessageBox::warning(this, "Lỗi", "Vui lòng click chọn 1 cuốn sách trên bảng để sửa!"); return;
        }
        QString id = idInput->text(); // Lấy ID đang bị khóa trên form
        manager.updateBook(id, Book(id, titleInput->text(), authorInput->text(), 
                                    quantityInput->text().toInt(), priceInput->text().toDouble()));
        refreshInventoryTable();
        clearForm();
        QMessageBox::information(this, "Thành công", "Đã cập nhật thông tin sách!");
    });

    connect(btnDelete, &QPushButton::clicked, this, [this, clearForm]() {
        int row = inventoryTable->currentRow();
        if (row < 0) return;
        QString id = inventoryTable->item(row, 0)->text();
        manager.removeBook(id);
        refreshInventoryTable();
        clearForm();
    });

    // BẮT ĐẦU CHÈN TỪ ĐÂY (Logic Lọc dữ liệu Real-time)
    connect(searchInventoryInput, &QLineEdit::textChanged, this, [this](const QString &text) {
        for (int i = 0; i < inventoryTable->rowCount(); ++i) {
            // Kiểm tra xem Mã sách (cột 0) hoặc Tên sách (cột 1) có chứa chữ vừa gõ không
            bool matchId = inventoryTable->item(i, 0)->text().contains(text, Qt::CaseInsensitive);
            bool matchName = inventoryTable->item(i, 1)->text().contains(text, Qt::CaseInsensitive);
            
            // Nếu không khớp cái nào thì ẩn dòng đó đi (Hocus Pocus 🪄)
            inventoryTable->setRowHidden(i, !(matchId || matchName)); 
        }
    });
    // KẾT THÚC CHÈN

    return page;
}

// mới thêm 27/04
QWidget* MainWindow::createReportPage() {
    reportPage = new QWidget();
    QVBoxLayout* reportLayout = new QVBoxLayout(reportPage);

    // Tiêu đề tổng
    QLabel* title = new QLabel("<b>BÁO CÁO & THỐNG KÊ</b>");
    title->setStyleSheet("font-size: 22px; color: #2C3E50; margin-bottom: 10px;");
    
    lblTotalBooks = new QLabel("Tổng số sách trong kho: 0");
    lblTotalBooks->setStyleSheet("font-size: 16px; color: #34495e;");
    
    lblTotalValue = new QLabel("Tổng giá trị kho hàng: 0 VNĐ");
    lblTotalValue->setStyleSheet("font-size: 16px; color: #c0392b; font-weight: bold; margin-bottom: 15px;");
    
    reportLayout->addWidget(title);
    reportLayout->addWidget(lblTotalBooks);
    reportLayout->addWidget(lblTotalValue);

    // ================= TÁCH ĐÔI MÀN HÌNH =================
    QHBoxLayout* bodyLayout = new QHBoxLayout();
    
    // --- CỘT TRÁI: Bảng sách sắp hết ---
    QVBoxLayout* leftCol = new QVBoxLayout();
    QLabel* subTitle = new QLabel("⚠️ Sách sắp hết hàng (Tồn kho < 10):");
    subTitle->setStyleSheet("font-size: 16px; color: #d35400; font-weight: bold;");
    
    tableLowStock = new QTableWidget(0, 3);
    tableLowStock->setHorizontalHeaderLabels({"Mã sách", "Tên sách", "Tồn kho"});
    
    // Tắt tính năng ép cột cuối giãn ra
    tableLowStock->horizontalHeader()->setStretchLastSection(false);
    
    // Cột 0 (Mã sách): Tự động co vừa bằng với chữ "Mã sách"
    tableLowStock->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    
    // Cột 1 (Tên sách): Ép giãn tối đa chiếm hết mọi khoảng trống ở giữa
    tableLowStock->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    
    // Cột 2 (Tồn kho): Fix cứng độ rộng khoảng 100 pixel cho gọn gàng
    tableLowStock->setColumnWidth(2, 100);

    tableLowStock->setEditTriggers(QAbstractItemView::NoEditTriggers);
    leftCol->addWidget(subTitle);
    leftCol->addWidget(tableLowStock);

    // --- CỘT PHẢI: Kết quả kinh doanh ---
    QVBoxLayout* rightCol = new QVBoxLayout();
    rightCol->setAlignment(Qt::AlignTop); // Đẩy các phần tử lên trên cùng
    
    QLabel* rightTitle = new QLabel("📈 Kết Quả Kinh Doanh");
    rightTitle->setStyleSheet("font-size: 18px; color: #2C3E50; font-weight: bold; margin-bottom: 10px;");
    
    lblTotalSold = new QLabel("0");
    lblTotalRevenue = new QLabel("0 VNĐ");
    lblTotalCost = new QLabel("0 VNĐ"); // <--- Khởi tạo biến mới
    lblTotalProfit = new QLabel("0 VNĐ");

    // Tận dụng lại hàm createStatCard đã viết ở phần Dashboard!
    rightCol->addWidget(rightTitle);
    rightCol->addWidget(createStatCard("Số sách đã bán ra", lblTotalSold, "#007BFF"));  // Màu xanh dương
    rightCol->addWidget(createStatCard("Tổng doanh thu", lblTotalRevenue, "#28A745")); // Màu xanh lá
    // THÊM THẺ TIỀN SÁCH VÀO ĐÂY (Màu Tím cho đẹp)
    rightCol->addWidget(createStatCard("Tiền vốn sách (Ước tính 70%)", lblTotalCost, "#6F42C1"));
    rightCol->addWidget(createStatCard("Tiền lời (Ước tính)", lblTotalProfit, "#FD7E14")); // Màu cam
    rightCol->addStretch(); // Tránh bị giãn thẻ khi phóng to màn hình

    // Gắn 2 cột vào Body (Tỉ lệ 6 - 4)
    bodyLayout->addLayout(leftCol, 6); 
    bodyLayout->addLayout(rightCol, 4); 

    reportLayout->addLayout(bodyLayout);

    // Nút làm mới
    QPushButton* btnRefresh = new QPushButton("🔄 Làm mới dữ liệu");
    btnRefresh->setStyleSheet("background-color: #007bff; color: white; padding: 10px; border-radius: 5px; font-weight: bold; font-size: 14px; margin-top: 10px;");
    connect(btnRefresh, &QPushButton::clicked, this, &MainWindow::refreshReportData);
    reportLayout->addWidget(btnRefresh);
    
    refreshReportData(); 
    return reportPage;
}
// mới thêm 27/04

// 1. HÀM CẬP NHẬT SÁCH LÊN QUẦY
void MainWindow::refreshSalesBookTable() {
    salesBookTable->setRowCount(0);
    QVector<Book> books = manager.getAllBooks();
    for (int i = 0; i < books.size(); ++i) {
        salesBookTable->insertRow(i);
        salesBookTable->setItem(i, 0, new QTableWidgetItem(books[i].getId()));
        salesBookTable->setItem(i, 1, new QTableWidgetItem(books[i].getTitle()));
        salesBookTable->setItem(i, 2, new QTableWidgetItem(QString::number(books[i].getQuantity())));
        
        QString priceStr = QLocale(QLocale::English).toString(books[i].getPrice(), 'f', 0).replace(",", ".") + " VNĐ";
        salesBookTable->setItem(i, 3, new QTableWidgetItem(priceStr));
    }
}

// 2. HÀM TÍNH LẠI TỔNG TIỀN HÓA ĐƠN (CÓ TÍNH CHIẾT KHẤU VIP)
void MainWindow::updateCartTotal() {
    double total = 0;
    for(int i = 0; i < cartTable->rowCount(); ++i) {
        QString id = cartTable->item(i, 0)->text();
        int qty = cartTable->item(i, 2)->text().toInt();
        Book* b = manager.findBookById(id);
        if(b) total += (b->getPrice() * qty);
    }

    // 1. Tính toán chiết khấu các loại
    double vipDiscountAmount = total * currentDiscount;
    double voucherDiscountAmount = (total * voucherPercent) + voucherFlat;
    
    double finalTotal = total - vipDiscountAmount - voucherDiscountAmount;
    if (finalTotal < 0) finalTotal = 0; // Chống bug âm tiền

    // 2. Định dạng chuỗi
    QString formattedFinal = QLocale(QLocale::English).toString(finalTotal, 'f', 0).replace(",", ".") + " VNĐ";

    // 3. Hiển thị UI có gạch ngang nếu CÓ BẤT KỲ KHUYẾN MÃI NÀO (VIP hoặc Voucher)
    if (vipDiscountAmount > 0 || voucherDiscountAmount > 0) {
        QString formattedOld = QLocale(QLocale::English).toString(total, 'f', 0).replace(",", ".");
        
        // Tạo thêm dòng chữ báo hiệu áp dụng Voucher
        QString voucherAlert = "";
        if (voucherDiscountAmount > 0) {
            voucherAlert = "<br/><span style='color:#28A745; font-size:14px;'>🎟️ Đã áp dụng Voucher giảm giá!</span>";
        }

        // Ép mã HTML để gạch ngang giá cũ và in đậm giá mới
        lblCartTotal->setText(QString("<s style='color:#868E96; font-size:18px;'>%1 VNĐ</s>%2<br/> <b style='color:#C92127;'>%3</b>")
                              .arg(formattedOld).arg(voucherAlert).arg(formattedFinal));
    } else {
        // Không có khuyến mãi thì in giá bình thường
        lblCartTotal->setText(QString("<b style='color:#C92127;'>%1</b>").arg(formattedFinal));
    }
}

// 3. TẠO GIAO DIỆN TRANG BÁN HÀNG
QWidget* MainWindow::createSalesPage() {
    QWidget *page = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(page); // Chia ngang
    
    // ================= BÊN TRÁI: DANH SÁCH SÁCH =================
    QFrame *leftFrame = new QFrame();
    leftFrame->setObjectName("ContentCard"); // Dùng lại CSS bo góc
    QVBoxLayout *leftLayout = new QVBoxLayout(leftFrame);
    
    QLabel *lblLeftTitle = new QLabel("📚 Danh sách sản phẩm (Click đúp để thêm vào giỏ)");
    lblLeftTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #2B6CB0;");
    leftLayout->addWidget(lblLeftTitle);

    // BẮT ĐẦU CHÈN TỪ ĐÂY
    searchSalesInput = new QLineEdit();
    searchSalesInput->setPlaceholderText("🔍 Quét mã vạch hoặc gõ tên sách...");
    searchSalesInput->setStyleSheet("padding: 10px; border: 2px solid #28A745; border-radius: 5px; margin-bottom: 10px;");
    leftLayout->addWidget(searchSalesInput);
    // KẾT THÚC CHÈN
    
    salesBookTable = new QTableWidget(0, 4);
    salesBookTable->setHorizontalHeaderLabels({"Mã", "Tên sách", "Tồn kho", "Giá bán"});
    salesBookTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    salesBookTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // KHÓA: Không cho sửa text trực tiếp
    salesBookTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    leftLayout->addWidget(salesBookTable);
    
    // ================= BÊN PHẢI: GIỎ HÀNG =================
    QFrame *rightFrame = new QFrame();
    rightFrame->setObjectName("ContentCard");
    rightFrame->setFixedWidth(500); // Chốt cứng chiều rộng khu tính tiền
    QVBoxLayout *rightLayout = new QVBoxLayout(rightFrame);
    
    QLabel *lblRightTitle = new QLabel("🛒 Hóa đơn thanh toán");
    lblRightTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #2B6CB0;");
    rightLayout->addWidget(lblRightTitle);
    
    cartTable = new QTableWidget(0, 5);
    cartTable->setHorizontalHeaderLabels({"Mã", "Tên", "SL", "Đơn giá", "Thành tiền"});
    cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cartTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    cartTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    cartTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers); // Khóa gõ phím trực tiếp
    rightLayout->addWidget(cartTable);

    // BẮT ĐẦU CHÈN TỪ ĐÂY: --- KHU VỰC KHÁCH HÀNG VIP ---
    QGroupBox *groupVIP = new QGroupBox("👑 Khách hàng thân thiết");
    groupVIP->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #EAEAEA; border-radius: 8px; margin-top: 10px; padding-top: 15px; } QGroupBox::title { color: #C92127; subcontrol-origin: margin; left: 10px; padding: 0 5px; }");
    QVBoxLayout *vipLayout = new QVBoxLayout(groupVIP);
    
    QHBoxLayout *phoneLayout = new QHBoxLayout();
    txtCustomerPhone = new QLineEdit();
    txtCustomerPhone->setPlaceholderText("Nhập SĐT khách hàng (VD: 0999999999)...");
    QPushButton *btnCheckVIP = new QPushButton("🔍 Check");
    btnCheckVIP->setStyleSheet("background-color: #343A40; color: white; padding: 8px 15px; border-radius: 5px; font-weight: bold;"); // Nút đen ngầu
    
    // BẮT ĐẦU CHÈN THÊM TỪ ĐÂY: Nút Cấp Thẻ
    QPushButton *btnIssueCard = new QPushButton("💳 Cấp thẻ VIP");
    btnIssueCard->setStyleSheet("background-color: #FFC107; color: black; padding: 8px 15px; border-radius: 5px; font-weight: bold;");
    
    phoneLayout->addWidget(txtCustomerPhone);
    phoneLayout->addWidget(btnCheckVIP);
    phoneLayout->addWidget(btnIssueCard); // Đưa nút vào giao diện
    // KẾT THÚC CHÈN
    
    lblCustomerInfo = new QLabel("Khách lẻ (Không có chiết khấu)");
    lblCustomerInfo->setStyleSheet("color: #6C757D; font-style: italic; font-size: 13px;");
 
// 28/04
// --- CHÈN THÊM KHU VỰC VOUCHER VÀO DƯỚI KHU VỰC VIP ---
    QHBoxLayout *voucherLayout = new QHBoxLayout();
    txtVoucher = new QLineEdit();
    txtVoucher->setPlaceholderText("Nhập mã Voucher...");
    QPushButton *btnApplyVoucher = new QPushButton("🏷️ Áp dụng");
    btnApplyVoucher->setStyleSheet("background-color: #E2E8F0; color: black; padding: 8px; font-weight: bold; border-radius: 5px;");
    
    voucherLayout->addWidget(txtVoucher);
    voucherLayout->addWidget(btnApplyVoucher);
// 28/04

    vipLayout->addLayout(phoneLayout);
    vipLayout->addWidget(lblCustomerInfo);

// 28/04
    vipLayout->addLayout(voucherLayout); // <--- Add cái layout voucher vào Form
// 28/04

    rightLayout->addWidget(groupVIP);
    // KẾT THÚC CHÈN
    
    // Khu vực Tổng tiền
    QHBoxLayout *totalLayout = new QHBoxLayout();
    QLabel *lblTotalText = new QLabel("Tổng cộng:");
    lblTotalText->setStyleSheet("font-size: 20px; font-weight: bold;");
    lblCartTotal = new QLabel("0 VNĐ");
    lblCartTotal->setStyleSheet("font-size: 26px; font-weight: bold; color: #DC3545;"); // Số to, màu đỏ
    totalLayout->addWidget(lblTotalText);
    totalLayout->addStretch();
    totalLayout->addWidget(lblCartTotal);
    rightLayout->addLayout(totalLayout);
    
    // Nút chức năng
    QPushButton *btnCheckout = new QPushButton("💳 THANH TOÁN (TRỪ KHO)");
    btnCheckout->setStyleSheet("background-color: #28A745; color: white; padding: 15px; font-size: 16px; font-weight: bold; border-radius: 8px;");
    QPushButton *btnClearCart = new QPushButton("🗑️ Làm mới giỏ");
    btnClearCart->setStyleSheet("background-color: #6C757D; color: white; padding: 10px; font-weight: bold; border-radius: 5px;");
    
    rightLayout->addWidget(btnCheckout);
    rightLayout->addWidget(btnClearCart);
    
    mainLayout->addWidget(leftFrame);
    mainLayout->addWidget(rightFrame);
    
    // ================= LOGIC KẾT NỐI SỰ KIỆN =================
    
    // SỰ KIỆN 1: Click đúp vào sách bên trái để chui vào giỏ hàng
    connect(salesBookTable, &QTableWidget::itemDoubleClicked, this, [this](QTableWidgetItem *item) {
        int row = item->row();
        QString id = salesBookTable->item(row, 0)->text();
        QString title = salesBookTable->item(row, 1)->text();
        int stock = salesBookTable->item(row, 2)->text().toInt();
        
        Book* book = manager.findBookById(id);
        if (!book) return;
        if (stock <= 0) { QMessageBox::warning(this, "Hết hàng", "Sách này đã hết hàng!"); return; }
        
        // Quét xem sách đã có trong giỏ chưa. Có rồi thì +1 số lượng
        bool found = false;
        for(int i = 0; i < cartTable->rowCount(); ++i) {
            if (cartTable->item(i, 0)->text() == id) {
                int currentQty = cartTable->item(i, 2)->text().toInt();
                if (currentQty >= stock) {
                    QMessageBox::warning(this, "Lỗi", "Không đủ số lượng trong kho!"); return;
                }
                cartTable->item(i, 2)->setText(QString::number(currentQty + 1));
                double total = (currentQty + 1) * book->getPrice();
                cartTable->item(i, 4)->setText(QLocale(QLocale::English).toString(total, 'f', 0).replace(",", ".") + " VNĐ");
                found = true; break;
            }
        }
        
        // Nếu chưa có, thêm dòng mới vào giỏ
        if (!found) {
            int r = cartTable->rowCount();
            cartTable->insertRow(r);
            cartTable->setItem(r, 0, new QTableWidgetItem(id));
            cartTable->setItem(r, 1, new QTableWidgetItem(title));
            cartTable->setItem(r, 2, new QTableWidgetItem("1"));
            QString priceStr = QLocale(QLocale::English).toString(book->getPrice(), 'f', 0).replace(",", ".") + " VNĐ";
            cartTable->setItem(r, 3, new QTableWidgetItem(priceStr)); 
            cartTable->setItem(r, 4, new QTableWidgetItem(priceStr)); 
        }
        updateCartTotal(); // Tính lại tiền
    });
    
    // SỰ KIỆN 2: Xóa giỏ hàng
    connect(btnClearCart, &QPushButton::clicked, this, [this]() {
        cartTable->setRowCount(0);
        updateCartTotal();
    });
    
    // SỰ KIỆN 3: Thanh toán & In Hóa Đơn
    connect(btnCheckout, &QPushButton::clicked, this, [this]() {
        if(cartTable->rowCount() == 0) { QMessageBox::warning(this, "Trống", "Giỏ hàng đang trống!"); return; }
        
        // 1. TẠO FORMAT HÓA ĐƠN CHUẨN SIÊU THỊ
        QString billId = "HD" + QDateTime::currentDateTime().toString("yyMMdd_HHmmss");
        QString billContent;
        QTextStream stream(&billContent);
        
        stream << "================================================\n";
        stream << "             NHA SACH PHU THANH PRO             \n";
        stream << "           Khu Cong Nghe Cao, TP.HCM            \n";
        stream << "================================================\n";
        stream << "Ma HD: " << billId << "\n";
        stream << "Ngay in: " << QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss") << "\n";

        // --- THÊM ĐÚNG 1 DÒNG NÀY ĐỂ IN TÊN THU NGÂN ---
        stream << "Thu ngan: " << currentUserRole << "\n";

    // mới thêm 27/04
        if (txtCustomerPhone->text().isEmpty()) {
                stream << "Khach hang: Khach vang lai\n";
        } else {
                stream << "Khach hang: " << txtCustomerPhone->text() << "\n";
        }

    // mới thêm 27/04



        stream << "------------------------------------------------\n";
        // Căn chỉnh cột: Tên (25 ký tự), SL (5 ký tự), Thành tiền (15 ký tự)
        stream << QString("%1 %2 %3\n").arg("Ten san pham", -25).arg("SL", -5).arg("Thanh tien", 15);
        stream << "------------------------------------------------\n";

        // 2. QUÉT GIỎ HÀNG: TRỪ KHO VÀ GHI VÀO HÓA ĐƠN
        for(int i = 0; i < cartTable->rowCount(); ++i) {
            QString id = cartTable->item(i, 0)->text();
            QString name = cartTable->item(i, 1)->text();
            int qty = cartTable->item(i, 2)->text().toInt();
            QString itemTotal = cartTable->item(i, 4)->text();
            
            // Trừ kho
            Book* b = manager.findBookById(id);
            if(b) {
                Book updatedBook = *b;
                updatedBook.setQuantity(b->getQuantity() - qty);
                manager.updateBook(id, updatedBook);
            }

            // Xử lý chuỗi tên sách: Nếu dài quá thì cắt bớt cho khỏi vỡ form hóa đơn
            QString shortName = name.length() > 22 ? name.left(19) + "..." : name;
            stream << QString("%1 %2 %3\n").arg(shortName, -25).arg(qty, -5).arg(itemTotal, 15);
        }
        
        stream << "------------------------------------------------\n";

        // TÍNH LẠI TỔNG TIỀN "SẠCH" ĐỂ IN FILE (Không xài HTML)
        double totalToPrint = 0;
        for(int i = 0; i < cartTable->rowCount(); ++i) {
            QString itemTotalStr = cartTable->item(i, 4)->text(); 
            itemTotalStr.remove(" VNĐ").remove("."); 
            totalToPrint += itemTotalStr.toDouble();
        }
        
        // Chuẩn bị các con số
        double discountAmount = totalToPrint * currentDiscount;
        double finalTotalToPrint = totalToPrint - discountAmount;

        // Định dạng thành chuỗi tiền tệ (VD: 1.000.000 VNĐ)
        QString subTotalStr = QLocale(QLocale::English).toString(totalToPrint, 'f', 0).replace(",", ".") + " VNĐ";
        QString discountStr = "-" + QLocale(QLocale::English).toString(discountAmount, 'f', 0).replace(",", ".") + " VNĐ";
        QString finalTotalStr = QLocale(QLocale::English).toString(finalTotalToPrint, 'f', 0).replace(",", ".") + " VNĐ";

        // GHI VÀO HÓA ĐƠN
        if (currentDiscount > 0) {
            // Nếu là khách VIP có chiết khấu -> In chi tiết 3 dòng
            stream << QString("%1 %2\n").arg("TAM TINH:", -31).arg(subTotalStr, 15);
            stream << QString("%1 %2\n").arg(QString("CHIET KHAU VIP (%1%):").arg(currentDiscount * 100), -31).arg(discountStr, 15);
            stream << "------------------------------------------------\n";
            stream << QString("%1 %2\n").arg("TONG THANH TOAN:", -31).arg(finalTotalStr, 15);
        } else {
            // Khách lẻ bình thường -> Chỉ in 1 dòng Tổng cộng
            stream << QString("%1 %2\n").arg("TONG CONG:", -31).arg(subTotalStr, 15);
        }
    
        stream << "================================================\n";
        stream << "         Cam on quy khach & Hen gap lai!        \n";

        // 3. XUẤT RA FILE TXT


// mới thêm 27/04
// 3. XUẤT RA FILE TXT
        QFile billFile(billId + ".txt");
        if (billFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&billFile);
            out << billContent; // Bơm toàn bộ nội dung hóa đơn vào file
            billFile.close();
        }
// mới thêm 27/04




// ==========================================================
        // HỆ THỐNG CỘNG DỒN ĐIỂM & TỰ ĐỘNG THĂNG HẠNG KIM CƯƠNG
        // ==========================================================
        QString customerPhone = txtCustomerPhone->text();
        if (!customerPhone.isEmpty()) {
            QList<QString> lines;
            int oldPoints = 0;

            // 1. Quét danh sách cũ để tìm khách này có bao nhiêu điểm rồi
            QFile readFile("customers.txt");
            if (readFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&readFile);
                while (!in.atEnd()) {
                    QString line = in.readLine();
                    QStringList parts = line.split("|");
                    if (parts.size() >= 2) {
                        if (parts[0] == customerPhone) {
                            oldPoints = parts[1].toInt(); // Lấy điểm cũ
                        } else {
                            lines.append(line); // Lưu lại các khách hàng khác
                        }
                    }
                }
                readFile.close();
            }

            // 2. Tính điểm mới kiếm được từ hóa đơn này (10k = 1 điểm)
            int earnedPoints = finalTotalToPrint / 10000;
            if (earnedPoints == 0 && finalTotalToPrint > 0) earnedPoints = 1; // Mua ít nhất cũng tặng 1 điểm
            
            int newTotalPoints = oldPoints + earnedPoints; // Cộng dồn

            // 3. Ghi đè file với số điểm đã được cập nhật
            QFile writeFile("customers.txt");
            if (writeFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&writeFile);
                for (const QString& l : lines) {
                    out << l << "\n";
                }
                out << customerPhone << "|" << newTotalPoints << "\n"; // Lấy điểm mới nhất
                writeFile.close();
            }

            // ==========================================================
            // 4. BÙM! LOGIC TỰ ĐỘNG IN THẺ KIM CƯƠNG NỀN TRẮNG
            // ==========================================================
            if (oldPoints < 3000 && newTotalPoints >= 3000) {
                
                QPixmap card(600, 350);
                card.fill(Qt::white); // Ép nền trắng 100%

                QPainter painter(&card);
                painter.setRenderHint(QPainter::Antialiasing);

                // 1. Vẽ khung viền Xanh Kim Cương (Cyan) cho nổi trên nền trắng
                QPen diamondPen(QColor("#00B8D4")); // Xanh đậm hơn tí cho rõ
                diamondPen.setWidth(8);
                painter.setPen(diamondPen);
                painter.drawRect(10, 10, 580, 330);

                // 2. Tiêu đề màu Xanh Đậm Premium
                painter.setPen(QColor("#1A237E")); 
                painter.setFont(QFont("Arial", 30, QFont::Bold));
                painter.drawText(card.rect(), Qt::AlignTop | Qt::AlignHCenter, "\nFAHASA DIAMOND");

                // 3. Thông tin khách hàng - PHẢI DÙNG MÀU ĐEN/XÁM ĐẬM
                painter.setPen(QColor("#2D3748")); // Màu xám đen sang trọng
                painter.setFont(QFont("Arial", 16, QFont::Bold));
                painter.drawText(50, 160, "TÊN KHÁCH: KHÁCH VIP ĐẶC QUYỀN");
                painter.drawText(50, 210, "SĐT: " + customerPhone);
                
                // 4. Hạng thẻ dùng màu Xanh Diamond làm điểm nhấn
                painter.setPen(QColor("#00B8D4")); 
                painter.drawText(50, 260, "HẠNG THẺ: KIM CƯƠNG (DIAMOND)");

                // 5. Vẽ Mã Vạch màu ĐEN (Trên nền trắng phải dùng màu đen)
                painter.setPen(QPen(Qt::black, 3)); 
                for(int i = 0; i < 60; i++) {
                    if(rand() % 3 != 0) painter.drawLine(100 + i * 5, 290, 100 + i * 5, 320); 
                }
                painter.end();

                // Lưu thẻ
                QString fileName = "Auto_White_Diamond_" + customerPhone + ".png";
                card.save(fileName);
                
                // Bật Popup ăn mừng (Tui đã dặn ở Phase 1 là ép CSS cho nó trắng rồi đó)
                QMessageBox::information(this, "🎉 CHÚC MỪNG LÊN HẠNG!", 
                    QString("Khách hàng %1 vừa tích đủ %2 điểm!\n\n"
                            "Hệ thống đã tự động nâng cấp lên Hạng Kim Cương.\n"
                            "Kiểm tra file thẻ trắng: %3").arg(customerPhone).arg(newTotalPoints).arg(fileName));
            }
        }
        
        // Đoạn code làm sạch giỏ hàng (giữ nguyên của ông)
        // cartTable->setRowCount(0);
        // ...
        
        // 4. DỌN DẸP VÀ ĐỒNG BỘ
        cartTable->setRowCount(0);
        updateCartTotal();
        refreshInventoryTable();  // Cập nhật lại kho
        refreshSalesBookTable();  // Cập nhật lại số lượng hiển thị trên quầy

        if (!customerPhone.isEmpty()) {
            // 1. Mở file khách hàng ở chế độ Append (Ghi thêm vào cuối)
            QFile file("customers.txt");
            if (file.open(QIODevice::Append | QIODevice::Text)) {
                QTextStream out(&file);
                // Lưu tạm định dạng: SĐT | Điểm (mặc định cho 10 điểm mỗi đơn)
                out << customerPhone << "|10\n"; 
                file.close();
            }
        }
    });

    // SỰ KIỆN 4: Quét SĐT Khách hàng từ file customers.txt
    connect(btnCheckVIP, &QPushButton::clicked, this, [this]() {
        QString phone = txtCustomerPhone->text();
        if (phone.isEmpty()) {
            QMessageBox::warning(this, "Trống", "Vui lòng nhập số điện thoại!");
            return;
        }

        bool found = false;
        int currentPoints = 0;

        // Mở file ra quét từng dòng
        QFile file("customers.txt");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                QStringList parts = line.split("|");
                if (parts.size() >= 2 && parts[0] == phone) {
                    currentPoints = parts[1].toInt();
                    found = true;
                    break; // Tìm thấy thì dừng quét luôn cho lẹ
                }
            }
            file.close();
        }

        if (found) {
            // Nạp dữ liệu vào class Customer để nó tự xếp hạng
            Customer c(phone, currentPoints);
            
            QString rankInfo = QString("⭐ Khách hàng: %1 - Điểm: %2\n").arg(c.getRank()).arg(c.getPoints());
            
            // Xử lý giảm giá theo hạng mới (Đồng 3%, Bạc 5%, Vàng 7%, Kim Cương 10%)
            if (c.getRank() == "Kim Cương") {
                rankInfo += "💎 Áp dụng mức chiết khấu Kim Cương 10%!";
                lblCustomerInfo->setStyleSheet("color: #9B59B6; font-weight: bold; font-size: 14px;"); // Màu Tím Sang Trọng
                currentDiscount = 0.10;
            } 
            else if (c.getRank() == "Vàng") {
                rankInfo += "🎉 Áp dụng mức chiết khấu Vàng 7%!";
                lblCustomerInfo->setStyleSheet("color: #FFC107; font-weight: bold; font-size: 14px;"); // Màu Vàng Gold
                currentDiscount = 0.07;
            } 
            else if (c.getRank() == "Bạc") {
                rankInfo += "🎉 Áp dụng mức chiết khấu Bạc 5%!";
                lblCustomerInfo->setStyleSheet("color: #17A2B8; font-weight: bold; font-size: 14px;"); // Màu Xanh Bạc
                currentDiscount = 0.05;
            } 
            else { // Mặc định là hạng Đồng
                rankInfo += "🎉 Áp dụng mức chiết khấu Đồng 3%!";
                lblCustomerInfo->setStyleSheet("color: #D35400; font-weight: bold; font-size: 14px;"); // Màu Đồng
                currentDiscount = 0.03;
            }
            
            lblCustomerInfo->setText(rankInfo);

        } else {
            lblCustomerInfo->setText("Khách hàng mới (Sẽ tự động tạo thẻ sau khi thanh toán)");
            lblCustomerInfo->setStyleSheet("color: #007BFF; font-weight: bold; font-size: 13px;");
            currentDiscount = 0.0;
        }
        
        updateCartTotal(); // Tính lại tổng tiền ngay lập tức
    });
    
    // SỰ KIỆN NÚT THANH TOÁN (Sửa nhẹ: Reset luôn khách VIP sau khi thanh toán xong)
    // TÌM ĐOẠN "cartTable->setRowCount(0);" trong sự kiện btnCheckout và THÊM vào dưới nó:
    /* cartTable->setRowCount(0);
       txtCustomerPhone->clear();
       currentDiscount = 0.0;
       lblCustomerInfo->setText("Khách lẻ (Không có chiết khấu)");
       lblCustomerInfo->setStyleSheet("color: #6C757D; font-style: italic; font-size: 13px;");
       updateCartTotal();
    */

    // SỰ KIỆN 5: Bấm nút Cấp Thẻ VIP (Dành riêng cho Khách Kim Cương)
    connect(btnIssueCard, &QPushButton::clicked, this, [this]() {
        QString phone = txtCustomerPhone->text();
        if (phone.isEmpty()) {
            QMessageBox::warning(this, "Lỗi", "Khách chưa đọc Số điện thoại thì in thẻ cho ai hả nhân viên kia?");
            return;
        }

        // 1. GHI DATABASE: Tặng thẳng 3000 điểm để lên mốc Kim Cương!
        QFile file("customers.txt");
        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&file);
            out << phone << "|3000\n"; 
            file.close();
        }

        // 2. HỌA SĨ QPAINTER: Vẽ tấm thẻ Kim Cương siêu cấp
        QPixmap card(600, 350);
        card.fill(QColor("#1A1A1A")); // Nền Đen Nhám cực ngầu và sang trọng

        QPainter painter(&card);
        painter.setRenderHint(QPainter::Antialiasing); // Khử răng cưa

        // Vẽ Khung viền màu Xanh Kim Cương (Diamond Cyan)
        QPen diamondPen(QColor("#00E5FF"));
        diamondPen.setWidth(6);
        painter.setPen(diamondPen);
        painter.drawRect(15, 15, 570, 320);

        // Vẽ Tiêu đề Thẻ
        painter.setFont(QFont("Arial", 28, QFont::Bold));
        painter.drawText(card.rect(), Qt::AlignTop | Qt::AlignHCenter, "\nFAHASA DIAMOND");

        // Vẽ Thông tin khách hàng
        painter.setPen(Qt::white); 
        painter.setFont(QFont("Arial", 16, QFont::Bold));
        painter.drawText(50, 160, "TÊN KHÁCH: KHÁCH HÀNG VIP");
        painter.drawText(50, 210, "SĐT: " + phone);
        
        // Đổi màu chữ Hạng thẻ cho lấp lánh
        painter.setPen(QColor("#00E5FF")); 
        painter.drawText(50, 260, "HẠNG THẺ: KIM CƯƠNG (DIAMOND)");

        // Vẽ Mã Vạch (Barcode) trắng ở đáy thẻ
        painter.setPen(QPen(Qt::white, 3)); 
        for(int i = 0; i < 60; i++) {
            if(rand() % 3 != 0) { 
                painter.drawLine(100 + i * 5, 290, 100 + i * 5, 320); 
            }
        }
        painter.end(); // Cất cọ

        // 3. Xuất file ảnh ra ổ cứng
        QString fileName = "TheDiamond_" + phone + ".png";
        card.save(fileName);
        QMessageBox::information(this, "Thành công", "Đã cấp thẻ KIM CƯƠNG thành công!\nKiểm tra file: " + fileName);
    });

    // SỰ KIỆN 6: SỬA SỐ LƯỢNG TRỰC TIẾP TRONG GIỎ HÀNG
    connect(cartTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int column) {
        // Chỉ cho phép sửa khi click đúng vào CỘT SỐ LƯỢNG (Cột số 2)
        if (column != 2) return;

        QString id = cartTable->item(row, 0)->text();
        QString name = cartTable->item(row, 1)->text();
        int currentQty = cartTable->item(row, 2)->text().toInt();

        // Tìm sách trong kho để biết tồn kho tối đa là bao nhiêu
        Book* b = manager.findBookById(id);
        if (!b) return;
        int maxQty = b->getQuantity();

        // Bật bảng hỏi số lượng y như lúc Thêm vào giỏ
        bool ok;
        int newQty = QInputDialog::getInt(this, "Sửa số lượng",
                                          "Nhập số lượng mới cho:\n" + name,
                                          currentQty, 1, maxQty, 1, &ok);
        
        // Nếu bấm Ok và số lượng có thay đổi
        if (ok && newQty != currentQty) {
            // 1. Cập nhật con số mới lên giao diện
            cartTable->item(row, 2)->setText(QString::number(newQty));

            // 2. Lấy đơn giá và Tính lại Thành tiền của dòng đó
            QString priceStr = cartTable->item(row, 3)->text();
            double price = priceStr.remove(" VNĐ").remove(".").toDouble();
            
            double itemTotal = price * newQty;
            QString formattedItemTotal = QLocale(QLocale::English).toString(itemTotal, 'f', 0).replace(",", ".") + " VNĐ";
            cartTable->item(row, 4)->setText(formattedItemTotal);

            // 3. Tự động cộng lại cục Bill đỏ chót ở dưới cùng
            updateCartTotal();
        }
    });

// 28/04
    // --- LOGIC XỬ LÝ KHI BẤM NÚT ÁP DỤNG VOUCHER ---
    connect(btnApplyVoucher, &QPushButton::clicked, this, [this]() {
        QString code = txtVoucher->text().trimmed();
        if(code.isEmpty()) return;
        
        QFile file("vouchers.txt"); // Đọc file chứa voucher
        bool found = false;
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while(!in.atEnd()) {
                QStringList parts = in.readLine().split("|");
                if(parts.size() == 2 && parts[0] == code) {
                    double val = parts[1].toDouble();
                    if(val < 1.0) { voucherPercent = val; voucherFlat = 0.0; } // Dưới 1.0 -> Giảm %
                    else { voucherFlat = val; voucherPercent = 0.0; }          // Trên 1.0 -> Giảm thẳng tiền
                    found = true;
                    QMessageBox::information(this, "Thành công", "Áp dụng Voucher thành công!");
                    break;
                }
            }
            file.close();
        }
        if(!found) {
            QMessageBox::warning(this, "Lỗi", "Mã giảm giá không hợp lệ hoặc đã hết hạn!");
            voucherPercent = 0.0; voucherFlat = 0.0;
        }
        updateCartTotal(); // Tính lại tiền
    });
// 28/04

    // Logic tìm kiếm cho Quầy Bán hàng
    connect(searchSalesInput, &QLineEdit::textChanged, this, [this](const QString &text) {
        for (int i = 0; i < salesBookTable->rowCount(); ++i) {
            bool matchId = salesBookTable->item(i, 0)->text().contains(text, Qt::CaseInsensitive);
            bool matchName = salesBookTable->item(i, 1)->text().contains(text, Qt::CaseInsensitive);
            salesBookTable->setRowHidden(i, !(matchId || matchName));
        }
    });
    
    return page;
}

QWidget* MainWindow::createSettingsPage() {
    QWidget *page = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(page);
    mainLayout->setSpacing(20);

    // --- TIÊU ĐỀ TRANG ---
    QLabel *lblTitle = new QLabel("⚙️ Cài đặt Hệ thống");
    lblTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2B6CB0;");
    mainLayout->addWidget(lblTitle);

    // --- KHỐI 1: THÔNG TIN CỬA HÀNG ---
    QGroupBox *groupStore = new QGroupBox("🏢 Thông tin Cửa hàng");
    groupStore->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #CBD5E0; border-radius: 5px; margin-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; color: #2B6CB0; }");
    QFormLayout *storeLayout = new QFormLayout(groupStore);
    
    QLineEdit *txtStoreName = new QLineEdit("Nhà Sách Phú Thành");
    QLineEdit *txtAddress = new QLineEdit("Khu Công Nghệ Cao, TP.HCM");
    QLineEdit *txtPhone = new QLineEdit("0379.701.715");
    QPushButton *btnSaveStore = new QPushButton("💾 Lưu thông tin");
    btnSaveStore->setStyleSheet("background-color: #3182CE; color: white; padding: 8px; border-radius: 5px; font-weight: bold;");
    
    storeLayout->addRow("Tên cửa hàng:", txtStoreName);
    storeLayout->addRow("Địa chỉ:", txtAddress);
    storeLayout->addRow("Số điện thoại:", txtPhone);
    storeLayout->addRow("", btnSaveStore);
    mainLayout->addWidget(groupStore);

    // --- KHỐI 2: QUẢN LÝ NHÂN VIÊN (BẢN PRO MAX) ---
    QGroupBox *groupStaff = new QGroupBox("👥 Quản lý Nhân viên");
    groupStaff->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #CBD5E0; border-radius: 5px; margin-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; color: #2B6CB0; }");
    QVBoxLayout *staffLayout = new QVBoxLayout(groupStaff);
    
    QTableWidget *staffTable = new QTableWidget(0, 3); // Khởi tạo 0 dòng
    staffTable->setHorizontalHeaderLabels({"Mã NV", "Tên Nhân Viên", "Chức vụ"});
    staffTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    staffTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    staffTable->setEditTriggers(QAbstractItemView::NoEditTriggers); 
    
    // Form nhập liệu nhân viên
    QHBoxLayout *staffInputLayout = new QHBoxLayout();
    QLineEdit *txtStaffId = new QLineEdit(); txtStaffId->setPlaceholderText("Mã NV...");
    QLineEdit *txtStaffName = new QLineEdit(); txtStaffName->setPlaceholderText("Tên Nhân Viên...");
    QComboBox *cmbStaffRole = new QComboBox();
    cmbStaffRole->addItems({"Nhân viên Bán hàng", "Thủ kho", "Quản lý", "Quản trị viên (Admin)"});
    cmbStaffRole->setStyleSheet("padding: 8px; border: 1px solid #CBD5E0; border-radius: 5px; background: white; color: #2D3748;");

    staffInputLayout->addWidget(txtStaffId);
    staffInputLayout->addWidget(txtStaffName);
    staffInputLayout->addWidget(cmbStaffRole);

    // Cụm Nút bấm chức năng
    QHBoxLayout *staffActionLayout = new QHBoxLayout();
    QPushButton *btnAddStaff = new QPushButton("➕ Tuyển nhân viên");
    QPushButton *btnDeleteStaff = new QPushButton("❌ Sa thải");
    btnAddStaff->setStyleSheet("background-color: #28A745; color: white; padding: 8px; border-radius: 5px; font-weight: bold;");
    btnDeleteStaff->setStyleSheet("background-color: #DC3545; color: white; padding: 8px; border-radius: 5px; font-weight: bold;");
    
    staffActionLayout->addWidget(btnAddStaff);
    staffActionLayout->addWidget(btnDeleteStaff);

    staffLayout->addWidget(staffTable);
    staffLayout->addLayout(staffInputLayout);
    staffLayout->addLayout(staffActionLayout);
    mainLayout->addWidget(groupStaff);

    // --- KHỐI 3: DỮ LIỆU & BẢO MẬT ---
    QGroupBox *groupData = new QGroupBox("💾 Dữ liệu & Hệ thống");
    groupData->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #CBD5E0; border-radius: 5px; margin-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; color: #2B6CB0; }");
    QHBoxLayout *dataLayout = new QHBoxLayout(groupData);
    
    QPushButton *btnExport = new QPushButton("📥 Sao lưu dữ liệu (Backup)");
    btnExport->setStyleSheet("background-color: #28A745; color: white; padding: 12px; border-radius: 5px; font-weight: bold;");
    QPushButton *btnReset = new QPushButton("⚠️ Khôi phục Cài đặt gốc");
    btnReset->setStyleSheet("background-color: #DC3545; color: white; padding: 12px; border-radius: 5px; font-weight: bold;");
    
    dataLayout->addWidget(btnExport);
    dataLayout->addWidget(btnReset);
    mainLayout->addWidget(groupData);

    mainLayout->addStretch();

    // ================= SỰ KIỆN LOGIC =================
    
    // Hàm phụ trợ giúp thêm nhân viên vào bảng
    auto addStaffToTable = [staffTable](QString id, QString name, QString role) {
        int r = staffTable->rowCount();
        staffTable->insertRow(r);
        staffTable->setItem(r, 0, new QTableWidgetItem(id));
        staffTable->setItem(r, 1, new QTableWidgetItem(name));
        staffTable->setItem(r, 2, new QTableWidgetItem(role));
    };

    // Đổ sẵn 2 nhân viên cứng của tiệm vào
    addStaffToTable("NV01", "Phạm Phú Thành", "Quản trị viên (Admin)");
    addStaffToTable("NV02", "Trần Thị Thu Ngân", "Nhân viên Bán hàng");

    // Bấm nút Lưu Cửa hàng
    connect(btnSaveStore, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "Thành công", "Đã cập nhật thông tin cửa hàng!");
    });
    
    // Bấm nút Tuyển nhân viên
    connect(btnAddStaff, &QPushButton::clicked, this, [=]() {
        if(txtStaffId->text().isEmpty() || txtStaffName->text().isEmpty()) {
            QMessageBox::warning(this, "Lỗi", "Chưa nhập đủ thông tin mà đòi tuyển à? Vui lòng nhập Mã và Tên nhân viên!");
            return;
        }
        addStaffToTable(txtStaffId->text(), txtStaffName->text(), cmbStaffRole->currentText());
        txtStaffId->clear();
        txtStaffName->clear();
        QMessageBox::information(this, "Thành công", "Tuyển thêm được một hảo hán!");
    });

    // Bấm nút Sa thải
    connect(btnDeleteStaff, &QPushButton::clicked, this, [=]() {
        int r = staffTable->currentRow();
        if(r < 0) {
            QMessageBox::warning(this, "Lỗi", "Vui lòng click chọn một nhân viên trên bảng để sa thải!");
            return;
        }
        // Hiện thông báo xác nhận trước khi đuổi việc
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Xác nhận sa thải", 
            "Bạn có chắc chắn muốn sa thải nhân viên này không?", 
            QMessageBox::Yes | QMessageBox::No);
            
        if (reply == QMessageBox::Yes) {
            staffTable->removeRow(r);
        }
    });

    connect(btnReset, &QPushButton::clicked, this, [this]() {
        QMessageBox::StandardButton reply = QMessageBox::critical(this, "Cảnh báo Nguy hiểm", 
            "Hành động này sẽ XÓA TOÀN BỘ SÁCH trong kho và không thể khôi phục!\nBạn có chắc chắn muốn xóa?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            QMessageBox::information(this, "Đã hủy", "Để an toàn, chức năng Xóa toàn bộ đang bị khóa trong bản Demo này.");
        }
    });

    return page;
}

void MainWindow::refreshInventoryTable() {
    inventoryTable->setRowCount(0); // Xóa trắng bảng cũ
    QVector<Book> books = manager.getAllBooks(); // Lấy danh sách từ Kho
    
    for (int i = 0; i < books.size(); ++i) {
        inventoryTable->insertRow(i);
        inventoryTable->setItem(i, 0, new QTableWidgetItem(books[i].getId()));
        inventoryTable->setItem(i, 1, new QTableWidgetItem(books[i].getTitle()));
        inventoryTable->setItem(i, 2, new QTableWidgetItem(books[i].getAuthor()));
        inventoryTable->setItem(i, 3, new QTableWidgetItem(QString::number(books[i].getQuantity())));
        
        // Dùng QLocale để chèn dấu phẩy ngăn cách hàng nghìn, sau đó đổi dấu phẩy thành dấu chấm
        QString formattedPrice = QLocale(QLocale::English).toString(books[i].getPrice(), 'f', 0).replace(",", ".") + " VNĐ";
        inventoryTable->setItem(i, 4, new QTableWidgetItem(formattedPrice));    }

    // Gọi hàm này ở cuối cùng
    updateDashboardStats();

    if (salesBookTable) {          // <--- CHÈN THÊM DÒNG NÀY
        refreshSalesBookTable();   // <--- CHÈN THÊM DÒNG NÀY (Để quầy bán hàng cũng cập nhật)
    }                              // <--- CHÈN THÊM DÒNG NÀY



    
// mới thêm 27/04
// ==========================================
    // CHÈN THÊM DÒNG NÀY ĐỂ AUTO-SYNC BÁO CÁO!
    // ==========================================
    if (tableLowStock != nullptr) {
        refreshReportData();
    }
// mới thêm 27/04



    // --- LƯU TỰ ĐỘNG ---
    manager.saveToFile("database_sach.txt"); // <--- CHÈN THÊM DÒNG NÀY Ở CUỐI CÙNG
}


/*
void MainWindow::updateDashboardStats() {
    int totalStock = 0;
    double totalValue = 0;
    QVector<Book> books = manager.getAllBooks();

    for (const auto& b : books) {
        totalStock += b.getQuantity();
        totalValue += (b.getQuantity() * b.getPrice());
    }



/*
    // Cập nhật text cho các Label trên Dashboard
    if (lblTotalBooks) lblTotalBooks->setText(QString::number(totalStock));
    // Dùng 'f', 0 để định dạng tiền tệ không có số thập phân
    if (lblTotalValue) {
            QString formattedTotal = QLocale(QLocale::English).toString(totalValue, 'f', 0).replace(",", ".") + " VNĐ";
            lblTotalValue->setText(formattedTotal); 
    }



// mới thêm 27/04
    // Cập nhật text cho các Label trên Dashboard
    if (lblDashTotalBooks) lblDashTotalBooks->setText(QString::number(totalStock));
    
    if (lblDashTotalValue) {
            QString formattedTotal = QLocale(QLocale::English).toString(totalValue, 'f', 0).replace(",", ".") + " VNĐ";
            lblDashTotalValue->setText(formattedTotal); 
    }
// mới thêm 27/04


    // --- KÍCH HOẠT QUÉT HÓA ĐƠN ---
    loadSalesHistory();
}
*/

/* // thêm vào vì cần đổi hàm loadSalesHistory() 
// --- HÀM QUÉT Ổ CỨNG VÀ ĐỌC LỊCH SỬ BÁN HÀNG ---
void MainWindow::loadSalesHistory() {
    if (!recentSalesTable) return;
    recentSalesTable->setRowCount(0); // Xóa trắng bảng cũ

    QDir dir("."); // Quét thư mục hiện tại (thư mục build)
    QStringList filters;
    filters << "HD*.txt"; // Chỉ tìm file có tên bắt đầu bằng "HD" và đuôi ".txt"
    
    // Lấy danh sách file, sắp xếp theo thời gian mới nhất lên đầu
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time); 

    int row = 0;
    for (const QFileInfo &fileInfo : files) {
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString maHD, ngayBan, tongTien;
            
            // Quét từng dòng trong file hóa đơn để lấy thông tin
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.startsWith("Ma HD:")) maHD = line.mid(7).trimmed();
                if (line.startsWith("Ngay in:")) ngayBan = line.mid(9).trimmed();
                if (line.startsWith("TONG CONG:")) tongTien = line.mid(10).trimmed();
            }
            file.close();

            // Lắp đạn vào bảng
            recentSalesTable->insertRow(row);
            recentSalesTable->setItem(row, 0, new QTableWidgetItem(maHD));
            recentSalesTable->setItem(row, 1, new QTableWidgetItem("Khách vãng lai")); // Có thể nâng cấp đọc SĐT sau
            recentSalesTable->setItem(row, 2, new QTableWidgetItem(ngayBan));
            
            // Highlight màu đỏ cho số tiền nhìn cho sướng mắt
            QTableWidgetItem *moneyItem = new QTableWidgetItem(tongTien);
            moneyItem->setForeground(QBrush(QColor("#C92127")));
            moneyItem->setFont(QFont("Arial", 10, QFont::Bold));
            recentSalesTable->setItem(row, 3, moneyItem);
            
            row++;
            if (row >= 10) break; // Chỉ hiển thị tối đa 10 hóa đơn gần nhất cho gọn
        }
    }
}
*/


// mới thêm 27/04
void MainWindow::updateDashboardStats() {
    // 1. TÍNH TỔNG SỐ LƯỢNG TỒN VÀ GIÁ TRỊ KHO (Dữ liệu từ InventoryManager)
    int totalStock = 0;
    double totalValue = 0;
    QVector<Book> books = manager.getAllBooks();

    for (const auto& b : books) {
        totalStock += b.getQuantity();
        totalValue += (b.getQuantity() * b.getPrice());
    }

    if (lblDashTotalBooks) lblDashTotalBooks->setText(QString::number(totalStock));
    
    if (lblDashTotalValue) {
        QString formattedTotal = QLocale(QLocale::English).toString(totalValue, 'f', 0).replace(",", ".") + " VNĐ";
        lblDashTotalValue->setText(formattedTotal); 
    }

    // =========================================================
// =========================================================
    // 2. TÍNH TỔNG HÓA ĐƠN (Đếm số lượng file bắt đầu bằng HD)
    // =========================================================
    QDir dir(".");
    QStringList filters;
    filters << "HD*.txt";
    
    // TẠO BIẾN files ĐỂ LƯU LẠI DANH SÁCH FILE HÓA ĐƠN
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    if (lblDashTotalInvoices) {
        lblDashTotalInvoices->setText(QString::number(files.size()));
    }

    // =========================================================
    // 3. TÍNH TỔNG KHÁCH HÀNG (Đếm số dòng trong customers.txt)
    // =========================================================
    int totalCustomers = 0;
    QFile file("customers.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            in.readLine();
            totalCustomers++; // Mỗi dòng là 1 khách hàng
        }
        file.close();
    }
    
    if (lblDashTotalCustomers) {
        lblDashTotalCustomers->setText(QString::number(totalCustomers));
    }

    // =========================================================
    // 4. CẬP NHẬT BIỂU ĐỒ TỒN KHO THỰC TẾ
    // =========================================================
    inventoryChart->removeAllSeries(); // Xóa sạch cột cũ
    inventoryAxisX->clear();           // Xóa sạch tên trục X cũ

    QBarSeries *newSeries = new QBarSeries();
    QBarSet *set = new QBarSet("Tồn kho");
    set->setColor(QColor("#3182CE")); // Màu xanh dương cho cột
    
    QStringList categories;
    int maxQty = 10;
    int count = 0;
    
    for (const auto& b : books) {
        if (count >= 10) break; // Chỉ lấy 10 cuốn đầu tiên cho đỡ rối mắt
        *set << b.getQuantity();
        categories << b.getTitle().left(12) + "..."; // Rút gọn tên sách cho vừa trục X
        if (b.getQuantity() > maxQty) maxQty = b.getQuantity();
        count++;
    }
    
    newSeries->append(set);
    inventoryChart->addSeries(newSeries);
    
    inventoryAxisX->append(categories);
    inventoryAxisY->setRange(0, maxQty + (maxQty * 0.2));
    
    newSeries->attachAxis(inventoryAxisX);
    newSeries->attachAxis(inventoryAxisY);

    // =========================================================
    // 5. QUÉT HÓA ĐƠN TÌM TOP 10 SÁCH BÁN CHẠY
    // =========================================================
    QMap<QString, int> bookSales;
    for (const QFileInfo &fileInfo : files) {
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            bool isReadingItems = false;
            
            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.startsWith("Ten san pham")) {
                    isReadingItems = true;
                    in.readLine(); // Bỏ qua dòng gạch ngang
                    continue;
                }
                if (isReadingItems) {
                    if (line.startsWith("----")) {
                        isReadingItems = false;
                        continue;
                    }
                    if (line.length() >= 31) {
                        // Bóc tách tên sách và số lượng từ hóa đơn
                        QString name = line.left(25).trimmed();
                        int qty = line.mid(26, 5).trimmed().toInt();
                        bookSales[name] += qty; // Cộng dồn số lượng
                    }
                }
            }
            file.close();
        }
    }

    // Sắp xếp Map theo số lượng bán (Giảm dần)
    QList<QPair<int, QString>> sortedList;
    for (auto it = bookSales.begin(); it != bookSales.end(); ++it) {
        sortedList.append(qMakePair(it.value(), it.key()));
    }
    std::sort(sortedList.begin(), sortedList.end(), std::greater<QPair<int, QString>>());

    // Đổ dữ liệu vào bảng Top 10
    if (tableTopBooks) {
        tableTopBooks->setRowCount(0);
        int topCount = 0;
        for (const auto& pair : sortedList) {
            if (topCount >= 10) break;
            tableTopBooks->insertRow(topCount);
            
            // Xử lý Cột Hạng (Tô màu Vàng/Bạc/Đồng cho 3 hạng đầu)
            QTableWidgetItem* rankItem = new QTableWidgetItem(QString("#%1").arg(topCount + 1));
            rankItem->setTextAlignment(Qt::AlignCenter);
            rankItem->setFont(QFont("Arial", 10, QFont::Bold));
            if(topCount == 0) rankItem->setForeground(QBrush(QColor("#FFD700")));      // Vàng Top 1
            else if(topCount == 1) rankItem->setForeground(QBrush(QColor("#A9A9A9"))); // Bạc Top 2
            else if(topCount == 2) rankItem->setForeground(QBrush(QColor("#CD7F32"))); // Đồng Top 3
            
            tableTopBooks->setItem(topCount, 0, rankItem);
            
            // Tên sách
            tableTopBooks->setItem(topCount, 1, new QTableWidgetItem(pair.second)); 
            
            // Số lượng đã bán
            QTableWidgetItem* qtyItem = new QTableWidgetItem(QString::number(pair.first));
            qtyItem->setTextAlignment(Qt::AlignCenter);
            qtyItem->setFont(QFont("Arial", 10, QFont::Bold));
            qtyItem->setForeground(QBrush(QColor("#28A745"))); // Màu xanh lá
            tableTopBooks->setItem(topCount, 2, qtyItem); 
            
            topCount++;
        }
    }

    // --- KÍCH HOẠT QUÉT HÓA ĐƠN VÀO BẢNG BÊN DƯỚI ---
    loadSalesHistory();
    loadImportHistory();
}
// mới thêm 27/04

// mới thêm 27/04
void MainWindow::loadSalesHistory() {
    if (!recentSalesTable) return;
    recentSalesTable->setRowCount(0); // Xóa trắng bảng cũ

    QDir dir("."); 
    QStringList filters;
    filters << "HD*.txt"; 
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time); 

    int row = 0;
    for (const QFileInfo &fileInfo : files) {
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString maHD, ngayBan, tongTien, khachHang;
            
            khachHang = "Khách vãng lai"; // Mặc định nếu không tìm thấy

            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.startsWith("Ma HD:")) maHD = line.mid(7).trimmed();
                if (line.startsWith("Ngay in:")) ngayBan = line.mid(9).trimmed();
                
                // BẮT SĐT KHÁCH HÀNG Ở ĐÂY:
                if (line.startsWith("Khach hang:")) khachHang = line.mid(12).trimmed();

                // BẮT TỔNG TIỀN (Chơi bao hết cả khách VIP lẫn khách lẻ)
                if (line.startsWith("TONG CONG:")) tongTien = line.mid(10).trimmed();
                if (line.startsWith("TONG THANH TOAN:")) tongTien = line.mid(16).trimmed();
            }
            file.close();

            // Lắp đạn vào bảng
            recentSalesTable->insertRow(row);
            recentSalesTable->setItem(row, 0, new QTableWidgetItem(maHD));
            recentSalesTable->setItem(row, 1, new QTableWidgetItem(khachHang)); // SĐT sẽ hiện ở đây
            recentSalesTable->setItem(row, 2, new QTableWidgetItem(ngayBan));
            
            QTableWidgetItem *moneyItem = new QTableWidgetItem(tongTien);
            moneyItem->setForeground(QBrush(QColor("#C92127")));
            moneyItem->setFont(QFont("Arial", 10, QFont::Bold));
            recentSalesTable->setItem(row, 3, moneyItem);
            
            row++;
            if (row >= 10) break; // Chỉ hiển thị tối đa 10 hóa đơn
        }
    }
}
// mới thêm 27/04




// mới thêm 27/04
// ==========================================
// CÁC HÀM XỬ LÝ (SLOTS) CHO TRANG BÁO CÁO
// ==========================================

void MainWindow::on_btnMenuReport_clicked() {
    // Chuyển sang trang Báo cáo (Nếu xài nút nhấn riêng)
    refreshReportData(); 
}


void MainWindow::refreshReportData() {
    // 1. TÍNH TOÁN KHO HÀNG (GIỮ NGUYÊN)
    int totalBooks = 0;
    double totalValue = 0.0;
    QVector<Book> books = manager.getAllBooks(); 
    tableLowStock->setRowCount(0); 
    
    int row = 0;
    for (const auto& book : books) {
        totalBooks += book.getQuantity();
        totalValue += book.getQuantity() * book.getPrice();
        
        if (book.getQuantity() < 10) {
            tableLowStock->insertRow(row);
            tableLowStock->setItem(row, 0, new QTableWidgetItem(book.getId()));
            tableLowStock->setItem(row, 1, new QTableWidgetItem(book.getTitle()));
            QTableWidgetItem* qtyItem = new QTableWidgetItem(QString::number(book.getQuantity()));
            qtyItem->setForeground(QBrush(Qt::red)); 
            qtyItem->setTextAlignment(Qt::AlignCenter); 
            tableLowStock->setItem(row, 2, qtyItem);
            row++;
        }
    }
    
    lblTotalBooks->setText("📚 Tổng số lượng sách trong kho: " + QString::number(totalBooks) + " cuốn");
    QString formattedValue = QLocale(QLocale::Vietnamese, QLocale::Vietnam).toString(totalValue, 'f', 0);
    lblTotalValue->setText("💰 Tổng giá trị kho hàng: " + formattedValue + " VNĐ");

    // ==============================================================
    // 2. QUÉT HÓA ĐƠN ĐỂ TÍNH: SỐ SÁCH BÁN RA, DOANH THU & TIỀN LỜI
    // ==============================================================
    int totalSold = 0;
    double totalRevenue = 0.0;

    QDir dir(".");
    QStringList filters;
    filters << "HD*.txt";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

    for (const QFileInfo &fileInfo : files) {
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            bool isReadingItems = false;
            
            while (!in.atEnd()) {
                QString line = in.readLine();

                // Quét tìm Doanh Thu (bắt từ khóa TONG CONG hoặc TONG THANH TOAN)
                if (line.startsWith("TONG CONG:") || line.startsWith("TONG THANH TOAN:")) {
                    QString moneyStr = line.split(":").last().trimmed();
                    moneyStr.remove(" VNĐ").remove(".");
                    totalRevenue += moneyStr.toDouble();
                }

                // Quét tìm Số lượng sách bán ra (nằm giữa dòng "Ten san pham" và "---")
                if (line.startsWith("Ten san pham")) {
                    isReadingItems = true;
                    in.readLine(); // Bỏ qua dòng gạch ngang ngay dưới nó
                    continue;
                }
                if (isReadingItems) {
                    if (line.startsWith("----")) {
                        isReadingItems = false; // Kết thúc khu vực liệt kê sách
                        continue;
                    }
                    // Bóc tách cột Số Lượng (Nằm ở vị trí ký tự số 26, dài 5 ký tự theo format hóa đơn của bạn)
                    QString qtyStr = line.mid(26, 5).trimmed();
                    totalSold += qtyStr.toInt();
                }
            }
            file.close();
        }
    }

    // Tiền lời = Doanh thu - Tiền vốn. 
    // Giả sử vốn = 70% giá bán -> Tỉ suất lợi nhuận là 30% (0.3)
    double totalProfit = totalRevenue * 0.3;
    double totalCost = totalRevenue - totalProfit; // <--- TÍNH TIỀN VỐN (Sẽ bằng 70%)

    // Đẩy dữ liệu lên 4 thẻ Card
    lblTotalSold->setText(QString::number(totalSold) + " cuốn");
    
    QString formattedRev = QLocale(QLocale::Vietnamese, QLocale::Vietnam).toString(totalRevenue, 'f', 0) + " VNĐ";
    lblTotalRevenue->setText(formattedRev);
    
    // ĐẨY TIỀN VỐN LÊN UI
    QString formattedCost = QLocale(QLocale::Vietnamese, QLocale::Vietnam).toString(totalCost, 'f', 0) + " VNĐ";
    lblTotalCost->setText(formattedCost);
    
    QString formattedProfit = QLocale(QLocale::Vietnamese, QLocale::Vietnam).toString(totalProfit, 'f', 0) + " VNĐ";
    lblTotalProfit->setText(formattedProfit);
}
// mới thêm 27/04

// 28/04
// --- HÀM QUÉT Ổ CỨNG VÀ ĐỌC LỊCH SỬ NHẬP KHO ---
void MainWindow::loadImportHistory() {
    if (!recentImportsTable) return;
    recentImportsTable->setRowCount(0); // Xóa trắng bảng cũ

    QDir dir(".");
    QStringList filters;
    filters << "PN*.txt"; // Chỉ quét các file bắt đầu bằng "PN" (Phiếu Nhập)
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time); 

    int row = 0;
    for (const QFileInfo &fileInfo : files) {
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString maPN, nhaCungCap, ngayNhap, tongTien;

            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.startsWith("Ma Phieu:")) maPN = line.mid(9).trimmed();
                if (line.startsWith("Nha cung cap:")) nhaCungCap = line.mid(13).trimmed();
                if (line.startsWith("Ngay nhap:")) ngayNhap = line.mid(10).trimmed();
                if (line.startsWith("TONG TIEN:")) tongTien = line.mid(10).trimmed();
            }
            file.close();

            // Đổ dữ liệu lên bảng
            recentImportsTable->insertRow(row);
            recentImportsTable->setItem(row, 0, new QTableWidgetItem(maPN));
            recentImportsTable->setItem(row, 1, new QTableWidgetItem(nhaCungCap));
            recentImportsTable->setItem(row, 2, new QTableWidgetItem(ngayNhap));
            
            // Highlight màu xanh lá cho tiền nhập kho
            QTableWidgetItem *moneyItem = new QTableWidgetItem(tongTien);
            moneyItem->setForeground(QBrush(QColor("#28A745")));
            moneyItem->setFont(QFont("Arial", 10, QFont::Bold));
            recentImportsTable->setItem(row, 3, moneyItem);
            
            row++;
            if (row >= 10) break; // Hiển thị 10 phiếu gần nhất
        }
    }
}
// 28/04