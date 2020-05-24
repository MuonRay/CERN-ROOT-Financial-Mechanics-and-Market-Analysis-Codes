#!/bin/bash
# This shell script downloads data from Yahoo from your list of picks below.

sh GetData.sh SPY

# Long picks from this filter
#http://finviz.com/screener.ashx?v=111&f=cap_midover,fa_debteq_u0.5,sh_avgvol_o1000,sh_price_o20,ta_sma20_pa10,ta_sma200_pa10,ta_sma50_pa10&ft=4
LONG_PICKS='ALV BVF CHRW CMI EXPD FFIV INFA NTY RVBD TIE' 
for SECURITY in $LONG_PICKS
do
sh GetData.sh $SECURITY
done

# Short picks from this filter
#http://finviz.com/screener.ashx?v=111&f=cap_largeover,fa_debteq_high,sh_avgvol_o2000,sh_price_o20,ta_sma20_pb,ta_sma200_pb,ta_sma50_pb&ft=4
SHORT_PICKS='AU BBT BK CHK ESRX MDT MHS NTRS RIG' 
for SECURITY in $SHORT_PICKS
do
sh GetData.sh $SECURITY
done

