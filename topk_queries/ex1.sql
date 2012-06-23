select
generate_analyze(
'{"lineitem", "orders", "part"}',
'{"l_extendedprice", "o_totalprice", "p_retailprice"}',
200,
'select count(*)
	from
		lineitem,
		orders,
		part
	where
	o_orderkey = l_orderkey and
	p_partkey = l_partkey	and '
)
