select c_nationkey, count(*), sum(o_totalprice) Revenue, avg(c_acctbal)
from customer, orders 
where c_acctbal > 9963 and c_nationkey < 5
and o_custkey = c_custkey
and o_orderdate <= '1992-06-09'
group by c_nationkey
order by 1;

