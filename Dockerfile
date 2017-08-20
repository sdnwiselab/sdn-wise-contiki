FROM multiarch/ubuntu-core:i386-xenial
MAINTAINER Sebastiano Milardo <s.milardo at hotmail.it>

ENV JAVA_TOOL_OPTIONS -Dfile.encoding=UTF8
ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-i386 

# Update and install minimal
RUN apt-get update \
    && apt-get install \ 
        --yes \
        --no-install-recommends \
        --no-install-suggests \
    build-essential \
    binutils-msp430 \
    gcc-msp430 \
    msp430-libc \
    binutils-avr \
    gcc-avr \
    gdb-avr \
    avr-libc \
    avrdude \
    binutils-arm-none-eabi \
    gcc-arm-none-eabi \
    gdb-arm-none-eabi \
    openjdk-8-jdk \
    openjdk-8-jre \
    ant \
    libncurses5-dev \
    doxygen \
    srecord \
    git \
    autoconf \
    automake \
    ca-certificates \
    git \
    libtool \
    net-tools \
    openssh-client \
    patch \
    curl \
    iputils-ping \

# Clean up packages
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -ms /bin/bash cooja
USER cooja 
ADD . /home/cooja/sdn-wise-contiki
WORKDIR /home/cooja/sdn-wise-contiki

LABEL org.label-schema.name="SDN-WISE" \
      org.label-schema.description="The stateful Software Defined Networking solution for the Internet of Things" \
      org.label-schema.url="http://sdn-wise.dieei.unict.it/" \
      org.label-schema.schema-version="1.0"

WORKDIR /home/cooja/sdn-wise-contiki/contiki/tools/cooja
RUN ant jar
