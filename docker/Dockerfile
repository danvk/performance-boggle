FROM debian:stretch

MAINTAINER Neszt Tibor <tibor@neszt.hu>

RUN \
	apt-get update && apt-get -y upgrade && \
	apt-get -y install git make g++ && \
	cd /usr/src && \
	git clone https://github.com/danvk/performance-boggle.git && \
	cd performance-boggle && \
	make

CMD ["sh", "-c", "cd /usr/src/performance-boggle && make test"]
