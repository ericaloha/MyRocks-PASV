Select min(l_shipdate), min(o_totalprice) from lineitem, orders  where o_orderkey = l_orderkey and  o_orderkey < 1000000 and l_partkey < 1000000;

