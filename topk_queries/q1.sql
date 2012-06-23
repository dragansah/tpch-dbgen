select *
	from
		lineitem,
		orders,
		part
	where
	o_orderkey = l_orderkey and
	p_partkey = l_partkey

	order by l_extendedprice + o_totalprice + p_retailprice desc
	LIMIT 1000
