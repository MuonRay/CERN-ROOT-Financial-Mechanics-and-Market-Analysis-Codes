#!/bin/bash
#
# GetData.sh
#
# Downloads security price histories from Yahoo 
#
# Beginning date defined by STARTDAY, STARTMONTH, STARTYEAR


# List of securities of interest		
SECURITYSET=$1			

# Get current date and date one month prior
YEAR=`date +"%Y"`
MONTH=`date +"%m"`
DAY=`date +"%d"`
STARTYEAR=$(expr $YEAR - 0)
STARTDAY=$DAY
if [ "$MONTH" -eq "1" ]
then
	STARTMONTH="12"
	STARTYEAR=$(expr $YEAR - 0)
else
	STARTMONTH=$(expr $MONTH - 3)
fi
# Offset months as required by Yahoo URL format
MONTH=$(expr $MONTH - 1)
STARTMONTH=$(expr $STARTMONTH - 1)

# Loop through securities to get CSV files for each from Yahoo
echo
echo "Downloading:"
for SECURITY in $SECURITYSET
do
	URL="http://ichart.finance.yahoo.com/table.csv?s=$SECURITY&d=$MONTH&e=$DAY&f=$YEAR&g=d&a=$STARTMONTH&b=$STARTDAY&c=$STARTYEAR&ignore=.csv"
	echo $SECURITY...
	curl $URL -s -o /var/tmp/$SECURITY.CSV
done

# Loop through securities to process each CSV file and append to SecurityPrices.qif
echo
echo "Processing:"
for SECURITY in $SECURITYSET
do
    echo $SECURITY...
    sed -e 's/,/\ /g'  /var/tmp/$SECURITY.CSV | while read DATE OPEN HIGH LOW CLOSE VOLUME ADJCLOSE
    do
	if [ "$DATE" == "Date" ]
	then
	    
	    sed -e 's/,/\ /g'  /var/tmp/$SECURITY.CSV | while read DATE OPEN HIGH LOW CLOSE VOLUME ADJCLOSE
	    do
       		if [ "$DATE" != "Date" ]
		then
		    DATE=`echo $DATE | sed -e 's/\-/\ /g' \
			-e 's/\(.*\) \(.*\) \(.*\)/\2\/\1\/\3/' \
			-e 's/Jan/01/' -e 's/Feb/02/' -e 's/Mar/03/' -e 's/Apr/04/' \
			-e 's/May/05/' -e 's/Jun/06/' -e 's/Jul/07/' -e 's/Aug/08/' \
			-e 's/Sep/09/' -e 's/Oct/10/' -e 's/Nov/11/' -e 's/Dec/12/' `
		    echo $SECURITY $DATE $OPEN $HIGH $LOW $CLOSE $VOLUME $ADJCLOSE  >> /var/tmp/temp.txt
		fi
	    done
	   
	fi
    done
done

PWDdir=$PWD
DATAdir=$PWD/Data

# Convert Unix-formatted file to Mac-formatted file
cat /var/tmp/temp.txt | tr '\n' '\r' > $DATAdir/$SECURITY.txt
rm /var/tmp/temp.txt

# Remove CSV files
for SECURITY in $SECURITYSET
do
	rm -f /var/tmp/$SECURITY.CSV
done