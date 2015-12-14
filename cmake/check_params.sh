#!/bin/sh

if [ ! "$#" = "1" ]; then
  echo "Usage : $0 <Path to Param.cpp>"
  exit
fi

sed -rn "s/^.*parseInt\(\"(.*)\".*/\1/p" $1 > parsed.tmp123456
sed -rn "s/^.*parseDouble\(\"(.*)\".*/\1/p" $1 >> parsed.tmp123456
sed -rn "s/^.*parseString\(\"(.*)\".*/\1/p" $1 >> parsed.tmp123456
sed -rn "s/^.*parseBool\(\"(.*)\".*/\1/p" $1 >> parsed.tmp123456
# These are not parsed using parseXXX, lets add them by hand :
echo "ppc_models.performance_model.TON" >> parsed.tmp123456
echo "ppc_models.performance_model.TIN" >> parsed.tmp123456
echo "ppc_models.performance_model.IO" >> parsed.tmp123456
echo "ppc_models.performance_model.BERS" >> parsed.tmp123456
echo "ppc_models.performance_model.PTON" >> parsed.tmp123456
echo "ppc_models.performance_model.PTIN" >> parsed.tmp123456
echo "ppc_models.performance_model.PIO" >> parsed.tmp123456
echo "ppc_models.performance_model.PBERS" >> parsed.tmp123456



sed -rn 's/.*ADD_PARAM\("([a-zA-Z0-9\_\.]+)", .*/\1/p' $1 > defined.tmp123456

while read line; do
  match=`grep $line defined.tmp123456`
  if [ "$match" = "" ]; then
    echo "Please define parameter \"$line\" in Param.cpp, in the static function \"getParamList()\", using that format :"
    echo "ADD_PARAM(\"$line\", \"<description>\", <TYPE>, <Optional ?>);"
    echo "(See the function for more info)"
    rm parsed.tmp123456
    rm defined.tmp123456
    exit
  fi
done < parsed.tmp123456

echo "ALL OK"

rm parsed.tmp123456
rm defined.tmp123456
