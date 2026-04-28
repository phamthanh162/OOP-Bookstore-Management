cmake_minimum_required(VERSION 3.16)
project(DOAN_OOP VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Chỉ định chính xác thư mục chứa file Qt6Config.cmake
set(CMAKE_PREFIX_PATH "C:/msys64/ucrt64")
set(Qt6_DIR "C:/msys64/ucrt64/lib/cmake/Qt6")

# Thêm chữ Charts vào cuối
find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)

# GOM TẤT CẢ FILE .cpp VÀO 1 LỆNH DUY NHẤT
add_executable(DOAN_OOP
    src/main.cpp
    src/Book.cpp                 
    src/InventoryManager.cpp   
    src/MainWindow.cpp           # <--- Thêm dòng này vào  
    src/LoginDialog.cpp   # <--- THÊM DÒNG NÀY
    include/MainWindow.h         # <--- Thêm dòng này để ép CMake phải đọc nó
    include/LoginDialog.h # <--- THÊM DÒNG NÀY LUÔN CHO CHẮC

    # mới thêm 27/04
    src/ReportManager.cpp   # <--- Thêm dòng này
    include/ReportManager.h # <--- Thêm dòng này
    # mới thêm 27/04
)

target_include_directories(DOAN_OOP PRIVATE include)
# Thêm Qt6::Charts vào
target_link_libraries(DOAN_OOP PRIVATE Qt6::Widgets Qt6::Charts)