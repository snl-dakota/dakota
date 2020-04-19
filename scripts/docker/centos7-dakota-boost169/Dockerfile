# CentOS7, replacing Boost 1.53 with 1.69
# Experimentall, in supportof building Dakota with MUQ

# Base on existing CentOS7/Dakota specification
FROM centos7-dakota
MAINTAINER Brian Adams <briadam@sandia.gov>

# Remove Boost 1.53.x to avoid conflicts
RUN yum -y erase boost-*

RUN yum -y update && yum -y install \
boost169.x86_64 \
boost169-devel.x86_64

# Force some symlinks so CMake can find Boost
RUN for f in `ls /usr/lib64/libboost_*.so.1.69.0`; do sf=`echo $f | cut -f1 -d'.'`; ln -s $f ${sf}.so; done
