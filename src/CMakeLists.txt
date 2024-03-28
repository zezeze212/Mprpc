#aux_source_directory: 这是CMake提供的一个命令，用于将指定目录下的所有源文件添加到一个变量中。
#.:这表示当前 CMakeLists.txt 文件所在的目录。. 是表示当前目录的特殊符号。
#SRC_LIST: 这是一个自定义的变量名，用于存储找到的源文件列表。
#综合起来，这行代码的作用是将当前目录（即包含此CMakeLists.txt文件的目录）下的所有源文件添加到一个名为SRC_LIST的变量中。
#aux_source_directory(. SRC_LIST)
set(SRC_LIST
    mprpcapplication.cc 
    rpcprovider.cc 
    rpcheader.pb.cc 
    mprpcconfig.cc 
    mprpcchannel.cc 
    mprpccontroller.cc
    lockqueue.cc
    logger.cc
    zookeeperutil.cc
    )

#add_executable: 这是 CMake 提供的一个命令，用于向项目中添加一个可执行文件。
#provider: 这是要添加的可执行文件的名称。在这个例子中，可执行文件的名称是 provider。
#${SRC_LIST}: 这是之前通过 aux_source_directory 命令存储的源文件列表变量。${SRC_LIST} 将会被展开为该列表中的所有源文件，从而告诉 CMake 将这些源文件编译成一个名为 provider 的可执行文件。
#综合起来，这行代码的作用是将 SRC_LIST 中列出的所有源文件编译为一个名为 provider 的可执行文件。

add_library(mprpc ${SRC_LIST})


# muduo_net写在muduo_base前，两者也有依赖关系
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)

