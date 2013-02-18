General
=======

This is a simple HTTP Proxy made in C++

Preconditions:
Usable port: 14805
Permissions to write on the directory where the project is. Plus all directories beneath it.
Boost libraries under /u/cs/grad/yingdi/boost/

How to use skeleton
===================

Due to restrictions on lnxsrv, several steps necessary to compile and run (if you're using this skeleton):

1. The following lines should be added to your ~/.profile

    export PATH=/usr/local/cs/bin:$PATH
    export LD_LIBRARY_PATH=/u/cs/grad/yingdi/boost/lib:/usr/local/lib64/:$LD_LIBRARY_PATH

2. To build/rebuild the code

    make clean && make

3. All compiled executables are located in build/, so you can run them as this:

    build/http-proxy


