cmake_minimum_required(VERSION 3.2)
project(open62541-performance)


find_package(open62541 1.0.0 REQUIRED COMPONENTS FullNamespace)


add_executable(read-speed-server
               src/read-speed-server.c)

target_link_libraries(read-speed-server open62541::open62541)

add_executable(read-speed-client
               src/read-speed-client.c)

target_link_libraries(read-speed-client open62541::open62541)