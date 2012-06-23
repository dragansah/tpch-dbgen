select count(*)

	from
		supplier,
		lineitem,
		orders,
		customer,
		part,
		partsupp,
		nation n1,
		nation n2

	where
	s_suppkey = l_suppkey
	and o_orderkey = l_orderkey
	and c_custkey = o_custkey
	
	and ps_partkey = p_partkey
	and ps_suppkey = s_suppkey
	and ps_partkey = l_partkey
	and ps_suppkey = l_suppkey
	
	and s_nationkey = n1.n_nationkey
	and c_nationkey = n2.n_nationkey
