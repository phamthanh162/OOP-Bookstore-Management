#include "MainWindow.h" // hihi
#include "Customer.h"

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
#include <QInputDialog>
#include <QScreen>
#include <QGuiApplication>
#include <QEvent>
#include <QPalette>
#include <QSqlQuery>
#include <QSqlError>
#include <QToolTip>

QLocale locale(QLocale::Vietnamese, QLocale::Vietnam);

MainWindow::MainWindow(QString role, QWidget *parent) : QWidget(parent), currentUserRole(role) {
    setWindowTitle("FETEL - Hệ thống Quản trị");
    
    QRect screenGeom = QGuiApplication::primaryScreen()->availableGeometry();
    
    int appWidth = qMin(1200, screenGeom.width() - 80);
    int appHeight = qMin(800, screenGeom.height() - 80);
    resize(appWidth, appHeight);

    int x = screenGeom.x() + (screenGeom.width() - appWidth) / 2;
    int y = screenGeom.y() + (screenGeom.height() - appHeight) / 2;
    move(x, y);

    setupUI(); 
    updateTheme(); 
    
    if (currentUserRole == "Staff") {
        sideMenu->item(0)->setHidden(true); 
        sideMenu->item(1)->setHidden(true); 
        sideMenu->item(5)->setHidden(true); 
        
        sideMenu->setCurrentRow(2);
        stackedPages->setCurrentIndex(2);
        
        setWindowTitle("FETEL POS - Quầy Thu Ngân (Nhân viên)");
    } else {
        sideMenu->setCurrentRow(0);
        stackedPages->setCurrentIndex(0);
        setWindowTitle("FETEL - Hệ thống Quản trị (Quyền Admin)");
    }

    manager.loadFromDatabase();
    refreshInventoryTable(); 
}

void MainWindow::setupUI() {
    QFrame *sidebarFrame = new QFrame();
    sidebarFrame->setFixedWidth(250);
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebarFrame);
    sidebarLayout->setContentsMargins(0, 0, 0, 0); 
    sidebarLayout->setSpacing(0);

    sideMenu = new QListWidget();
    sideMenu->addItem("📊 Tổng quan (Dashboard)");
    sideMenu->addItem("📦 Quản lý Kho sách");
    sideMenu->addItem("🛒 Bán hàng (POS)");
    sideMenu->addItem("📈 Báo cáo & Thống kê");
    sideMenu->addItem("👥 Quản lý Khách hàng");
    sideMenu->addItem("⚙️ Cài đặt hệ thống");

    QPushButton *btnLogout = new QPushButton("  🚪 ĐĂNG XUẤT");
    btnLogout->setObjectName("BtnLogout"); 
    btnLogout->setCursor(Qt::PointingHandCursor);

    sidebarLayout->addWidget(sideMenu);
    sidebarLayout->addWidget(btnLogout); 

    stackedPages = new QStackedWidget();
    
    stackedPages->addWidget(createDashboardPage());
    stackedPages->addWidget(createInventoryPage());
    stackedPages->addWidget(createSalesPage());
    stackedPages->addWidget(createReportPage()); 
    stackedPages->addWidget(createCustomerPage());
    stackedPages->addWidget(createSettingsPage());

    connect(sideMenu, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row >= 0) stackedPages->setCurrentIndex(row);
    });

    connect(btnLogout, &QPushButton::clicked, this, [this]() {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Xác nhận");
        msgBox.setText("Bạn có chắc chắn muốn đăng xuất khỏi hệ thống FETEL?");
        msgBox.setIcon(QMessageBox::Question);
        
        QPushButton *btnYes = msgBox.addButton("🚪 Đăng xuất", QMessageBox::YesRole);
        msgBox.addButton("❌ Hủy", QMessageBox::NoRole);
        
        msgBox.exec(); 
        if (msgBox.clickedButton() == btnYes) {
            emit logoutRequested();
        }
    });

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(sidebarFrame);
    mainLayout->addWidget(stackedPages);

    sideMenu->setCurrentRow(0); 
}

QFrame* createStatCard(QString title, QLabel* valueLabel, QString color) {
    QFrame *card = new QFrame();
    card->setObjectName("StatCard"); 
    QVBoxLayout *layout = new QVBoxLayout(card);
    
    QLabel *lblTitle = new QLabel(title);
    lblTitle->setStyleSheet("color: gray; font-size: 14px;");
    
    valueLabel->setStyleSheet(QString("color: %1; font-size: 24px; font-weight: bold;").arg(color));
    
    layout->addWidget(lblTitle);
    layout->addWidget(valueLabel);
    return card;
}

QWidget* MainWindow::createDashboardPage() {
    QWidget *page = new QWidget();
    
    QGridLayout *layout = new QGridLayout(page);
    layout->setSpacing(20);

    lblDashTotalBooks = new QLabel("0"); 
    lblDashTotalValue = new QLabel("0 đ"); 
    lblDashTotalInvoices = new QLabel("0");  
    lblDashTotalCustomers = new QLabel("0"); 

    layout->addWidget(createStatCard("Tổng số lượng tồn", lblDashTotalBooks, "#007BFF"), 0, 0); 
    layout->addWidget(createStatCard("Tổng giá trị kho", lblDashTotalValue, "#28A745"), 0, 1); 
    layout->addWidget(createStatCard("Tổng hóa đơn", lblDashTotalInvoices, "#FD7E14"), 0, 2); 
    layout->addWidget(createStatCard("Tổng khách hàng", lblDashTotalCustomers, "#6F42C1"), 0, 3);

    QFrame *chartFrame = new QFrame();
    chartFrame->setObjectName("ContentCard");
    QVBoxLayout *chartLayout = new QVBoxLayout(chartFrame);
    
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
    layout->addWidget(chartFrame, 1, 0, 1, 3); // Ép Chart chiếm 3 cột

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
    tableTopBooks->setEditTriggers(QAbstractItemView::NoEditTriggers); 
    tableTopBooks->setSelectionMode(QAbstractItemView::NoSelection); 
    topLayout->addWidget(tableTopBooks);
    
    layout->addWidget(topBooksFrame, 1, 3, 1, 1); // Top 10 lùi về góc, chiếm 1 cột

    recentImportsTable = new QTableWidget(0, 4); 
    recentImportsTable->setHorizontalHeaderLabels({"Mã Phiếu", "Nhà cung cấp", "Ngày nhập", "Tổng tiền"});
    recentImportsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    recentImportsTable->setEditTriggers(QAbstractItemView::NoEditTriggers); 
    layout->addWidget(recentImportsTable, 2, 0, 1, 2);

    recentSalesTable = new QTableWidget(0, 4); 
    recentSalesTable->setHorizontalHeaderLabels({"Mã HĐ", "Khách hàng", "Ngày bán", "Tổng tiền"});
    recentSalesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    recentSalesTable->setEditTriggers(QAbstractItemView::NoEditTriggers); 
    layout->addWidget(recentSalesTable, 2, 2, 1, 2);

    return page;
}

QWidget* MainWindow::createInventoryPage() {
    QWidget *page = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(page);

    QFrame *formFrame = new QFrame();
    formFrame->setObjectName("ContentCard"); 
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

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *btnAdd = new QPushButton("➕ Thêm sách mới");
    QPushButton *btnEdit = new QPushButton("✏️ Sửa sách đang chọn"); 
    QPushButton *btnDelete = new QPushButton("❌ Xóa sách đang chọn");
    
    btnAdd->setStyleSheet("background-color: #28A745; color: white; padding: 8px; border-radius: 5px; font-weight: bold;");
    btnEdit->setStyleSheet("background-color: #FFC107; color: black; padding: 8px; border-radius: 5px; font-weight: bold;"); 
    btnDelete->setStyleSheet("background-color: #DC3545; color: white; padding: 8px; border-radius: 5px; font-weight: bold;");

    buttonLayout->addWidget(btnAdd);
    buttonLayout->addWidget(btnEdit);
    buttonLayout->addWidget(btnDelete);
    formLayout->addRow("", buttonLayout);

    mainLayout->addWidget(formFrame);

    searchInventoryInput = new QLineEdit();
    searchInventoryInput->setPlaceholderText("🔍 Nhập Mã sách hoặc Tên sách để tìm kiếm nhanh...");
    searchInventoryInput->setStyleSheet("padding: 10px; border: 2px solid #3182CE; border-radius: 5px; font-size: 14px; margin-bottom: 10px;");
    mainLayout->addWidget(searchInventoryInput);

    inventoryTable = new QTableWidget(0, 5);
    inventoryTable->setFocusPolicy(Qt::NoFocus); // Diệt viền Focus
    inventoryTable->verticalHeader()->setVisible(false); // Ẩn cột STT thừa
    inventoryTable->setSelectionBehavior(QAbstractItemView::SelectRows); 
    inventoryTable->setHorizontalHeaderLabels({"Mã sách", "Tên sách", "Tác giả", "Số lượng", "Giá bán"});
    inventoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mainLayout->addWidget(inventoryTable);

    auto clearForm = [this]() {
        idInput->clear(); titleInput->clear(); authorInput->clear();
        quantityInput->clear(); priceInput->clear();
        idInput->setReadOnly(false); 
        idInput->setStyleSheet("background-color: white; color: #2D3748;");
        inventoryTable->clearSelection();
    };

    connect(inventoryTable, &QTableWidget::itemSelectionChanged, this, [this, clearForm]() {
        int row = inventoryTable->currentRow();
        if (row >= 0) {
            idInput->setText(inventoryTable->item(row, 0)->text());
            titleInput->setText(inventoryTable->item(row, 1)->text());
            authorInput->setText(inventoryTable->item(row, 2)->text());
            quantityInput->setText(inventoryTable->item(row, 3)->text());
            
            QString priceStr = inventoryTable->item(row, 4)->text();
            priceStr.replace(".", "").replace(" VNĐ", "");
            priceInput->setText(priceStr);

            idInput->setReadOnly(true);
            idInput->setStyleSheet("background-color: #E2E8F0; color: #718096;"); 
        } else {
            clearForm();
        }
    });

    connect(btnAdd, &QPushButton::clicked, this, [this, clearForm]() {
        if (idInput->text().isEmpty() || titleInput->text().isEmpty()) {
            QMessageBox::warning(this, "Lỗi", "Vui lòng nhập Mã sách và Tên sách!"); return;
        }
        manager.addBook(Book(idInput->text(), titleInput->text(), authorInput->text(), 
                             quantityInput->text().toInt(), priceInput->text().toDouble()));

        QString pnId = "PN" + QDateTime::currentDateTime().toString("yyMMdd_HHmmss");
        QFile pnFile("datafiles/" + pnId + ".txt");
        if (pnFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&pnFile);
            out << "Ma Phieu: " << pnId << "\n";
            out << "Nha cung cap: NXB Van Hoc\n"; 
            out << "Ngay nhap: " << QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss") << "\n";
            
            double importPrice = priceInput->text().toDouble() * 0.7;
            double totalImportValue = importPrice * quantityInput->text().toInt();
            QString formattedTotal = QLocale(QLocale::English).toString(totalImportValue, 'f', 0).replace(",", ".") + " VNĐ";
            
            out << "TONG TIEN: " << formattedTotal << "\n";
            pnFile.close();
        }

        loadImportHistory();
        refreshInventoryTable();
        clearForm();
    });

    connect(btnEdit, &QPushButton::clicked, this, [this, clearForm]() {
        int row = inventoryTable->currentRow();
        if (row < 0) {
            QMessageBox::warning(this, "Lỗi", "Vui lòng click chọn 1 cuốn sách trên bảng để sửa!"); return;
        }
        QString id = idInput->text(); 
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

    connect(searchInventoryInput, &QLineEdit::textChanged, this, [this](const QString &text) {
        for (int i = 0; i < inventoryTable->rowCount(); ++i) {
            bool matchId = inventoryTable->item(i, 0)->text().contains(text, Qt::CaseInsensitive);
            bool matchName = inventoryTable->item(i, 1)->text().contains(text, Qt::CaseInsensitive);
            inventoryTable->setRowHidden(i, !(matchId || matchName)); 
        }
    });

    return page;
}

QWidget* MainWindow::createReportPage() {
    reportPage = new QWidget();
    QVBoxLayout* reportLayout = new QVBoxLayout(reportPage);

    QLabel* title = new QLabel("<b>BÁO CÁO & THỐNG KÊ</b>");
    title->setStyleSheet("font-size: 22px; margin-bottom: 10px;"); 
    
    lblTotalBooks = new QLabel("Tổng số sách trong kho: 0");
    lblTotalBooks->setStyleSheet("font-size: 16px;"); 

    lblTotalValue = new QLabel("Tổng giá trị kho hàng: 0 VNĐ");
    lblTotalValue->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 15px;"); 

    reportLayout->addWidget(title);
    reportLayout->addWidget(lblTotalBooks);
    reportLayout->addWidget(lblTotalValue);

    QHBoxLayout* bodyLayout = new QHBoxLayout();
    
    QVBoxLayout* leftCol = new QVBoxLayout();
    QLabel* subTitle = new QLabel("⚠️ Sách sắp hết hàng (Tồn kho < 10):");
    subTitle->setStyleSheet("font-size: 16px; font-weight: bold;"); 
    
    tableLowStock = new QTableWidget(0, 3);
    tableLowStock->setHorizontalHeaderLabels({"Mã sách", "Tên sách", "Tồn kho"});
    tableLowStock->horizontalHeader()->setStretchLastSection(false);
    tableLowStock->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tableLowStock->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tableLowStock->setColumnWidth(2, 100);
    tableLowStock->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    tableLowStock->setSelectionBehavior(QAbstractItemView::SelectRows); // Bấm vào là chọn cả hàng
    tableLowStock->setFocusPolicy(Qt::NoFocus); // Xóa viền xanh bao ô
    tableLowStock->verticalHeader()->setVisible(false); // Ẩn STT
    
    leftCol->addWidget(subTitle);
    leftCol->addWidget(tableLowStock);

    QVBoxLayout* rightCol = new QVBoxLayout();
    rightCol->setAlignment(Qt::AlignTop); 
    
    QLabel* rightTitle = new QLabel("📈 Kết Quả Kinh Doanh");
    rightTitle->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 10px;"); 
    
    lblTotalSold = new QLabel("0");
    lblTotalRevenue = new QLabel("0 VNĐ");
    lblTotalCost = new QLabel("0 VNĐ"); 
    lblTotalProfit = new QLabel("0 VNĐ");

    rightCol->addWidget(rightTitle);
    rightCol->addWidget(createStatCard("Số sách đã bán ra", lblTotalSold, "#007BFF"));  
    rightCol->addWidget(createStatCard("Tổng doanh thu", lblTotalRevenue, "#28A745")); 
    rightCol->addWidget(createStatCard("Tiền vốn sách (Ước tính 70%)", lblTotalCost, "#6F42C1"));
    rightCol->addWidget(createStatCard("Tiền lời (Ước tính)", lblTotalProfit, "#FD7E14")); 
    rightCol->addStretch(); 

    bodyLayout->addLayout(leftCol, 6); 
    bodyLayout->addLayout(rightCol, 4); 

    reportLayout->addLayout(bodyLayout);

    QPushButton* btnRefresh = new QPushButton("🔄 Làm mới dữ liệu");
    btnRefresh->setStyleSheet("background-color: #007bff; color: white; padding: 10px; border-radius: 5px; font-weight: bold; font-size: 14px; margin-top: 10px;");
    connect(btnRefresh, &QPushButton::clicked, this, &MainWindow::refreshReportData);
    reportLayout->addWidget(btnRefresh);
    
    refreshReportData(); 
    return reportPage;
}

void MainWindow::refreshSalesBookTable() {
    salesBookTable->setRowCount(0);
    salesBookTable->verticalHeader()->setVisible(false);
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

void MainWindow::updateCartTotal() {
    double total = 0;
    for(int i = 0; i < cartTable->rowCount(); ++i) {
        QString id = cartTable->item(i, 0)->text();
        int qty = cartTable->item(i, 2)->text().toInt();
        Book* b = manager.findBookById(id);
        if(b) total += (b->getPrice() * qty);
    }

    double vipDiscountAmount = total * currentDiscount;
    double voucherDiscountAmount = (total * voucherPercent) + voucherFlat;
    
    double finalTotal = total - vipDiscountAmount - voucherDiscountAmount;
    if (finalTotal < 0) finalTotal = 0; 

    QString formattedFinal = QLocale(QLocale::English).toString(finalTotal, 'f', 0).replace(",", ".") + " VNĐ";

    if (vipDiscountAmount > 0 || voucherDiscountAmount > 0) {
        QString formattedOld = QLocale(QLocale::English).toString(total, 'f', 0).replace(",", ".");
        
        QString voucherAlert = "";
        if (voucherDiscountAmount > 0) {
            voucherAlert = "<br/><span style='color:#28A745; font-size:14px;'>🎟️ Đã áp dụng Voucher giảm giá!</span>";
        }

        lblCartTotal->setText(QString("<s style='color:#868E96; font-size:18px;'>%1 VNĐ</s>%2<br/> <b style='color:#C92127;'>%3</b>")
                              .arg(formattedOld).arg(voucherAlert).arg(formattedFinal));
    } else {
        lblCartTotal->setText(QString("<b style='color:#C92127;'>%1</b>").arg(formattedFinal));
    }
}

QWidget* MainWindow::createSalesPage() {
    QWidget *page = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(page); 
    
    QFrame *leftFrame = new QFrame();
    leftFrame->setObjectName("ContentCard"); 
    QVBoxLayout *leftLayout = new QVBoxLayout(leftFrame);
    
    QLabel *lblLeftTitle = new QLabel("📚 Danh sách sản phẩm (Click đúp để thêm vào giỏ)");
    lblLeftTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #2B6CB0;");
    leftLayout->addWidget(lblLeftTitle);

    searchSalesInput = new QLineEdit();
    searchSalesInput->setPlaceholderText("🔍 Quét mã vạch hoặc gõ tên sách...");
    searchSalesInput->setStyleSheet("padding: 10px; border: 2px solid #28A745; border-radius: 5px; margin-bottom: 10px;");
    leftLayout->addWidget(searchSalesInput);
    
    salesBookTable = new QTableWidget(0, 4);
    salesBookTable->setFocusPolicy(Qt::NoFocus);
    salesBookTable->verticalHeader()->setVisible(false);
    salesBookTable->setHorizontalHeaderLabels({"Mã", "Tên sách", "Tồn kho", "Giá bán"});
    salesBookTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    salesBookTable->setEditTriggers(QAbstractItemView::NoEditTriggers); 
    salesBookTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    leftLayout->addWidget(salesBookTable);
    
    QFrame *rightFrame = new QFrame();
    rightFrame->setObjectName("ContentCard");
    rightFrame->setFixedWidth(500); 
    QVBoxLayout *rightLayout = new QVBoxLayout(rightFrame);
    
    QLabel *lblRightTitle = new QLabel("🛒 Hóa đơn thanh toán");
    lblRightTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #2B6CB0;");
    rightLayout->addWidget(lblRightTitle);
    
    cartTable = new QTableWidget(0, 5);
    cartTable->setFocusPolicy(Qt::NoFocus);
    cartTable->verticalHeader()->setVisible(false);
    cartTable->setHorizontalHeaderLabels({"Mã", "Tên", "SL", "Đơn giá", "Thành tiền"});
    cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    cartTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    cartTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    cartTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    rightLayout->addWidget(cartTable);

    QGroupBox *groupVIP = new QGroupBox("👑 Khách hàng thân thiết");
    groupVIP->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #EAEAEA; border-radius: 8px; margin-top: 10px; padding-top: 15px; } QGroupBox::title { color: #DC3545; subcontrol-origin: margin; left: 10px; padding: 0 5px; }");
    QVBoxLayout *vipLayout = new QVBoxLayout(groupVIP);
    
    QHBoxLayout *phoneLayout = new QHBoxLayout();
    txtCustomerPhone = new QLineEdit();
    txtCustomerPhone->setPlaceholderText("Nhập SĐT khách hàng (VD: 0999999999)...");
    QPushButton *btnCheckVIP = new QPushButton("🔍 Check");
    btnCheckVIP->setStyleSheet("background-color: #343A40; color: white; padding: 8px 15px; border-radius: 5px; font-weight: bold;"); 
    
    QPushButton *btnIssueCard = new QPushButton("💳 Cấp thẻ VIP");
    btnIssueCard->setStyleSheet("background-color: #FFC107; color: black; padding: 8px 15px; border-radius: 5px; font-weight: bold;");
    
    phoneLayout->addWidget(txtCustomerPhone);
    phoneLayout->addWidget(btnCheckVIP);
    phoneLayout->addWidget(btnIssueCard); 
    
    lblCustomerInfo = new QLabel("Khách lẻ (Không có chiết khấu)");
    lblCustomerInfo->setStyleSheet("color: #6C757D; font-style: italic; font-size: 13px;");
 
    QHBoxLayout *voucherLayout = new QHBoxLayout();
    txtVoucher = new QLineEdit();
    txtVoucher->setPlaceholderText("Nhập mã Voucher...");
    QPushButton *btnApplyVoucher = new QPushButton("🏷️ Áp dụng");
    btnApplyVoucher->setStyleSheet("background-color: #E2E8F0; color: black; padding: 8px; font-weight: bold; border-radius: 5px;");
    
    voucherLayout->addWidget(txtVoucher);
    voucherLayout->addWidget(btnApplyVoucher);

    vipLayout->addLayout(phoneLayout);
    vipLayout->addWidget(lblCustomerInfo);
    vipLayout->addLayout(voucherLayout); 

    rightLayout->addWidget(groupVIP);
    
    QHBoxLayout *totalLayout = new QHBoxLayout();
    QLabel *lblTotalText = new QLabel("Tổng cộng:");
    lblTotalText->setStyleSheet("font-size: 20px; font-weight: bold;");
    lblCartTotal = new QLabel("0 VNĐ");
    lblCartTotal->setStyleSheet("font-size: 26px; font-weight: bold; color: #DC3545;"); 
    totalLayout->addWidget(lblTotalText);
    totalLayout->addStretch();
    totalLayout->addWidget(lblCartTotal);
    rightLayout->addLayout(totalLayout);
    
    QPushButton *btnCheckout = new QPushButton("💳 THANH TOÁN (TRỪ KHO)");
    btnCheckout->setStyleSheet("background-color: #28A745; color: white; padding: 15px; font-size: 16px; font-weight: bold; border-radius: 8px;");
    QPushButton *btnClearCart = new QPushButton("🗑️ Làm mới giỏ");
    btnClearCart->setStyleSheet("background-color: #6C757D; color: white; padding: 10px; font-weight: bold; border-radius: 5px;");
    
    rightLayout->addWidget(btnCheckout);
    rightLayout->addWidget(btnClearCart);
    
    mainLayout->addWidget(leftFrame);
    mainLayout->addWidget(rightFrame);
    
    connect(salesBookTable, &QTableWidget::itemDoubleClicked, this, [this](QTableWidgetItem *item) {
        int row = item->row();
        QString id = salesBookTable->item(row, 0)->text();
        QString title = salesBookTable->item(row, 1)->text();
        int stock = salesBookTable->item(row, 2)->text().toInt();
        
        Book* book = manager.findBookById(id);
        if (!book) return;
        if (stock <= 0) { QMessageBox::warning(this, "Hết hàng", "Sách này đã hết hàng!"); return; }
        
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
        updateCartTotal(); 
    });
    
    connect(btnClearCart, &QPushButton::clicked, this, [this]() {
        cartTable->setRowCount(0);
        txtVoucher->clear();
        voucherPercent = 0.0;
        voucherFlat = 0.0;
        updateCartTotal();
    });
    
    connect(btnCheckout, &QPushButton::clicked, this, [this]() {
        if(cartTable->rowCount() == 0) { QMessageBox::warning(this, "Trống", "Giỏ hàng đang trống!"); return; }
        
        QString billId = "HD" + QDateTime::currentDateTime().toString("yyMMdd_HHmmss");
        QString billContent;
        QTextStream stream(&billContent);
        
        stream << "================================================\n";
        stream << "             NHÀ SÁCH FETEL PRO             \n";
        stream << "    227 đường Nguyễn Văn Cừ, Quận 5, TP.HCM            \n";
        stream << "================================================\n";
        stream << "Ma HD: " << billId << "\n";
        stream << "Ngay in: " << QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss") << "\n";
        stream << "Thu ngan: " << currentUserRole << "\n";

        if (txtCustomerPhone->text().isEmpty()) {
                stream << "Khach hang: Khach vang lai\n";
        } else {
                stream << "Khach hang: " << txtCustomerPhone->text() << "\n";
        }

        stream << "------------------------------------------------\n";
        stream << QString("%1 %2 %3\n").arg("Ten san pham", -25).arg("SL", -5).arg("Thanh tien", 15);
        stream << "------------------------------------------------\n";

        for(int i = 0; i < cartTable->rowCount(); ++i) {
            QString id = cartTable->item(i, 0)->text();
            QString name = cartTable->item(i, 1)->text();
            int qty = cartTable->item(i, 2)->text().toInt();
            QString itemTotal = cartTable->item(i, 4)->text();
            
            Book* b = manager.findBookById(id);
            if(b) {
                Book updatedBook = *b;
                updatedBook.setQuantity(b->getQuantity() - qty);
                manager.updateBook(id, updatedBook);
            }

            QString shortName = name.length() > 22 ? name.left(19) + "..." : name;
            stream << QString("%1 %2 %3\n").arg(shortName, -25).arg(qty, -5).arg(itemTotal, 15);
        }
        
        stream << "------------------------------------------------\n";

        double totalToPrint = 0;
        for(int i = 0; i < cartTable->rowCount(); ++i) {
            QString itemTotalStr = cartTable->item(i, 4)->text(); 
            itemTotalStr.remove(" VNĐ").remove("."); 
            totalToPrint += itemTotalStr.toDouble();
        }
        
        double discountAmount = totalToPrint * currentDiscount;
        double finalTotalToPrint = totalToPrint - discountAmount;

        QString subTotalStr = QLocale(QLocale::English).toString(totalToPrint, 'f', 0).replace(",", ".") + " VNĐ";
        QString discountStr = "-" + QLocale(QLocale::English).toString(discountAmount, 'f', 0).replace(",", ".") + " VNĐ";
        QString finalTotalStr = QLocale(QLocale::English).toString(finalTotalToPrint, 'f', 0).replace(",", ".") + " VNĐ";

        if (currentDiscount > 0) {
            stream << QString("%1 %2\n").arg("TAM TINH:", -31).arg(subTotalStr, 15);
            stream << QString("%1 %2\n").arg(QString("CHIET KHAU VIP (%1%):").arg(currentDiscount * 100), -31).arg(discountStr, 15);
            stream << "------------------------------------------------\n";
            stream << QString("%1 %2\n").arg("TONG THANH TOAN:", -31).arg(finalTotalStr, 15);
        } else {
            stream << QString("%1 %2\n").arg("TONG CONG:", -31).arg(subTotalStr, 15);
        }
    
        stream << "================================================\n";
        stream << "         Cam on quy khach & Hen gap lai!        \n";

        QFile billFile("datafiles/" + billId + ".txt");
        if (billFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&billFile);
            out << billContent; 
            billFile.close();
        }

QString customerPhone = txtCustomerPhone->text();
        if (!customerPhone.isEmpty()) {
            int earnedPoints = finalTotalToPrint / 10000;
            if (earnedPoints == 0 && finalTotalToPrint > 0) earnedPoints = 1; 

            int oldPoints = 0;
            QString oldName = "";
            bool isNewCustomer = true; // Cờ đánh dấu để phân biệt lần đầu mua

            // 1. Kiểm tra xem khách đã có trong Database chưa
            QSqlQuery checkQuery;
            checkQuery.prepare("SELECT points, name FROM Customers WHERE phone = :phone");
            checkQuery.bindValue(":phone", customerPhone);
            if (checkQuery.exec() && checkQuery.next()) {
                oldPoints = checkQuery.value(0).toInt();
                oldName = checkQuery.value(1).toString();
                isNewCustomer = false; // Đã tìm thấy -> Không phải khách mới
            }

            int newTotalPoints = oldPoints + earnedPoints; 

            // 2. LOGIC ĐỊNH DANH CHUẨN:
            if (isNewCustomer) {
                oldName = "Khách hàng Mới"; // Lần đầu mua hàng
            } else {
                // Từ lần 2 trở đi: Xét hạng Đồng, Bạc, Vàng (< 3000) vs Kim Cương (>= 3000)
                if (newTotalPoints >= 3000) {
                    oldName = "Khách hàng VIP";
                } else {
                    oldName = "Khách hàng thường";
                }
            }

            // 3. Lưu đè dữ liệu mới vào SQLite
            QSqlQuery updateQuery;
            updateQuery.prepare("INSERT OR REPLACE INTO Customers (phone, name, points) VALUES (:phone, :name, :points)");
            updateQuery.bindValue(":phone", customerPhone);
            updateQuery.bindValue(":name", oldName);
            updateQuery.bindValue(":points", newTotalPoints);
            updateQuery.exec();

            // 4. Logic lên hạng thẻ Kim Cương (GIỮ NGUYÊN ĐOẠN CODE VẼ QPixmap Ở ĐÂY)
            if (oldPoints < 3000 && newTotalPoints >= 3000) {
                QPixmap card(600, 350);
                card.fill(Qt::white); 

                QPainter painter(&card);
                painter.setRenderHint(QPainter::Antialiasing);

                QPen diamondPen(QColor("#00B8D4")); 
                diamondPen.setWidth(8);
                painter.setPen(diamondPen);
                painter.drawRect(10, 10, 580, 330);

                painter.setPen(QColor("#1A237E")); 
                painter.setFont(QFont("Arial", 30, QFont::Bold));
                painter.drawText(card.rect(), Qt::AlignTop | Qt::AlignHCenter, "\nFETEL DIAMOND");

                painter.setPen(QColor("#2D3748")); 
                painter.setFont(QFont("Arial", 16, QFont::Bold));
                painter.drawText(50, 160, "TÊN KHÁCH: KHÁCH VIP ĐẶC QUYỀN");
                painter.drawText(50, 210, "SĐT: " + customerPhone);
                
                painter.setPen(QColor("#00B8D4")); 
                painter.drawText(50, 260, "HẠNG THẺ: KIM CƯƠNG (DIAMOND)");

                painter.setPen(QPen(Qt::black, 3)); 
                for(int i = 0; i < 60; i++) {
                    if(rand() % 3 != 0) painter.drawLine(100 + i * 5, 290, 100 + i * 5, 320); 
                }
                painter.end();

                QString fileName = "Auto_White_Diamond_" + customerPhone + ".png";
                card.save("datafiles/" + fileName);
                
                QMessageBox::information(this, "🎉 CHÚC MỪNG LÊN HẠNG!", 
                    QString("Khách hàng %1 vừa tích đủ %2 điểm!\n\n"
                            "Hệ thống đã tự động nâng cấp lên Hạng Kim Cương.\n"
                            "Kiểm tra file thẻ trắng: %3").arg(customerPhone).arg(newTotalPoints).arg(fileName));
            }
        }
        
        cartTable->setRowCount(0);

        txtVoucher->clear();
        voucherPercent = 0.0;
        voucherFlat = 0.0;
        txtCustomerPhone->clear(); 
        lblCustomerInfo->setText("Khách lẻ (Không có chiết khấu)");
        currentDiscount = 0.0;

        updateCartTotal();
        refreshInventoryTable();  
        refreshSalesBookTable();  
        refreshCustomerTable();

    });

/*
// bị bỏ 30/04
    connect(btnCheckVIP, &QPushButton::clicked, this, [this]() {
        QString phone = txtCustomerPhone->text();
        if (phone.isEmpty()) {
            QMessageBox::warning(this, "Trống", "Vui lòng nhập số điện thoại!");
            return;
        }

        bool found = false;
        int currentPoints = 0;

        QFile file("datafiles/db_customers.txt");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                QStringList parts = line.split("|");
                if (parts.size() >= 2 && parts[0] == phone) {
                    currentPoints = parts[1].toInt();
                    found = true;
                    break; 
                }
            }
            file.close();
        }

        if (found) {
            Customer c(phone, currentPoints);
            
            QString rankInfo = QString("⭐ Khách hàng: %1 - Điểm: %2\n").arg(c.getRank()).arg(c.getPoints());
            
            if (c.getRank() == "Kim Cương") {
                rankInfo += "💎 Áp dụng mức chiết khấu Kim Cương 10%!";
                lblCustomerInfo->setStyleSheet("color: #9B59B6; font-weight: bold; font-size: 14px;"); 
                currentDiscount = 0.10;
            } 
            else if (c.getRank() == "Vàng") {
                rankInfo += "🎉 Áp dụng mức chiết khấu Vàng 7%!";
                lblCustomerInfo->setStyleSheet("color: #FFC107; font-weight: bold; font-size: 14px;"); 
                currentDiscount = 0.07;
            } 
            else if (c.getRank() == "Bạc") {
                rankInfo += "🎉 Áp dụng mức chiết khấu Bạc 5%!";
                lblCustomerInfo->setStyleSheet("color: #17A2B8; font-weight: bold; font-size: 14px;"); 
                currentDiscount = 0.05;
            } 
            else { 
                rankInfo += "🎉 Áp dụng mức chiết khấu Đồng 3%!";
                lblCustomerInfo->setStyleSheet("color: #D35400; font-weight: bold; font-size: 14px;"); 
                currentDiscount = 0.03;
            }
            
            lblCustomerInfo->setText(rankInfo);

        } else {
            lblCustomerInfo->setText("Khách hàng mới (Sẽ tự động tạo thẻ sau khi thanh toán)");
            lblCustomerInfo->setStyleSheet("color: #007BFF; font-weight: bold; font-size: 13px;");
            currentDiscount = 0.0;
        }
        
        updateCartTotal(); 
    });
// bị bỏ 30/04
*/ 

// 30/04
    connect(btnCheckVIP, &QPushButton::clicked, this, [this]() {
        QString phone = txtCustomerPhone->text();
        if (phone.isEmpty()) {
            QMessageBox::warning(this, "Trống", "Vui lòng nhập số điện thoại!");
            return;
        }

        bool found = false;
        int currentPoints = 0;
        QString customerName = "Khách hàng";

        // ĐỌC TỪ DATABASE SQLITE THAY VÌ FILE TXT CŨ
        QSqlQuery query;
        query.prepare("SELECT points, name FROM Customers WHERE phone = :phone");
        query.bindValue(":phone", phone);
        if (query.exec() && query.next()) {
            currentPoints = query.value(0).toInt();
            customerName = query.value(1).toString();
            found = true;
        }

        if (found) {
            Customer c(phone, currentPoints, customerName);
            QString rankInfo = QString("⭐ Khách hàng: %1 - Điểm: %2\n").arg(c.getRank()).arg(c.getPoints());
            
            if (c.getRank() == "Kim Cương") {
                rankInfo += "💎 Áp dụng mức chiết khấu Kim Cương 10%!";
                lblCustomerInfo->setStyleSheet("color: #9B59B6; font-weight: bold; font-size: 14px;"); 
                currentDiscount = 0.10;
            } 
            else if (c.getRank() == "Vàng") {
                rankInfo += "🎉 Áp dụng mức chiết khấu Vàng 7%!";
                lblCustomerInfo->setStyleSheet("color: #FFC107; font-weight: bold; font-size: 14px;"); 
                currentDiscount = 0.07;
            } 
            else if (c.getRank() == "Bạc") {
                rankInfo += "🎉 Áp dụng mức chiết khấu Bạc 5%!";
                lblCustomerInfo->setStyleSheet("color: #17A2B8; font-weight: bold; font-size: 14px;"); 
                currentDiscount = 0.05;
            } 
            else { 
                rankInfo += "🎉 Áp dụng mức chiết khấu Đồng 3%!";
                lblCustomerInfo->setStyleSheet("color: #D35400; font-weight: bold; font-size: 14px;"); 
                currentDiscount = 0.03;
            }
            lblCustomerInfo->setText(rankInfo);

        } else {
            lblCustomerInfo->setText("Khách hàng mới (Sẽ tự động tạo thẻ sau khi thanh toán)");
            lblCustomerInfo->setStyleSheet("color: #007BFF; font-weight: bold; font-size: 13px;");
            currentDiscount = 0.0;
        }
        updateCartTotal(); 
    });
// 30/04
 

    connect(btnIssueCard, &QPushButton::clicked, this, [this]() {
        QString phone = txtCustomerPhone->text();
        if (phone.isEmpty()) {
            QMessageBox::warning(this, "Lỗi", "Khách chưa đọc Số điện thoại thì in thẻ cho ai hả nhân viên kia?");
            return;
        }

    // 30/04
        // BƠM KHÁCH HÀNG THẲNG VÀO DATABASE
        QSqlQuery query;
        query.prepare("INSERT OR REPLACE INTO Customers (phone, name, points) VALUES (:phone, 'Khách VIP', 3000)");
        query.bindValue(":phone", phone);
        if(query.exec()) {
             refreshCustomerTable(); // Cập nhật lại ngay trên bảng
        }
    // 30/04
        // --- BẠN GIỮ NGUYÊN ĐOẠN CODE VẼ THẺ QPixmap TỪ ĐÂY TRỞ XUỐNG NHÉ ---
        // QFile file("datafiles/db_customers.txt");
        // if (file.open(QIODevice::Append | QIODevice::Text)) {
        //     QTextStream out(&file);
        //     out << phone << "|3000\n";
        //     file.close();
        

        QPixmap card(600, 350);
        card.fill(QColor("#1A1A1A")); 

        QPainter painter(&card);
        painter.setRenderHint(QPainter::Antialiasing); 

        QPen diamondPen(QColor("#00E5FF"));
        diamondPen.setWidth(6);
        painter.setPen(diamondPen);
        painter.drawRect(15, 15, 570, 320);

        painter.setFont(QFont("Arial", 28, QFont::Bold));
        painter.drawText(card.rect(), Qt::AlignTop | Qt::AlignHCenter, "\nFETEL DIAMOND");

        painter.setPen(Qt::white); 
        painter.setFont(QFont("Arial", 16, QFont::Bold));
        painter.drawText(50, 160, "TÊN KHÁCH: KHÁCH HÀNG VIP");
        painter.drawText(50, 210, "SĐT: " + phone);
        
        painter.setPen(QColor("#00E5FF")); 
        painter.drawText(50, 260, "HẠNG THẺ: KIM CƯƠNG (DIAMOND)");

        painter.setPen(QPen(Qt::white, 3)); 
        for(int i = 0; i < 60; i++) {
            if(rand() % 3 != 0) { 
                painter.drawLine(100 + i * 5, 290, 100 + i * 5, 320); 
            }
        }
        painter.end(); 

        QString fileName = "TheDiamond_" + phone + ".png";
        card.save("datafiles/" + fileName);
        QMessageBox::information(this, "Thành công", "Đã cấp thẻ KIM CƯƠNG thành công!\nKiểm tra file: " + fileName);
    });

    connect(cartTable, &QTableWidget::cellDoubleClicked, this, [this](int row, int column) {
        if (column != 2) return;

        QString id = cartTable->item(row, 0)->text();
        QString name = cartTable->item(row, 1)->text();
        int currentQty = cartTable->item(row, 2)->text().toInt();

        Book* b = manager.findBookById(id);
        if (!b) return;
        int maxQty = b->getQuantity();

        bool ok;
        int newQty = QInputDialog::getInt(this, "Sửa số lượng",
                                          "Nhập số lượng mới cho:\n" + name + "\n(Nhập 0 để xóa khỏi giỏ hàng)",
                                          currentQty, 0, maxQty, 1, &ok);
        
        if (ok && newQty != currentQty) {
            if (newQty == 0) {
                cartTable->removeRow(row);
            } else {
                cartTable->item(row, 2)->setText(QString::number(newQty));

                QString priceStr = cartTable->item(row, 3)->text();
                double price = priceStr.remove(" VNĐ").remove(".").toDouble();
                
                double itemTotal = price * newQty;
                QString formattedItemTotal = QLocale(QLocale::English).toString(itemTotal, 'f', 0).replace(",", ".") + " VNĐ";
                cartTable->item(row, 4)->setText(formattedItemTotal);
            }
            updateCartTotal();
        }
    }); 

/* bị xóa 30/04
    connect(btnApplyVoucher, &QPushButton::clicked, this, [this]() {
        QString code = txtVoucher->text().trimmed();

        if(code.isEmpty()) {
            voucherPercent = 0.0;
            voucherFlat = 0.0;
            updateCartTotal(); 
            return; 
        }

        QFile file("datafiles/db_vouchers.txt"); 
        bool found = false;
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while(!in.atEnd()) {
                QStringList parts = in.readLine().split("|");
                if(parts.size() == 2 && parts[0] == code) {
                    double val = parts[1].toDouble();
                    if(val < 1.0) { voucherPercent = val; voucherFlat = 0.0; } 
                    else { voucherFlat = val; voucherPercent = 0.0; }          
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
        updateCartTotal(); 
    });
// bị xóa 30/04
*/

// 30/04 
    connect(btnApplyVoucher, &QPushButton::clicked, this, [this]() {
        QString code = txtVoucher->text().trimmed();

        if(code.isEmpty()) {
            voucherPercent = 0.0;
            voucherFlat = 0.0;
            updateCartTotal(); 
            return; 
        }

        QFile file("datafiles/db_vouchers.txt"); 
        bool found = false;
        bool isExpired = false;
        bool isNotStarted = false; // Bổ sung biến kiểm tra "Chưa đến hạn"

        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while(!in.atEnd()) {
                QStringList parts = in.readLine().split("|");
                
                // Cấu trúc mới: Mã | Giá trị | Ngày bắt đầu | Ngày kết thúc
                if(parts.size() >= 2 && parts[0] == code) {
                    found = true;
                    
                    // Kiểm tra khoảng thời gian (Cột 2 là Bắt đầu, Cột 3 là Kết thúc)
                    if (parts.size() >= 4) {
                        QDate startDate = QDate::fromString(parts[2].trimmed(), "dd/MM/yyyy");
                        QDate endDate = QDate::fromString(parts[3].trimmed(), "dd/MM/yyyy");
                        QDate today = QDate::currentDate();

                        // 1. Kiểm tra xem đã đến ngày Sale chưa
                        if (startDate.isValid() && today < startDate) {
                            isNotStarted = true;
                            break; 
                        }
                        
                        // 2. Kiểm tra xem có bị quá hạn Sale không
                        if (endDate.isValid() && today > endDate) {
                            isExpired = true;
                            break; 
                        }
                    }

                    // Nếu lọt qua được các cửa ải trên -> Hợp lệ -> Tính tiền!
                    double val = parts[1].toDouble();
                    if(val < 1.0) { voucherPercent = val; voucherFlat = 0.0; } 
                    else { voucherFlat = val; voucherPercent = 0.0; }           
                    QMessageBox::information(this, "Thành công", "Áp dụng Voucher thành công!");
                    break;
                }
            }
            file.close();
        }
        
        // --- XUẤT THÔNG BÁO LỖI PHÂN LOẠI RÕ RÀNG ---
        if(!found) {
            QMessageBox::warning(this, "Lỗi", "Mã giảm giá không tồn tại!");
            voucherPercent = 0.0; voucherFlat = 0.0;
        } else if (isNotStarted) {
            QMessageBox::warning(this, "Chưa mở thưởng", "Chiến dịch Sale này chưa bắt đầu. Vui lòng quay lại sau!");
            voucherPercent = 0.0; voucherFlat = 0.0;
        } else if (isExpired) {
            QMessageBox::warning(this, "Rất tiếc", "Mã giảm giá này đã hết thời gian sử dụng!");
            voucherPercent = 0.0; voucherFlat = 0.0;
        }
        
        updateCartTotal(); 
    });
// 30/04

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

    QLabel *lblTitle = new QLabel("⚙️ Cài đặt Hệ thống");
    lblTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2B6CB0;");
    mainLayout->addWidget(lblTitle);

    QGroupBox *groupStore = new QGroupBox("🏢 Thông tin Cửa hàng");
    groupStore->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #CBD5E0; border-radius: 5px; margin-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; color: #2B6CB0; }");
    QFormLayout *storeLayout = new QFormLayout(groupStore);
    
    QLineEdit *txtStoreName = new QLineEdit("Nhà Sách Fetel Pro");
    QLineEdit *txtAddress = new QLineEdit("227 đường Nguyễn Văn Cừ, Quận 5, TP.HCM");
    QLineEdit *txtPhone = new QLineEdit("2370.141.922");
    QPushButton *btnSaveStore = new QPushButton("💾 Lưu thông tin");
    btnSaveStore->setStyleSheet("background-color: #3182CE; color: white; padding: 8px; border-radius: 5px; font-weight: bold;");
    
    storeLayout->addRow("Tên cửa hàng:", txtStoreName);
    storeLayout->addRow("Địa chỉ:", txtAddress);
    storeLayout->addRow("Số điện thoại:", txtPhone);
    storeLayout->addRow("", btnSaveStore);
    mainLayout->addWidget(groupStore);

    QGroupBox *groupStaff = new QGroupBox("👥 Quản lý Nhân viên");
    groupStaff->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #CBD5E0; border-radius: 5px; margin-top: 15px; } QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; color: #2B6CB0; }");
    QVBoxLayout *staffLayout = new QVBoxLayout(groupStaff);
    
    QTableWidget *staffTable = new QTableWidget(0, 3); 
    staffTable->setHorizontalHeaderLabels({"Mã NV", "Tên Nhân Viên", "Chức vụ"});
    staffTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    staffTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    staffTable->setEditTriggers(QAbstractItemView::NoEditTriggers); 
    
    QHBoxLayout *staffInputLayout = new QHBoxLayout();
    QLineEdit *txtStaffId = new QLineEdit(); txtStaffId->setPlaceholderText("Mã NV...");
    QLineEdit *txtStaffName = new QLineEdit(); txtStaffName->setPlaceholderText("Tên Nhân Viên...");
    QComboBox *cmbStaffRole = new QComboBox();
    cmbStaffRole->addItems({"Nhân viên Bán hàng", "Thủ kho", "Quản lý", "Quản trị viên (Admin)"});
    cmbStaffRole->setStyleSheet("padding: 8px; border: 1px solid #CBD5E0; border-radius: 5px; background: white; color: #2D3748;");

    staffInputLayout->addWidget(txtStaffId);
    staffInputLayout->addWidget(txtStaffName);
    staffInputLayout->addWidget(cmbStaffRole);

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

    auto addStaffToTable = [staffTable](QString id, QString name, QString role) {
        int r = staffTable->rowCount();
        staffTable->insertRow(r);
        staffTable->setItem(r, 0, new QTableWidgetItem(id));
        staffTable->setItem(r, 1, new QTableWidgetItem(name));
        staffTable->setItem(r, 2, new QTableWidgetItem(role));
    };

    addStaffToTable("NV01", "Phạm Phú Thành", "Quản trị viên (Admin)");
    addStaffToTable("NV03", "Trần Thị Thu Ngân", "Nhân viên Bán hàng");

    addStaffToTable("NV02", "Lê Nguyễn", "Quản lý");
    addStaffToTable("NV04", "Huỳnh Đức Trọng", "Thủ kho");
    
    connect(btnSaveStore, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(this, "Thành công", "Đã cập nhật thông tin cửa hàng!");
    });
    
    connect(btnAddStaff, &QPushButton::clicked, this, [=, this]() {
        if(txtStaffId->text().isEmpty() || txtStaffName->text().isEmpty()) {
            QMessageBox::warning(this, "Lỗi", "Chưa nhập đủ thông tin mà đòi tuyển à? Vui lòng nhập Mã và Tên nhân viên!");
            return;
        }
        addStaffToTable(txtStaffId->text(), txtStaffName->text(), cmbStaffRole->currentText());
        txtStaffId->clear();
        txtStaffName->clear();
        QMessageBox::information(this, "Thành công", "Tuyển thêm được một hảo hán!");
    });

    connect(btnDeleteStaff, &QPushButton::clicked, this, [=, this]() {
        int r = staffTable->currentRow();
        if(r < 0) {
            QMessageBox::warning(this, "Lỗi", "Vui lòng click chọn một nhân viên trên bảng để sa thải!");
            return;
        }
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
    inventoryTable->setRowCount(0); 
    QVector<Book> books = manager.getAllBooks(); 
    
    for (int i = 0; i < books.size(); ++i) {
        inventoryTable->insertRow(i);
        inventoryTable->setItem(i, 0, new QTableWidgetItem(books[i].getId()));
        inventoryTable->setItem(i, 1, new QTableWidgetItem(books[i].getTitle()));
        inventoryTable->setItem(i, 2, new QTableWidgetItem(books[i].getAuthor()));
        inventoryTable->setItem(i, 3, new QTableWidgetItem(QString::number(books[i].getQuantity())));
        
        QString formattedPrice = QLocale(QLocale::English).toString(books[i].getPrice(), 'f', 0).replace(",", ".") + " VNĐ";
        inventoryTable->setItem(i, 4, new QTableWidgetItem(formattedPrice));    }

    updateDashboardStats();

    if (salesBookTable) {          
        refreshSalesBookTable();   
    }                              
    
    if (tableLowStock != nullptr) {
        refreshReportData();
    }
}

void MainWindow::updateDashboardStats() {
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

    QDir dir("datafiles");
    QStringList filters;
    filters << "HD*.txt";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    if (lblDashTotalInvoices) {
        lblDashTotalInvoices->setText(QString::number(files.size()));
    }

    int totalCustomers = 0;
    QFile file("datafiles/db_customers.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            in.readLine();
            totalCustomers++; 
        }
        file.close();
    }
    
    if (lblDashTotalCustomers) {
        lblDashTotalCustomers->setText(QString::number(totalCustomers));
    }

    inventoryChart->removeAllSeries(); 
    inventoryAxisX->clear();           

    QBarSeries *newSeries = new QBarSeries();

    QBarSet *set = new QBarSet("Tồn kho");
    
    QLinearGradient gradient(0, 0, 0, 400);
    gradient.setColorAt(0.0, QColor("#00C6FF")); 
    gradient.setColorAt(1.0, QColor("#0072FF")); 
    set->setBrush(gradient); 
    
    // Đặt màu và font chữ cho nhãn (2 hàm này thì QBarSet có hỗ trợ)
    set->setLabelColor(QColor("#FFD700")); 
    set->setLabelFont(QFont("Arial", 11, QFont::Bold));
    
    QStringList categories;
    int maxQty = 10;
    int count = 0;
    
    for (const auto& b : books) {
        if (count >= 10) break;
        *set << b.getQuantity();
        
        // Cho phép dài 20 ký tự
        QString title = b.getTitle();
        if (title.length() > 20) {
            title = title.left(17) + "..."; 
        }
        categories << title; 
        
        if (b.getQuantity() > maxQty) maxQty = b.getQuantity();
        count++;
    }
    
    newSeries->append(set);
    inventoryChart->addSeries(newSeries);
    
    inventoryAxisX->append(categories);
    inventoryAxisY->setRange(0, maxQty + (maxQty * 0.2));
    
    // BƯỚC 1: Bắt buộc phải Attach trục vào trước
    newSeries->attachAxis(inventoryAxisX);
    newSeries->attachAxis(inventoryAxisY);

    // BƯỚC 2: Ép font nhỏ lại và xoay (-45) ở ngay đây thì nó mới chịu tác dụng!
    QFont axisFont = inventoryAxisX->labelsFont();
    axisFont.setPointSize(8); 
    inventoryAxisX->setLabelsFont(axisFont);
    inventoryAxisX->setLabelsAngle(-45); 

    // BƯỚC 3: Dập lề dưới 100px để chứa chữ nghiêng
    inventoryChart->setMargins(QMargins(0, 10, 0, 100)); 
    inventoryChart->layout()->setContentsMargins(0, 0, 0, 0);

    // --- HOVER TOOLTIP ---
    connect(newSeries, &QBarSeries::hovered, this, [categories](bool status, int index, QBarSet *barset) {
        if (status) {
            // Khi chuột trỏ vào cột -> Hiện Popup Tooltip mượt mà
            QString tooltipText = QString("<b style='color:#0088CC;'>%1</b><br/>Tồn kho: <b>%2</b> cuốn")
                                  .arg(categories.at(index))
                                  .arg(barset->at(index));
            QToolTip::showText(QCursor::pos(), tooltipText);
        } else {
            QToolTip::hideText(); // Ẩn khi đưa chuột ra ngoài
        }
    });

    newSeries->attachAxis(inventoryAxisX);
    newSeries->attachAxis(inventoryAxisY);

    // --- FIX LỖI ÉP DẸT (XÓA MARGIN THỪA CỦA CHART) ---
    inventoryChart->setMargins(QMargins(0, 10, 0, 0));
    inventoryChart->layout()->setContentsMargins(0, 0, 0, 0);

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
                    in.readLine(); 
                    continue;
                }
                if (isReadingItems) {
                    if (line.startsWith("----")) {
                        isReadingItems = false;
                        continue;
                    }
                    if (line.length() >= 31) {
                        QString name = line.left(25).trimmed();
                        int qty = line.mid(26, 5).trimmed().toInt();
                        bookSales[name] += qty; 
                    }
                }
            }
            file.close();
        }
    }

    QList<QPair<int, QString>> sortedList;
    for (auto it = bookSales.begin(); it != bookSales.end(); ++it) {
        sortedList.append(qMakePair(it.value(), it.key()));
    }
    std::sort(sortedList.begin(), sortedList.end(), std::greater<QPair<int, QString>>());

    if (tableTopBooks) {
        tableTopBooks->setRowCount(0);
        int topCount = 0;
        for (const auto& pair : sortedList) {
            if (topCount >= 10) break;
            tableTopBooks->insertRow(topCount);
            
            QTableWidgetItem* rankItem = new QTableWidgetItem(QString("#%1").arg(topCount + 1));
            rankItem->setTextAlignment(Qt::AlignCenter);
            rankItem->setFont(QFont("Arial", 10, QFont::Bold));
            if(topCount == 0) rankItem->setForeground(QBrush(QColor("#FFD700")));      
            else if(topCount == 1) rankItem->setForeground(QBrush(QColor("#A9A9A9"))); 
            else if(topCount == 2) rankItem->setForeground(QBrush(QColor("#CD7F32"))); 
            
            tableTopBooks->setItem(topCount, 0, rankItem);
            
            tableTopBooks->setItem(topCount, 1, new QTableWidgetItem(pair.second)); 
            
            QTableWidgetItem* qtyItem = new QTableWidgetItem(QString::number(pair.first));
            qtyItem->setTextAlignment(Qt::AlignCenter);
            qtyItem->setFont(QFont("Arial", 10, QFont::Bold));
            qtyItem->setForeground(QBrush(QColor("#28A745"))); 
            tableTopBooks->setItem(topCount, 2, qtyItem); 
            
            topCount++;
        }
    }

    loadSalesHistory();
    loadImportHistory();
}

void MainWindow::loadSalesHistory() {
    if (!recentSalesTable) return;
    recentSalesTable->setRowCount(0); 

    QDir dir("datafiles"); 
    QStringList filters;
    filters << "HD*.txt"; 
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time); 

    int row = 0;
    for (const QFileInfo &fileInfo : files) {
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString maHD, ngayBan, tongTien, khachHang;
            
            khachHang = "Khách vãng lai"; 

            while (!in.atEnd()) {
                QString line = in.readLine();
                if (line.startsWith("Ma HD:")) maHD = line.mid(7).trimmed();
                if (line.startsWith("Ngay in:")) ngayBan = line.mid(9).trimmed();
                
                if (line.startsWith("Khach hang:")) khachHang = line.mid(12).trimmed();

                if (line.startsWith("TONG CONG:")) tongTien = line.mid(10).trimmed();
                if (line.startsWith("TONG THANH TOAN:")) tongTien = line.mid(16).trimmed();
            }
            file.close();

            recentSalesTable->insertRow(row);
            recentSalesTable->setItem(row, 0, new QTableWidgetItem(maHD));
            recentSalesTable->setItem(row, 1, new QTableWidgetItem(khachHang)); 
            recentSalesTable->setItem(row, 2, new QTableWidgetItem(ngayBan));
            
            QTableWidgetItem *moneyItem = new QTableWidgetItem(tongTien);
            moneyItem->setForeground(QBrush(QColor("#C92127")));
            moneyItem->setFont(QFont("Arial", 10, QFont::Bold));
            recentSalesTable->setItem(row, 3, moneyItem);
            
            row++;
            if (row >= 10) break; 
        }
    }
}

void MainWindow::on_btnMenuReport_clicked() {
    refreshReportData(); 
}

void MainWindow::refreshReportData() {
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
            qtyItem->setForeground(QBrush(QColor("#FF5252")));
            qtyItem->setTextAlignment(Qt::AlignCenter); 
            tableLowStock->setItem(row, 2, qtyItem);
            row++;
        }
    }
    
    lblTotalBooks->setText("📚 Tổng số lượng sách trong kho: " + QString::number(totalBooks) + " cuốn");
    QString formattedValue = QLocale(QLocale::Vietnamese, QLocale::Vietnam).toString(totalValue, 'f', 0);
    lblTotalValue->setText("💰 Tổng giá trị kho hàng: " + formattedValue + " VNĐ");

    int totalSold = 0;
    double totalRevenue = 0.0;

    QDir dir("datafiles");
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

                if (line.startsWith("TONG CONG:") || line.startsWith("TONG THANH TOAN:")) {
                    QString moneyStr = line.split(":").last().trimmed();
                    moneyStr.remove(" VNĐ").remove(".");
                    totalRevenue += moneyStr.toDouble();
                }

                if (line.startsWith("Ten san pham")) {
                    isReadingItems = true;
                    in.readLine(); 
                    continue;
                }
                if (isReadingItems) {
                    if (line.startsWith("----")) {
                        isReadingItems = false; 
                        continue;
                    }
                    QString qtyStr = line.mid(26, 5).trimmed();
                    totalSold += qtyStr.toInt();
                }
            }
            file.close();
        }
    }

    double totalProfit = totalRevenue * 0.3;
    double totalCost = totalRevenue - totalProfit; 

    lblTotalSold->setText(QString::number(totalSold) + " cuốn");
    
    QString formattedRev = QLocale(QLocale::Vietnamese, QLocale::Vietnam).toString(totalRevenue, 'f', 0) + " VNĐ";
    lblTotalRevenue->setText(formattedRev);
    
    QString formattedCost = QLocale(QLocale::Vietnamese, QLocale::Vietnam).toString(totalCost, 'f', 0) + " VNĐ";
    lblTotalCost->setText(formattedCost);
    
    QString formattedProfit = QLocale(QLocale::Vietnamese, QLocale::Vietnam).toString(totalProfit, 'f', 0) + " VNĐ";
    lblTotalProfit->setText(formattedProfit);
}

void MainWindow::loadImportHistory() {
    if (!recentImportsTable) return;
    recentImportsTable->setRowCount(0); 

    QDir dir("datafiles");
    QStringList filters;
    filters << "PN*.txt"; 
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

            recentImportsTable->insertRow(row);
            recentImportsTable->setItem(row, 0, new QTableWidgetItem(maPN));
            recentImportsTable->setItem(row, 1, new QTableWidgetItem(nhaCungCap));
            recentImportsTable->setItem(row, 2, new QTableWidgetItem(ngayNhap));
            
            QTableWidgetItem *moneyItem = new QTableWidgetItem(tongTien);
            moneyItem->setForeground(QBrush(QColor("#28A745")));
            moneyItem->setFont(QFont("Arial", 10, QFont::Bold));
            recentImportsTable->setItem(row, 3, moneyItem);
            
            row++;
            if (row >= 10) break; 
        }
    }
}

void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::PaletteChange || 
        event->type() == QEvent::ApplicationPaletteChange || 
        event->type() == QEvent::ThemeChange) {
        
        updateTheme(); 
    }
    QWidget::changeEvent(event);
}

void MainWindow::updateTheme() {
    static bool isUpdating = false;
    if (isUpdating) return;
    isUpdating = true;

    bool isDark = QGuiApplication::palette().color(QPalette::Window).lightness() < 128;

    // --- CẬP NHẬT GIAO DIỆN BIỂU ĐỒ ---
    if (inventoryChart) {
        inventoryChart->setTheme(isDark ? QChart::ChartThemeDark : QChart::ChartThemeLight);
        inventoryChart->setBackgroundVisible(false); 
        
        // Tắt lưới nền cho trục X và Y để biểu đồ nhìn thanh thoát như Web
        if (!inventoryChart->axes(Qt::Horizontal).isEmpty())
            inventoryChart->axes(Qt::Horizontal).first()->setGridLineVisible(false);
        if (!inventoryChart->axes(Qt::Vertical).isEmpty())
            inventoryChart->axes(Qt::Vertical).first()->setGridLineVisible(false);
    }

    // --- CẬP NHẬT MÀU SẮC CHUNG ---
    QString textColor = isDark ? "#C9D1D9" : "#2D3748"; 
    QString cardBg = isDark ? "rgba(22, 27, 34, 0.6)" : "rgba(255, 255, 255, 0.6)"; 

    // --- BỘ CSS HOÀN CHỈNH CHO TOÀN APP ---
    QString qss = QString(R"(
        /* Menu Bên Trái */
        QListWidget {
            background-color: transparent;
            font-size: 15px; font-weight: bold;
            border: none; border-right: 1px solid #0088CC;
            padding-top: 20px; color: %1;
        }
        QListWidget::item:selected {
            background-color: rgba(0, 136, 204, 0.2); 
            color: #0088CC; border-left: 5px solid #0088CC;
        }
        QListWidget::item:hover { background-color: rgba(0, 136, 204, 0.1); }

        /* Các khối Card */
        QFrame#StatCard, QFrame#ContentCard {
            background-color: %2;
            border-radius: 10px; border: 1px solid #0088CC;
        }

        /* THUỐC TRỊ BỆNH CHO BẢNG - DIỆT TẬN GỐC VẠCH FUSION */
        QTableView {
            background-color: transparent;
            color: %1;
            alternate-background-color: rgba(0, 136, 204, 0.05); 
            border: none;
            gridline-color: rgba(0, 136, 204, 0.2); 
            outline: none;
            /* TUYỆT ĐỐI KHÔNG xài selection-background-color ở đây nữa */
        }
        
        QTableView::item {
            border: none; 
            padding: 5px;
        }

        /* CHÌA KHÓA: Ép Qt tự tô màu nền bằng Item, bypass cái vạch của Windows */
        QTableView::item:selected {
            background-color: rgba(0, 136, 204, 0.4); 
            color: #FFFFFF;
            border: none;
        }

        QHeaderView::section {
            background-color: rgba(0, 136, 204, 0.1);
            color: #0088CC; font-weight: bold;
            border: none; border-bottom: 2px solid #0088CC;
            padding: 5px;
        }
        
        QHeaderView::section:vertical {
            border-bottom: none;
            border-right: 2px solid #0088CC;
            background-color: transparent;
        }

        QLabel { color: %1; }

        /* Nút Đăng Xuất Dưới Đáy */
        QPushButton#BtnLogout {
            background-color: transparent;
            text-align: left;
            padding: 15px 20px;
            font-size: 15px;
            font-weight: bold;
            border: none;
            border-top: 1px solid rgba(0, 136, 204, 0.3);
            border-right: 1px solid #0088CC;
            color: %1;
        }
        QPushButton#BtnLogout:hover {
            background-color: rgba(220, 53, 69, 0.1);
            color: #DC3545; 
            border-left: 5px solid #DC3545;
        }
    )").arg(textColor, cardBg);

    this->setStyleSheet(qss);

    isUpdating = false;

    QList<QTableWidget*> allTables = this->findChildren<QTableWidget*>();
    for (QTableWidget* table : allTables) {
        table->setFocusPolicy(Qt::NoFocus);
    }
}

QWidget* MainWindow::createCustomerPage() {
    QWidget *page = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(page);

    QLabel *lblTitle = new QLabel("👥 Quản lý Khách hàng Thân thiết");
    lblTitle->setStyleSheet("font-size: 24px; font-weight: bold; color: #2B6CB0; margin-bottom: 10px;");
    mainLayout->addWidget(lblTitle);

    QFrame *formFrame = new QFrame();
    formFrame->setObjectName("ContentCard");
    QFormLayout *formLayout = new QFormLayout(formFrame);
    
    txtCusPhone = new QLineEdit();
    txtCusPhone->setPlaceholderText("VD: 0987654321");
    txtCusName = new QLineEdit();
    txtCusName->setPlaceholderText("Nhập tên khách hàng...");
    txtCusPoints = new QLineEdit();
    txtCusPoints->setPlaceholderText("VD: 1500");

    formLayout->addRow("Số điện thoại:", txtCusPhone);
    formLayout->addRow("Tên khách hàng:", txtCusName);
    formLayout->addRow("Điểm tích lũy:", txtCusPoints);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *btnSaveCus = new QPushButton("💾 Lưu / Cập nhật");
    QPushButton *btnDeleteCus = new QPushButton("❌ Xóa khách hàng");
    
    btnSaveCus->setStyleSheet("background-color: #28A745; color: white; padding: 8px; border-radius: 5px; font-weight: bold;");
    btnDeleteCus->setStyleSheet("background-color: #DC3545; color: white; padding: 8px; border-radius: 5px; font-weight: bold;");
    
    buttonLayout->addWidget(btnSaveCus);
    buttonLayout->addWidget(btnDeleteCus);
    formLayout->addRow("", buttonLayout);
    mainLayout->addWidget(formFrame);

    searchCustomerInput = new QLineEdit();
    searchCustomerInput->setPlaceholderText("🔍 Tìm kiếm theo Số điện thoại hoặc Tên...");
    searchCustomerInput->setStyleSheet("padding: 10px; border: 2px solid #3182CE; border-radius: 5px; margin-top: 10px;");
    mainLayout->addWidget(searchCustomerInput);

    customerTable = new QTableWidget(0, 4);
    customerTable->setFocusPolicy(Qt::NoFocus);
    customerTable->verticalHeader()->setVisible(false);
    customerTable->setHorizontalHeaderLabels({"Số Điện Thoại", "Tên Khách Hàng", "Điểm", "Hạng Thẻ"});
    customerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    customerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    customerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainLayout->addWidget(customerTable);

    refreshCustomerTable();

    connect(customerTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        int row = customerTable->currentRow();
        if (row >= 0) {
            txtCusPhone->setText(customerTable->item(row, 0)->text());
            txtCusName->setText(customerTable->item(row, 1)->text());
            txtCusPoints->setText(customerTable->item(row, 2)->text());
            txtCusPhone->setReadOnly(true); 
            txtCusPhone->setStyleSheet("background-color: #E2E8F0; color: gray;");
        } else {
            txtCusPhone->clear(); txtCusName->clear(); txtCusPoints->clear();
            txtCusPhone->setReadOnly(false);
            txtCusPhone->setStyleSheet("");
        }
    });

    connect(btnSaveCus, &QPushButton::clicked, this, [this]() {
        QString phone = txtCusPhone->text().trimmed();
        QString name = txtCusName->text().trimmed();
        QString points = txtCusPoints->text().trimmed();
        
        if(phone.isEmpty()) { QMessageBox::warning(this, "Lỗi", "Vui lòng nhập SĐT!"); return; }
        if(points.isEmpty()) points = "0"; 

        // TỰ ĐỘNG GÁN TÊN CHUẨN NẾU Ô TÊN BỊ BỎ TRỐNG HOẶC LÀ TÊN HỆ THỐNG GÁN
        if (name.isEmpty() || name.contains("Khách hàng Mới") || name.contains("Khách hàng thường") || name.contains("Khách hàng VIP")) {
            if (points.toInt() >= 3000) {
                name = "Khách hàng VIP";
            } else {
                name = "Khách hàng thường";
            }
        }

        // Lệnh cập nhật chuẩn
        QSqlQuery query;
        query.prepare("INSERT OR REPLACE INTO Customers (phone, name, points) VALUES (:phone, :name, :points)");
        query.bindValue(":phone", phone);
        query.bindValue(":name", name);
        query.bindValue(":points", points.toInt());
        
        if (query.exec()) {
            refreshCustomerTable();
            QMessageBox::information(this, "Xong", "Đã lưu thông tin khách hàng!");
            txtCusName->clear(); 
        } else {
            QMessageBox::warning(this, "Lỗi Database", "Không thể lưu khách hàng!");
        }
    });

    connect(btnDeleteCus, &QPushButton::clicked, this, [this]() {
        int row = customerTable->currentRow();
        if (row < 0) {
            QMessageBox::warning(this, "Lỗi", "Vui lòng chọn 1 khách hàng trên bảng để xóa!"); return;
        }
        
        QString phoneToDelete = customerTable->item(row, 0)->text();
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Xác nhận", 
            "Bạn có chắc chắn muốn xóa khách hàng " + phoneToDelete + " không?", 
            QMessageBox::Yes | QMessageBox::No);
            
        if (reply == QMessageBox::Yes) {
            QSqlQuery query;
            query.prepare("DELETE FROM Customers WHERE phone = :phone");
            query.bindValue(":phone", phoneToDelete);
            
            if(query.exec()) {
                refreshCustomerTable();
                txtCusPhone->clear(); txtCusName->clear(); txtCusPoints->clear();
                txtCusPhone->setReadOnly(false);
                txtCusPhone->setStyleSheet("");
                QMessageBox::information(this, "Thành công", "Đã tiễn khách hàng ra chuồng gà!");
            }
        }
    });

    connect(searchCustomerInput, &QLineEdit::textChanged, this, [this](const QString &text) {
        for (int i = 0; i < customerTable->rowCount(); ++i) {
            bool match = customerTable->item(i, 0)->text().contains(text, Qt::CaseInsensitive) || 
                         customerTable->item(i, 1)->text().contains(text, Qt::CaseInsensitive);
            customerTable->setRowHidden(i, !match);
        }
    });

    return page;
}

void MainWindow::refreshCustomerTable() {
    if (!customerTable) return;
    customerTable->setRowCount(0); 

    QSqlQuery query("SELECT phone, points, name FROM Customers");
    int row = 0;
    while (query.next()) {
        QString phone = query.value(0).toString();
        int points = query.value(1).toInt();
        QString name = query.value(2).toString();
        
        Customer tempCus(phone, points, name); // Kế thừa logic tính Rank
        
        customerTable->insertRow(row);
        customerTable->setItem(row, 0, new QTableWidgetItem(tempCus.getPhone()));
        customerTable->setItem(row, 1, new QTableWidgetItem(tempCus.getName()));
        customerTable->setItem(row, 2, new QTableWidgetItem(QString::number(tempCus.getPoints())));
        
        QTableWidgetItem *rankItem = new QTableWidgetItem(tempCus.getRank());
        rankItem->setFont(QFont("Arial", 10, QFont::Bold));
        
        if (tempCus.getRank() == "Kim Cương") rankItem->setForeground(QBrush(QColor("#9B59B6"))); 
        else if (tempCus.getRank() == "Vàng") rankItem->setForeground(QBrush(QColor("#FFC107")));
        else if (tempCus.getRank() == "Bạc") rankItem->setForeground(QBrush(QColor("#17A2B8")));  
        else rankItem->setForeground(QBrush(QColor("#D35400"))); 

        customerTable->setItem(row, 3, rankItem);
        row++;
    }
}
