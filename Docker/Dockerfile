# TRACIS Processor: Docker/Dockerfile

# Copyright (C) 2022  Johnathan K Burchill

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

FROM ubuntu
RUN apt -y update
RUN apt -y upgrade
RUN apt -y install wget
RUN apt -y install gcc
RUN apt -y install g++
RUN apt -y install cmake
RUN apt -y install make
RUN apt -y install bash
RUN apt -y install zlib1g
RUN apt -y install zlib1g-dev
RUN apt -y install libxml2
RUN apt -y install libxml2-dev
RUN apt -y install libgsl-dev
RUN apt -y install libncurses-dev
RUN apt -y install libavutil-dev
RUN apt -y install libavcodec-dev
RUN apt -y install libswscale-dev
RUN apt -y install libavformat-dev
RUN apt -y install libswresample-dev
RUN apt -y install libx264-dev
RUN apt -y install git
RUN apt -y install gdb
RUN apt -y install cgdb
RUN apt -y install vim
RUN apt -y install pkg-config
RUN apt -y install zip
RUN apt -y install unzip

RUN addgroup science
RUN useradd -G science dataflow
USER dataflow
WORKDIR /home/dataflow/src
RUN cd /home/dataflow && \
wget https://spdf.gsfc.nasa.gov/pub/software/cdf/dist/cdf38_1/linux/cdf38_1-dist-cdf.tar.gz && \
tar xf cdf38_1-dist-cdf.tar.gz && \
cd cdf38_1-dist && \
make OS=linux ENV=gnu CURSES=no all
USER root
RUN cd /home/dataflow/cdf38_1-dist && \
make INSTALLDIR=/usr/local install && \
ldconfig
USER dataflow
RUN cd /home/dataflow && \
rm -rf cdf38_1-dist*
RUN cd /home/dataflow && \
git clone https://github.com/JohnathanBurchill/libtii && \
cd libtii && \
mkdir build && \
cd build && \
cmake .. && \
make
USER root
RUN mkdir /usr/local/include/tii
RUN cd /home/dataflow/libtii && \
cp include/* /usr/local/include/tii && \
cp build/lib/tii/libtii.a /usr/local/lib/ && \
cp build/lib/tiigraphics/libtiigraphics.a /usr/local/lib/
USER dataflow
RUN cd /home/dataflow && \
rm -rf cdf38_0-dist*
VOLUME ["/home/dataflow/src"]
ENTRYPOINT ["/bin/bash"]



