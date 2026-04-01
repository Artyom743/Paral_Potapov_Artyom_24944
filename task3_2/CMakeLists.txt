cmake_minimum_required(VERSION 3.10)

# Имя проекта
project(ServerTask)

# Установка стандарта C++20 (необходим для std::jthread)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск библиотеки потоков (pthread на Linux)
find_package(Threads REQUIRED)

# Добавление исполняемого файла
add_executable(server_app_pool task3-2pool.cpp)
add_executable(server_app task3-2.cpp)

# Линковка с библиотекой потоков
target_link_libraries(server_app_pool PRIVATE Threads::Threads)
target_link_libraries(server_app PRIVATE Threads::Threads)