FROM centos:7
MAINTAINER Vladimir Nachev "vnachev@uni-sofia.bg"

RUN yum -y install epel-release
RUN yum -y install mongodb mongodb-server node nodejs && \
    yum -y clean all
ADD start.sh /root
ADD collector /root/collector

EXPOSE 80
CMD bash /root/start.sh
