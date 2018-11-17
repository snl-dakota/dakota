#!/bin/bash

for f in *.yaml
do
  stem=${f%.yaml}
  ./generate_table.py $f ${stem}.html
done


