# Dockerfile to create a Dakota development environment
#

# use rolling build to get latest updates
FROM centos:7
MAINTAINER Jon Wilson <jwilso@sandia.gov>

# RUN yum -y install applydeltarpm

# ==========================================================================
# Install requested RPMs (package order is by Docker convention)
RUN yum -y update && yum -y install \
blas.x86_64 \
blas-devel.x86_64 \
boost.x86_64 \
boost-devel.x86_64 \
cmake.x86_64 \
gcc.x86_64 \
gcc-c++.x86_64 \
gcc-gfortran.x86_64 \
git.x86_64 \
gsl.x86_64 \
gsl-devel.x86_64 \
java-1.8.0-openjdk-devel.x86_64 \
lapack.x86_64 \
lapack-devel.x86_64 \
make.x86_64 \
openmpi.x86_64 \
openmpi-devel.x86_64 \
perl.x86_64 \
python.x86_64

# Listing potential packages that may be added later
# RUN yum -y install \
# dos2unix.x86_64 \
# doxygen.x86_64 \
# texlive-collection-latexrecommended.noarch

# ==========================================================================
# Finish construction
#   - create workspace directory
#   - install gosu utility (be sure that proxy is defined)
#   - install and configure entrypoint


##RUN mkdir /workspace

# Install "gosu" utility
# 1. Import Tianon Gravi/Andrew Page's public key
# 2. Download gosu app and app signature
# 3. Verify gosu app with app signature and Tianon's public key
# 4. Install app
RUN gpg --keyserver pool.sks-keyservers.net --recv-keys B42F6819007F00F88E364FD4036A9C25BF357DD4 \
    && curl -o /usr/local/bin/gosu -SL "https://github.com/tianon/gosu/releases/download/1.10/gosu-amd64" \
    && curl -o /usr/local/bin/gosu.asc -SL "https://github.com/tianon/gosu/releases/download/1.10/gosu-amd64.asc" \
    && gpg --verify /usr/local/bin/gosu.asc \
    && rm /usr/local/bin/gosu.asc \
    && rm -r /root/.gnupg/ \
    && chmod +x /usr/local/bin/gosu


# install and configure entrypoint script
COPY entrypoint.sh /usr/local/bin/entrypoint.sh
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]

