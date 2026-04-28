cmake_minimum_required(VERSION 3.16)
project(OOP_Bookstore_Management VERSION 1.0.0 LANGUAGES CXX)

# Thiết lập chuẩn ngôn ngữ C++26 và bắt buộc trình biên dịch phải tuân thủ
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Kích hoạt các công cụ tự động của Qt (Meta-Object, UI, Resource)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Chỉ định đường dẫn gốc để CMake tìm kiếm thư viện Qt6 trên hệ thống MSYS2
set(CMAKE_PREFIX_PATH "C:/msys64/ucrt64")
# CỰC KỲ QUAN TRỌNG: Chỉ đích danh thư mục chứa file cấu hình Qt6Config.cmake
set(Qt6_DIR "C:/msys64/ucrt64/lib/cmake/Qt6")

# Tìm kiếm và nạp các module bắt buộc của Qt6 (Widgets cho giao diện, Charts cho biểu đồ)
find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)

# Quản lý danh sách các file mã nguồn (.cpp)
set(SOURCES
    src/main.cpp
    src/Book.cpp
    src/Customer.cpp
    src/InventoryManager.cpp
    src/ReportManager.cpp
    src/MainWindow.cpp
    src/LoginDialog.cpp
)

# Quản lý danh sách các file header (.h)
set(HEADERS
    include/Book.h
    include/Customer.h
    include/InventoryManager.h
    include/ReportManager.h
    include/MainWindow.h
    include/LoginDialog.h
)

# Khởi tạo mục tiêu biên dịch (file thực thi) từ các file nguồn và header
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Chỉ định thư mục include để trình biên dịch có thể tìm thấy các file header
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Liên kết file thực thi với các thư viện Qt6
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Charts)

# Tự động đồng bộ thư mục dữ liệu sau mỗi lần biên dịch thành công (POST_BUILD)
# Sao chép toàn bộ nội dung từ thư mục 'datafiles' gốc sang thư mục build
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/datafiles"
    "${CMAKE_CURRENT_BINARY_DIR}/datafiles"
    COMMENT "Đang sao chép thư mục datafiles sang thư mục build..."
)