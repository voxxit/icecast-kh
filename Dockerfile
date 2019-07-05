FROM ubuntu:bionic

# Set time zone
ENV TZ="UTC"
RUN echo $TZ > /etc/timezone \
    # Avoid ERROR: invoke-rc.d: policy-rc.d denied execution of start.
    && sed -i "s/^exit 101$/exit 0/" /usr/sbin/policy-rc.d 

# Common packages
RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -q -y --no-install-recommends \
    curl git ca-certificates \
    build-essential libxml2 libxslt1-dev libvorbis-dev libssl-dev libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /tmp/install_icecast

COPY . .

RUN ./configure \
    && make \
    && make install

WORKDIR /usr/local/bin

CMD ["/usr/local/bin/icecast"]