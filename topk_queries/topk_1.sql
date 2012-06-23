select generate_analyze(
'{"lineitem", "orders", "part"}',
'{"l_extendedprice", "o_totalprice", "p_retailprice"}',
1000,
'select *
	from
		lineitem,
		orders,
		part
	where
	o_orderkey = l_orderkey and
	p_partkey = l_partkey and '
)