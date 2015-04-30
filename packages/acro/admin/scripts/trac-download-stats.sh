#!/usr/bin/env sh

FILE="downloads.csv"
COOKIES=cookies.tmp
REPO="https://software.sandia.gov/trac"
PROJECTS=( acro utilib fast pyutilib coopr pyomo canary spot )
#PROJECTS=( utilib )

if [ -z "$2" ]; then
    if [ -z "$1" ]; then
	echo ERROR: missing username.
    else
	echo ERROR: missing password.
    fi
    echo Usage: $0 username password [resolve-dns]
    echo
    exit 1
fi
USER=$1
PASS=$2

if [ -f $FILE ]; then
    echo "ERROR: Output file exists ($FILE)"
    exit 1
fi
if [ -f $COOKIES ]; then
    echo "ERROR: Cookies file exists ($COOKIES)"
    exit 1
fi

# Fetch session credentials
fetch_trac_downloads()
{
    URL=$REPO/${1}
    wget -q --user=$USER --password=$PASS "$URL/login" -O - \
	--keep-session-cookies --save-cookies $COOKIES >> /dev/null
    wget -q --load-cookies=${COOKIES} \
        "$URL/admin/general/downloader/stats?format=csv&renderall=true" \
	-O - | sed -n '/<textarea>/,/<\/textarea>/p' \
	| sed -r 's/<\/?textarea>//' | sed '1d' | sed -r 's/;/,/g' >> $FILE
}

for project in ${PROJECTS[@]}; do
    echo Downloading $project
    fetch_trac_downloads $project
done
rm $COOKIES

echo -n "Unique non-SNL IP addresses (~users):  "
cat $FILE | cut -d, -f3 | sort | uniq -c \
    | grep -v '"134.253' | grep -v name | grep -v '""' | wc -l

echo Unique IP address downloads by file:
cat $FILE | cut -d, -f2,3 | sort | uniq | cut -d, -f1 | uniq -c \
    | grep -v '"134.253' | grep -v name | grep -v '""' 

if [ ! -z "$3" ]; then
    echo "Resolving IP addresses:"
    bad=0
    for ip in $( cat $FILE | cut -d, -f3 | sort | uniq -c | sed 's/ /_/g' \
	    | grep -v '"134.253' | grep -v '""' | sed -r 's/"//g' ); do
	Host=`echo $ip | cut -c9- | xargs host`
	if [ `echo "$Host" | grep 'not found' | wc -l` -gt 0 ]; then
	    bad=$(($bad+1))
	else
	    echo $ip | cut -c1-8 | sed 's/_/ /g' | awk '{ printf("%7i ",$1) }'
	    if [ `echo "$Host" | grep pointer | wc -l` -gt 0 ]; then
		echo "$Host" | grep pointer | sed -r 's/^.*pointer//' \
		    | sed -r 's/\.$//'
	    else
		echo "$Host"
	    fi
	fi
    done
    echo "   ...and $bad unresolvable addresses"
fi

