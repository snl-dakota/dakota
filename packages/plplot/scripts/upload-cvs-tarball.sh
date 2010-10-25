#!/bin/bash
#
# Script for uploading the CVS snapshot tarball to the PLplot web site
#
# Copyright (C) 2004  Rafael Laboissiere
# This script in the public domain

# GPG signing is disabled by setting the environment variable NOGPG
# If the gpg command fails, then no mention to the detached signature
# is included in the tarball.

WWW_HOST=${WWW_HOST:-plplot.sf.net}
WWW_USER=${WWW_USER:-tomduck}
WWW_DIR=${WWW_DIR:-/home/groups/p/pl/plplot/htdocs/cvs-tarball}
GPGKEY=${GPGKEY:-0x17D965DB}

function run () {
    echo $*
    $*
}

if test $# != 1 ; then
    echo "Usage: $0 tarball.tar.gz"
    exit 1
fi

TARBALL=$1
TARBALL_NAME=`basename $TARBALL`

gpg_successful=no
test -z "$NOGPG" \
    && gpg --default-key $GPGKEY --detach-sign --armor $TARBALL \
    && gpg_successful=yes

INDEX=`tempfile --mode=0644`
DATE=`date --utc +"%Y-%m-%d %H:%M UTC"`

cat > $INDEX <<EOF
<html>
<head>
<title>PLplot CVS tarball</title>
</head>
<body>
<h2>Latest PLplot CVS snapshot distribution</h2>

<p> Tarball: <a href="$TARBALL_NAME">$TARBALL_NAME</a>.</p>
EOF

test "$gpg_successful" = yes && cat >> $INDEX <<EOF
<p> Detached GPG signature: <a href="$TARBALL_NAME.asc">$TARBALL_NAME.asc</a>
(signed with key
<a href="http://wwwkeys.pgp.net:11371/pks/lookup?op=index&search=$GPGKEY">$GPGKEY</a>)
</p>
EOF

cat >> $INDEX <<EOF
<p> Uploaded by <a href="http://sf.net/users/$WWW_USER">$WWW_USER</a>
on $DATE</p>

</body>
</html>
EOF

HOST=$WWW_USER@$WWW_HOST

run ssh $HOST rm -rf $WWW_DIR
run ssh $HOST mkdir $WWW_DIR
run scp $TARBALL $TARBALL.asc $HOST:$WWW_DIR
test "$gpg_successful" = yes && run scp $TARBALL.asc $HOST:$WWW_DIR
run scp $INDEX $HOST:$WWW_DIR/index.html
run ssh $HOST chgrp -R plplot $WWW_DIR
run ssh $HOST chmod -R g=u $WWW_DIR

#rm -f $TARBALL.asc $INDEX
