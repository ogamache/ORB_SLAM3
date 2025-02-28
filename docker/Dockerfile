FROM ubuntu:bionic

ENV DEBIAN_FRONTEND noninteractive

ARG USERNAME=user
ARG PASSWORD=docker
ARG MY_UID=1000

RUN apt update && apt upgrade -y
RUN apt install software-properties-common -y

# install apt dependencies
RUN apt -y update && apt -y install \
	apt-utils \
	build-essential \
	cmake \
	curl \
	ffmpeg \
	git \
	libavcodec-dev \
	libavdevice-dev \
	libavformat-dev \
	libavutil-dev \
	libboost-all-dev \
	libbz2-dev \
	libeigen3-dev \
	libegl1-mesa-dev \
	libffi-dev \
	libgdbm-dev \
	libgl1-mesa-dri \
	libgl1-mesa-glx \
	libglew-dev \
	liblzma-dev \
	libncursesw5-dev \
	libnss3-dev \
	libopencv-dev \
	libreadline-dev \
	libsqlite3-dev \
	libsuitesparse-dev \
	libssl-dev \
	locales \
	mesa-utils \
	nano \
	ninja-build \
	python3-dev \
	python3-opencv \
	python3-pip \
	python3-tk \
	sudo \
	tk-dev \
	vim \
	xz-utils \
	zlib1g-dev

# add user
RUN useradd -u ${MY_UID} -m ${USERNAME} -s /bin/bash  && yes ${PASSWORD} | passwd ${USERNAME}
RUN usermod -aG sudo ${USERNAME}

# Give ssh key github
RUN mkdir /home/${USERNAME}/.ssh
RUN ssh-keygen -q -t rsa -N '' -f /home/${USERNAME}/.ssh/id_rsa

# # Fix locales error
# RUN locale-gen en_US.UTF-8

USER $USERNAME
ENV HOME=/home/${USERNAME}

# install Pangolin
RUN cd /home/${USERNAME} && \
	git clone --recursive https://github.com/stevenlovegrove/Pangolin.git && \
	cd Pangolin && \
	git checkout v0.6 && \
	mkdir build && \
	cd build && \
	cmake .. && \
	cmake --build .

# Standard lines
CMD ["/bin/bash"]
WORKDIR /home/user/code/

