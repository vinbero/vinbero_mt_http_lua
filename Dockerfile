FROM vinbero/alpine-vinbero
MAINTAINER Byeonggon Lee (gonny952@gmail.com)

EXPOSE 80
COPY config.json /config.json

RUN apk update && apk add http-parser-dev lua5.3-dev

RUN git clone https://github.com/vinbero/vinbero_tcp
RUN git clone https://github.com/vinbero/vinbero_mt
RUN git clone https://github.com/vinbero/vinbero_tcp_mt_epoll
RUN git clone https://github.com/vinbero/vinbero_mt_epoll_http
RUN git clone https://github.com/vinbero/vinbero_mt_http_lua

RUN mkdir vinbero_tcp/build; cd vinbero_tcp/build; cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..; make; make test; make install
RUN mkdir vinbero_mt/build; cd vinbero_mt/build; cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..; make; make test; make install
RUN mkdir vinbero_tcp_mt_epoll/build; cd vinbero_tcp_mt_epoll/build; cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..; make; make test; make install
RUN mkdir vinbero_mt_epoll_http/build; cd vinbero_mt_epoll_http/build; cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..; make; make test; make install
RUN mkdir vinbero_mt_http_lua/build; cd vinbero_mt_http_lua/build; cmake -DCMAKE_C_FLAGS="-I /usr/include/lua5.3 -L /usr/lib/lua5.3" -DCMAKE_INSTALL_PREFIX:PATH=/usr ..; make; make test; make install
CMD ["/usr/bin/vinbero", "-c", "/config.json", "-f", "60"]
