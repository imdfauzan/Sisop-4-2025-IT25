FROM ubuntu:latest

RUN apt update && \
    apt install -y libfuse-dev gcc make && \
    mkdir -p /antink_mount /it24_host 

RUN ln -fs /usr/share/zoneinfo/Asia/Jakarta /etc/localtime && \
    echo "Asia/Jakarta" > /etc/timezone 


COPY antink.c /antink.c

WORKDIR /
RUN gcc antink.c -o antink -DFUSE_USE_VERSION=31 -D_FILE_OFFSET_BITS=64 -lfuse

CMD ["/antink", "-f", "--", "/antink_mount"]
