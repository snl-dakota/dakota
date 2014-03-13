#!/bin/sh

echo "----------"
echo "Basic Test"
echo "----------"
dprepro dprepro_test.params dprepro_test.template dprepro_test.input
diff dprepro_test.base dprepro_test.input

echo "-----------"
echo "Brace Delim"
echo "-----------"
dprepro dprepro_test.params dprepro_test.delim.template dprepro_test.input
diff dprepro_test.bracedelim.base dprepro_test.input

echo "-----------"
echo "Angle Delim"
echo "-----------"
dprepro --left-delimiter='<' --right-delimiter='>' \
  dprepro_test.params dprepro_test.delim.template dprepro_test.input
diff dprepro_test.angledelim.base dprepro_test.input

echo "------------"
echo "Random Delim"
echo "------------"
dprepro --left-delimiter='||' --right-delimiter='@' \
  dprepro_test.params dprepro_test.delim.template dprepro_test.input
diff dprepro_test.randomdelim.base dprepro_test.input

echo "--------------------"
echo "Double Bracket Delim"
echo "--------------------"
dprepro --left-delimiter='[[' --right-delimiter=']]' \
  dprepro_test.params dprepro_test.delim.template dprepro_test.input
diff dprepro_test.bracketdelim.base dprepro_test.input


