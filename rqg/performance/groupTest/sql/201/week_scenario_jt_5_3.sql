select l_returnflag,l_discount,l_linenumber,l_linestatus, count(*) from lineitem where l_shipdate between '1992-01-01' and '1996-06-30'
     group by 1,2,3,4 order by 1,2,3,4,5;
