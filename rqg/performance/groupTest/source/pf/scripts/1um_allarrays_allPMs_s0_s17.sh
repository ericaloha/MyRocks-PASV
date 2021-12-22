#! /bin/sh
#
echo  
echo ------------------------------------------------------------------
echo starting 1UM 1PM 1Array stream0 1TB i16 2x w/restart
echo -------------------------------------------------------------------
echo  
/home/pf/auto/common/step1.sh 1 1 1
/home/pf/auto/common/step2.sh 0 1t 16 2 Y
#
echo  
echo ------------------------------------------------------------------
echo starting 1UM 2PM 1Array stream0 1TB i16 2x w/restart
echo -------------------------------------------------------------------
echo  
/home/pf/auto/common/step1.sh 1 2 1
/home/pf/auto/common/step2.sh 0 1t 16 2 Y
#
echo  
echo ------------------------------------------------------------------
echo starting 1UM 4PM 1Array stream0 1TB i16 2x w/restart
echo ------------------------------------------------------------------
echo  
/home/pf/auto/common/step1.sh 1 4 1
/home/pf/auto/common/step2.sh 0 1t 16 2 Y
#
echo  
echo ------------------------------------------------------------------
echo starting 1UM 1PM 2Array stream0 1TB i16 2x w/restart
echo ------------------------------------------------------------------
echo  
/home/pf/auto/common/step1.sh 1 1 2
/home/pf/auto/common/step2.sh 0 1t 16 2 Y
#
echo  
echo ------------------------------------------------------------------
echo starting 1UM 2PM 2Array stream0 1TB i16 2x w/restart
echo ------------------------------------------------------------------
echo  
/home/pf/auto/common/step1.sh 1 2 2
/home/pf/auto/common/step2.sh 0 1t 16 2 Y
#
echo  
echo ------------------------------------------------------------------
echo starting 1UM 4PM 2Array stream0 1TB i16 2x w/restart
echo ------------------------------------------------------------------
echo  
/home/pf/auto/common/step1.sh 1 4 2
/home/pf/auto/common/step2.sh 0 1t 16 2 Y
#
echo  
echo ------------------------------------------------------------------
echo executing setLowMem100 script to prep for following stream1_7 tests
echo ------------------------------------------------------------------
echo  
/usr/local/Calpont/bin/setLowMem100.sh
#
echo  
echo ------------------------------------------------------------------
echo starting 1UM 1PM 1Array stream1_7 100GB i16 2x w/restart 
echo ------------------------------------------------------------------
echo  
/home/pf/auto/common/step1.sh 1 1 1
/home/pf/auto/common/step2.sh 1_7 100 16 2 Y
#
echo  
echo ------------------------------------------------------------------
echo starting 1UM 2PM 1Array stream1_7 100GB i16 2x w/restart 
echo ------------------------------------------------------------------
echo  
/home/pf/auto/common/step1.sh 1 2 1
/home/pf/auto/common/step2.sh 1_7 100 16 2 Y
#
echo  
echo ------------------------------------------------------------------
echo starting 1UM 4PM 1Array stream1_7 100GB i16 2x w/restart 
echo ------------------------------------------------------------------
echo  
/home/pf/auto/common/step1.sh 1 4 1
/home/pf/auto/common/step2.sh 1_7 100 16 2 Y
#
echo  
echo ------------------------------------------------------------------
echo starting 1UM 1PM 2Array stream1_7 100GB i16 2x w/restart 
echo ------------------------------------------------------------------
echo  
/home/pf/auto/common/step1.sh 1 1 2
/home/pf/auto/common/step2.sh 1_7 100 16 2 Y
#
echo  
echo ------------------------------------------------------------------
echo starting 1UM 2PM 2Array stream1_7 100GB i16 2x w/restart 
echo ------------------------------------------------------------------
echo  
/home/pf/auto/common/step1.sh 1 2 2
/home/pf/auto/common/step2.sh 1_7 100 16 2 Y
#
echo  
echo ------------------------------------------------------------------
echo starting 1UM 4PM 2Array stream1_7 100GB i16 2x w/restart 
echo ------------------------------------------------------------------
echo  
/home/pf/auto/common/step1.sh 1 4 2
/home/pf/auto/common/step2.sh 1_7 100 16 2 Y
#
echo  
echo ------------------------------------------------------------------
echo END OF WEEKEND AUTOMATED TEST - 203.9 S0 1T & S17 100GB
echo ------------------------------------------------------------------
echo  
# End of script
