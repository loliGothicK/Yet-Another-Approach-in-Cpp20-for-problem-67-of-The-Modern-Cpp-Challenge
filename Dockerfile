FROM ubuntu:20.04

# install packages
RUN buildDeps='gpg-agent wget' && \
    apt-get update -y && apt-get upgrade -y && \
    apt-get install -y \
        git \
        gcc \
        g++ \
        make \
        ca-certificates \
        build-essential \
        libtool \
        libstdc++6 \
        libssl-dev \
        libgmp-dev \
        libmpfr-dev \
        libmpc-dev \
        libisl-dev \
        flex \
        $buildDeps

# build cmake and gcc-head
RUN apt-get install -y --reinstall binutils && \
    wget https://github.com/Kitware/CMake/releases/download/v3.19.0-rc3/cmake-3.19.0-rc3.tar.gz && \
    tar xvf ./cmake-3.19.0-rc3.tar.gz && rm ./cmake-3.19.0-rc3.tar.gz && \
    cd cmake-3.19.0-rc3 && ./bootstrap && make -j11 && make install && cd .. && \
    git clone git://gcc.gnu.org/git/gcc.git && \
    cd gcc && mkdir build && cd build && \
    ../configure --enable-languages=c,c++ --disable-bootstrap --disable-multilib && \
    make BOOT_CFLAGS='-O3 --march=native' -j11 && \
    make install && \
    cd ../.. && rm -rf cmake-3.19.0-rc3 gcc

# build and install boost libraries 
RUN wget https://dl.bintray.com/boostorg/release/1.74.0/source/boost_1_74_0.tar.gz && \
    tar xvf ./boost_1_74_0.tar.gz && rm -rf ./boost_1_74_0.tar.gz && \
    cd boost_1_74_0 && \
    ./bootstrap.sh && \
    ./b2 toolset=gcc install release --without-mpi --without-python --without-test -j11 && \
    cd .. && rm -rf boost_1_74_0

# build and install {fmt} library and then remove packages
RUN apt-get install -y unzip && \
    wget https://github.com/fmtlib/fmt/releases/download/7.1.2/fmt-7.1.2.zip && \
    unzip ./fmt-7.1.2.zip && rm ./fmt-7.1.2.zip && \
    cd fmt-7.1.2 && \
    mkdir build && cd build && \
    cmake .. -DBUILD_SHARED_LIBS=TRUE && make -j11 && make install && \
    cd ../.. && rm -rf fmt-7.1.2 && \
    apt-get -qq purge --auto-remove -y git && \
    rm -rf /var/lib/apt/lists/*

ENV CC=/usr/local/bin/gcc \
    CXX=/usr/local/bin/g++ \
    LD_LIBRARY_PATH=/usr/local/lib64
