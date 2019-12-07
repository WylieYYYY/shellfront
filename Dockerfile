FROM gcc:latest
RUN apt-get update
RUN apt-get install -y autoconf automake libgtk-3-dev libtool libvte-2.91-dev pkg-config gcovr xvfb
