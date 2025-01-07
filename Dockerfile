FROM osrf/ros:noetic-desktop-full

# Install packages
RUN apt-get update \
	&& apt-get install -y -q --no-install-recommends \
	apt-utils \
	bash-completion \
	clang-format \
	gnupg2 \
	sshpass \
	sudo \
	vim \
	wget \
	xterm \
	&& apt-get clean

# Update and install necessary dependencies
RUN apt-get update \
	&& apt-get install -y -q --no-install-recommends \
	build-essential \
	dirmngr \
	cmake \
	git \
	gnupg2 \
	python3-numpy \
	python3-matplotlib \
	python3 \
	python3-dev \
	python3-pip \
	&& apt-get clean

# Dependency packages
RUN apt-get -q update \
	&& apt-get -y -q --no-install-recommends install \
	libboost-all-dev \
	libpython3-dev \
	libeigen3-dev \
	libncurses5-dev \
	coinor-libipopt-dev \
	&& apt-get clean

# Install ros packages
RUN apt-get -q update \
	&& apt-get -y -q --no-install-recommends install \
	ros-${ROS_DISTRO}-ifopt \
	ros-${ROS_DISTRO}-xpp \
	&& apt-get clean

# Setup environment
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

ARG USER_NAME=cmaybe
ARG GROUP_NAME=CAU
ARG WORKSPACE_NAME=ros_ws

ARG PROJECT_NAME=towr

# Add user info
ARG USER_UID=1000
ARG USER_GID=1000
RUN groupadd --gid ${USER_GID} ${GROUP_NAME} 

RUN useradd --create-home --shell /bin/bash \
	--uid ${USER_UID} --gid ${USER_GID} ${USER_NAME} \
	# Possible security risk
	&& echo "${USER_NAME}:${GROUP_NAME}" | sudo chpasswd \
	&& echo "${USER_NAME} ALL=(ALL) NOPASSWD:ALL" > "/etc/sudoers.d/${USER_NAME}"

# Make workspace 
RUN mkdir -p /home/${USER_NAME}/${WORKSPACE_NAME}/src/${PROJECT_NAME} \
	&& chown -R ${USER_NAME}:${GROUP_NAME} /home/${USER_NAME}/${WORKSPACE_NAME}
ENV HOME /home/${USER_NAME}
ENV WORKSPACE ${HOME}/${WORKSPACE_NAME}

# Shell
USER ${USER_NAME}
WORKDIR ${WORKSPACE}
ENV SHELL "/bin/bash"

RUN echo "export USER=${USER_NAME}" >> ${HOME}/.bashrc \
	&& echo "export GROUP=${GROUP_NAME}" >> ${HOME}/.bashrc \
	&& echo "source /opt/ros/${ROS_DISTRO}/setup.bash" >> ${HOME}/.bashrc

# Shell
USER ${USER_NAME}
WORKDIR ${WORKSPACE}
ENV SHELL "/bin/bash"