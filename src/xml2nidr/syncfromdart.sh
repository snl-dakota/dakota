#!/bin/sh

svn co https://dart.sandia.gov/svn/dart/DART-WB-Core/trunk/gov.sandia.dart.dakota/src/gov gov.incoming

rsync -avz --delete --exclude '.svn' gov.incoming/ gov 
 
rm -rf gov.incoming

# svn add/commit
